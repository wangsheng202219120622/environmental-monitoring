/*===============================================
*   文件名称：client.c
*   创 建 者：     
*   创建日期：2025年05月18日
*   描    述：优化网络通信与错误处理
================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <jpeglib.h>
#include <SDL2/SDL.h>

#define PORT 8080
#define MAX_RETRIES 3          // 网络错误重试次数
#define RECV_TIMEOUT_SEC 2     // 接收超时时间（秒）

// 函数声明
int decompress_jpeg(unsigned char *src, unsigned long src_size, unsigned char **dst, int *width, int *height);
int safe_recv(int sock, void *buf, size_t len, int flags);
void cleanup_resources(SDL_Texture *texture, SDL_Renderer *renderer, SDL_Window *window, unsigned char *jpeg_buffer, unsigned char *rgb_buffer);

int main(int argc, char *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *server_ip = "127.0.0.1"; // 默认IP地址

    // 检查命令行参数
    if (argc > 1) {
        server_ip = argv[1]; // 使用用户提供的IP地址
    }

    printf("Connecting to server at IP: %s\n", server_ip);

    // 创建socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    // 设置连接超时
    struct timeval tv = {5, 0};  // 5秒超时
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
        perror("Setsockopt error");
        close(sock);
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // 转换IP地址
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sock);
        return -1;
    }

    // 连接服务器（支持重试）
    int retries = 0;
    while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        if (retries++ < MAX_RETRIES) {
            fprintf(stderr, "Connection failed, retrying (%d/%d)...\n", retries, MAX_RETRIES);
            sleep(1);
        } else {
            perror("Connection Failed");
            close(sock);
            return -1;
        }
    }

    printf("Connected to server. Starting video display...\n");

    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize: %s\n", SDL_GetError());
        close(sock);
        return -1;
    }

    // 创建窗口（支持分辨率动态调整）
    SDL_Window *window = SDL_CreateWindow("Video Stream", 
                                         SDL_WINDOWPOS_UNDEFINED, 
                                         SDL_WINDOWPOS_UNDEFINED, 
                                         640, 480, 
                                         SDL_WINDOW_RESIZABLE);  // 允许窗口调整大小
    if (!window) {
        fprintf(stderr, "Window could not be created: %s\n", SDL_GetError());
        SDL_Quit();
        close(sock);
        return -1;
    }

    // 创建渲染器
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Renderer could not be created: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        close(sock);
        return -1;
    }

    // 主循环变量
    SDL_Texture *texture = NULL;
    int texture_width = 0;
    int texture_height = 0;
    SDL_Event e;
    int quit = 0;
    uint32_t jpeg_size;
    unsigned char *jpeg_buffer = NULL;
    unsigned char *rgb_buffer = NULL;

    while (!quit) {
        // 处理SDL事件（支持窗口调整）
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            } else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
                // 窗口大小变化时自动调整渲染
                if (texture) {
                    SDL_DestroyTexture(texture);
                    texture = NULL;
                }
            }
        }

        // ---------- 接收图像数据 ----------
        // 1. 接收图像大小头
        if (safe_recv(sock, &jpeg_size, sizeof(jpeg_size), 0) != sizeof(jpeg_size)) {
            fprintf(stderr, "Server disconnected or timeout\n");
            quit = 1;
            break;
        }
        jpeg_size = ntohl(jpeg_size);

        // 2. 接收JPEG数据
        jpeg_buffer = realloc(jpeg_buffer, jpeg_size);
        if (!jpeg_buffer) {
            perror("Failed to reallocate JPEG buffer");
            quit = 1;
            break;
        }

        if (safe_recv(sock, jpeg_buffer, jpeg_size, 0) != jpeg_size) {
            fprintf(stderr, "Incomplete JPEG data received\n");
            continue;
        }

        // ---------- 解压JPEG ----------
        if (decompress_jpeg(jpeg_buffer, jpeg_size, &rgb_buffer, &texture_width, &texture_height) != 0) {
            fprintf(stderr, "Skipping invalid JPEG frame\n");
            continue;
        }

        // ---------- 更新纹理 ----------
        if (!texture || texture_width != texture_width || texture_height != texture_height) {
            if (texture) SDL_DestroyTexture(texture);
            texture = SDL_CreateTexture(renderer, 
                                      SDL_PIXELFORMAT_RGB24, 
                                      SDL_TEXTUREACCESS_STREAMING, 
                                      texture_width, texture_height);
            if (!texture) {
                fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
                free(rgb_buffer);
                quit = 1;
                break;
            }
        }

        SDL_UpdateTexture(texture, NULL, rgb_buffer, texture_width * 3);
        free(rgb_buffer);
        rgb_buffer = NULL;

        // ---------- 渲染 ----------
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    // 清理资源
    cleanup_resources(texture, renderer, window, jpeg_buffer, rgb_buffer);
    close(sock);

    return 0;
}

// 安全接收函数（支持超时和部分接收）
int safe_recv(int sock, void *buf, size_t len, int flags) {
    size_t total_received = 0;
    while (total_received < len) {
        ssize_t received = recv(sock, (char*)buf + total_received, len - total_received, flags);
        if (received <= 0) {
            return received; // 错误或连接关闭
        }
        total_received += received;
    }
    return total_received;
}

// JPEG解压函数
int decompress_jpeg(unsigned char *src, unsigned long src_size, unsigned char **dst, int *width, int *height) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    // 设置JPEG源
    jpeg_mem_src(&cinfo, src, src_size);

    // 读取JPEG头
    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) {
        fprintf(stderr, "Failed to read JPEG header\n");
        return -1;
    }

    // 开始解压
    if (!jpeg_start_decompress(&cinfo)) {
        fprintf(stderr, "Failed to start JPEG decompression\n");
        return -1;
    }

    *width = cinfo.output_width;
    *height = cinfo.output_height;

    // 分配内存
    int row_stride = cinfo.output_width * cinfo.output_components;
    *dst = (unsigned char *)malloc(row_stride * cinfo.output_height);
    if (!*dst) {
        perror("Failed to allocate memory for decompressed image");
        return -1;
    }

    // 读取扫描线
    JSAMPROW row_pointer[1];
    while (cinfo.output_scanline < cinfo.output_height) {
        row_pointer[0] = *dst + cinfo.output_scanline * row_stride;
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
    }

    // 完成解压
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return 0;
}

// 清理资源函数
void cleanup_resources(SDL_Texture *texture, SDL_Renderer *renderer, SDL_Window *window, unsigned char *jpeg_buffer, unsigned char *rgb_buffer) {
    if (texture) SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    if (jpeg_buffer) free(jpeg_buffer);
    if (rgb_buffer) free(rgb_buffer);
    SDL_Quit();
}
