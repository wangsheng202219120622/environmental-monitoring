
//==========================================第三版-开始较快、后期变慢
/*
    多线程处理：
        创建了 FrameProcessor 类专门处理帧和推理,将帧处理移到单独的线程，避免阻塞网络接收
        使用信号槽机制进行线程间通信
    减少内存拷贝：
        优化了图像转换流程，减少不必要的拷贝,使用 QImage::copy() 确保线程安全
    资源管理：
        添加了 cleanup() 方法确保资源正确释放,改进析构函数逻辑
    显示优化：
        直接显示处理后的帧，不再进行中间处理
    其他优化：
        使用 QMetaObject::invokeMethod 进行线程安全的调用,添加互斥锁保护共享资源
*/


#include "client.h"
#include "ui_client.h"
#include <QHostAddress>
#include <QMessageBox>
#include <QImageReader>
#include <QDebug>
#include <QDataStream>
#include <QFile>
#include <opencv2/opencv.hpp>

FrameProcessor::FrameProcessor(Inference* inference, QObject* parent)
    : QObject(parent), m_inference(inference)
{
}

FrameProcessor::~FrameProcessor()
{
}

void FrameProcessor::processFrame(const QImage &frame)
{
    QMutexLocker locker(&m_mutex);

    // 将 QImage 转换为 cv::Mat
    QImage rgbImage = frame.convertToFormat(QImage::Format_RGB888);
    cv::Mat inputFrame(rgbImage.height(), rgbImage.width(), CV_8UC3,
                      const_cast<uchar*>(rgbImage.bits()),
                      rgbImage.bytesPerLine());

    // 运行推理
    std::vector<Detection> output = m_inference->runInference(inputFrame);

    // 绘制检测结果
    cv::Mat displayFrame = inputFrame.clone();
    for (const auto& detection : output)
    {
        cv::Rect box = detection.box;
        cv::Scalar color = detection.color;

        // 绘制检测框
        cv::rectangle(displayFrame, box, color, 2);

        // 绘制检测框文本
        std::string classString = detection.className + ' ' + std::to_string(detection.confidence).substr(0, 4);
        cv::Size textSize = cv::getTextSize(classString, cv::FONT_HERSHEY_DUPLEX, 1, 2, 0);
        cv::Rect textBox(box.x, box.y - 40, textSize.width + 10, textSize.height + 20);

        cv::rectangle(displayFrame, textBox, color, cv::FILLED);
        cv::putText(displayFrame, classString, cv::Point(box.x + 5, box.y - 10),
                   cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 2, 0);
    }

    // 转换回QImage
    QImage resultImage(displayFrame.data, displayFrame.cols, displayFrame.rows,
                      displayFrame.step, QImage::Format_RGB888);

    emit frameProcessed(resultImage.copy()); // 确保返回独立的内存
}

Client::Client(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Client),
    socket(new QTcpSocket(this)),
    videoLabel(new QLabel(this)),
    isReceiving(false),
    imageBytes(0),
    inference(new Inference("/home/hqyj/Desktop/Cry/Client_yolo/yolov8n.onnx", cv::Size(640, 480), "classes.txt", false)),
    processorThread(new QThread(this)),
    frameProcessor(new FrameProcessor(inference))
{
    ui->setupUi(this);

    // UI设置
    videoLabel->setAlignment(Qt::AlignCenter);
    videoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->videoLayout->addWidget(videoLabel);

    // 将处理器移到单独线程
    frameProcessor->moveToThread(processorThread);
    processorThread->start();

    setupConnections();
}

Client::~Client()
{
    cleanup();
    delete ui;
}

void Client::cleanup()
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->disconnectFromHost();
    }

    processorThread->quit();
    processorThread->wait();
    delete frameProcessor;
    delete inference;
}

void Client::setupConnections()
{
    connect(ui->startButton, &QPushButton::clicked, this, &Client::on_startButton_clicked);
    connect(ui->stopButton, &QPushButton::clicked, this, &Client::on_stopButton_clicked);
    connect(socket, &QTcpSocket::readyRead, this, &Client::readFromServer);
    connect(socket, &QTcpSocket::disconnected, this, [this](){
        qDebug() << "Disconnected from server";
        isReceiving = false;
        ui->startButton->setEnabled(true);
        ui->stopButton->setEnabled(false);
    });
    connect(socket, &QTcpSocket::connected, this, &Client::onSocketConnected);
    connect(frameProcessor, &FrameProcessor::frameProcessed, this, &Client::displayProcessedFrame);
}

void Client::on_startButton_clicked()
{
    if (socket->state() != QAbstractSocket::ConnectedState) {
        socket->connectToHost("192.168.7.1", 8080);
        ui->startButton->setEnabled(false);
    }
}

void Client::on_stopButton_clicked()
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->disconnectFromHost();
    }
    isReceiving = false;
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
}

void Client::onSocketConnected()
{
    qDebug() << "Connected to server successfully";
    isReceiving = true;
    buffer.clear();
    imageBytes = 0;
    ui->stopButton->setEnabled(true);
}

void Client::onSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    QMessageBox::critical(this, "连接错误",
        QString("无法连接到服务器: %1").arg(socket->errorString()));
    ui->startButton->setEnabled(true);
}

void Client::readFromServer()
{
    buffer.append(socket->readAll());

    while (true) {
        if (imageBytes == 0) {
            if (buffer.size() < static_cast<int>(sizeof(quint32)))
                return;

            QDataStream stream(buffer);
            stream >> imageBytes;
            buffer.remove(0, sizeof(quint32));
            qDebug() << "Expecting image size:" << imageBytes;
        }

        if (buffer.size() >= static_cast<int>(imageBytes)) {
            QByteArray imageData = buffer.left(imageBytes);
            buffer.remove(0, imageBytes);
            imageBytes = 0;

            QImage image;
            if (image.loadFromData(imageData, "JPEG")) {
                // 将帧发送到处理器线程
                QMetaObject::invokeMethod(frameProcessor, "processFrame",
                    Qt::QueuedConnection,
                    Q_ARG(QImage, image));
            } else {
                qDebug() << "Failed to decode image data";
            }
        } else {
            break;
        }
    }
}

void Client::displayProcessedFrame(const QImage &image)
{
    if (!image.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(image);
        if (!pixmap.isNull()) {
            videoLabel->setPixmap(pixmap.scaled(
                videoLabel->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            ));
        }
    }
}






//==============================================第二版代码-yolo慢检测


//#include "client.h"
//#include "ui_client.h"
//#include <QHostAddress>
//#include <QMessageBox>
//#include <QImageReader>
//#include <QDebug>
//#include <QDataStream>
//#include <QFile>
//#include <opencv2/opencv.hpp>

//Client::Client(QWidget *parent) :
//    QWidget(parent),
//    ui(new Ui::Client),
//    socket(new QTcpSocket(this)),
//    videoLabel(new QLabel(this)),
//    isReceiving(false),
//    imageBytes(0)
//{
//    ui->setupUi(this);

//    // UI设置
//    videoLabel->setAlignment(Qt::AlignCenter);
//    videoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    ui->videoLayout->addWidget(videoLabel);

//    // 初始化 Inference 对象
//    inference = new Inference("/home/hqyj/Desktop/Client/yolov8n.onnx", cv::Size(640, 480), "classes.txt", false);

//    setupConnections();
//}

//Client::~Client()
//{
//    socket->disconnectFromHost();
//    delete inference;
//    delete ui;
//}

//void Client::setupConnections()
//{
//    connect(ui->startButton, &QPushButton::clicked, this, &Client::on_startButton_clicked);
//    connect(ui->stopButton, &QPushButton::clicked, this, &Client::on_stopButton_clicked);
//    connect(socket, &QTcpSocket::readyRead, this, &Client::readFromServer);
//    connect(socket, &QTcpSocket::disconnected, this, [this](){
//        qDebug() << "Disconnected from server";
//        isReceiving = false;
//        ui->startButton->setEnabled(true);
//        ui->stopButton->setEnabled(false);
//    });
//    connect(socket, &QTcpSocket::connected, this, &Client::onSocketConnected);
//}

//void Client::on_startButton_clicked()
//{
//    if (socket->state() != QAbstractSocket::ConnectedState) {
//        socket->connectToHost("192.168.7.1", 8080);
//        ui->startButton->setEnabled(false);
//    }
//}

//void Client::on_stopButton_clicked()
//{
//    if (socket->state() == QAbstractSocket::ConnectedState) {
//        socket->disconnectFromHost();
//    }
//    isReceiving = false;
//    ui->startButton->setEnabled(true);
//    ui->stopButton->setEnabled(false);
//}

//void Client::onSocketConnected()
//{
//    qDebug() << "Connected to server successfully";
//    isReceiving = true;
//    buffer.clear();
//    imageBytes = 0;
//    ui->stopButton->setEnabled(true);
//}

//void Client::onSocketError(QAbstractSocket::SocketError error)
//{
//    Q_UNUSED(error);
//    QMessageBox::critical(this, "连接错误",
//        QString("无法连接到服务器: %1").arg(socket->errorString()));
//    ui->startButton->setEnabled(true);
//}

//void Client::readFromServer()
//{
//    buffer.append(socket->readAll());

//    while (true) {
//        if (imageBytes == 0) {
//            if (buffer.size() < static_cast<int>(sizeof(quint32)))
//                return;

//            QDataStream stream(buffer);
//            stream >> imageBytes;
//            buffer.remove(0, sizeof(quint32));
//            qDebug() << "Expecting image size:" << imageBytes;
//        }

//        if (buffer.size() >= static_cast<int>(imageBytes)) {
//            QByteArray imageData = buffer.left(imageBytes);
//            buffer.remove(0, imageBytes);
//            imageBytes = 0;

//            QImage image;
//            if (image.loadFromData(imageData, "JPEG")) {
//                processFrame(image);
//            } else {
//                qDebug() << "Failed to decode image data";
//            }
//        } else {
//            break;
//        }
//    }
//}

//void Client::processFrame(const QImage &image)
//{
//    // 将 QImage 转换为 cv::Mat，确保颜色通道顺序正确
//    QImage rgbImage = image.convertToFormat(QImage::Format_RGB888);
//    cv::Mat frame(rgbImage.height(), rgbImage.width(), CV_8UC3,
//                 const_cast<uchar*>(rgbImage.bits()),
//                 rgbImage.bytesPerLine());

//    // 克隆数据以确保独立的内存空间
//    cv::Mat frameCopy = frame.clone();

//    // 运行推理
//    std::vector<Detection> output = inference->runInference(frameCopy);

//    // 绘制检测结果
//    cv::Mat displayFrame = frameCopy.clone();
//    int detections = output.size();

//    for (int i = 0; i < detections; ++i)
//    {
//        Detection detection = output[i];
//        cv::Rect box = detection.box;
//        cv::Scalar color = detection.color;

//        // 绘制检测框
//        cv::rectangle(displayFrame, box, color, 2);

//        // 绘制检测框文本
//        std::string classString = detection.className + ' ' + std::to_string(detection.confidence).substr(0, 4);
//        cv::Size textSize = cv::getTextSize(classString, cv::FONT_HERSHEY_DUPLEX, 1, 2, 0);
//        cv::Rect textBox(box.x, box.y - 40, textSize.width + 10, textSize.height + 20);

//        cv::rectangle(displayFrame, textBox, color, cv::FILLED);
//        cv::putText(displayFrame, classString, cv::Point(box.x + 5, box.y - 10),
//                   cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 2, 0);
//    }

//    // 转换回QImage显示
//    QImage resultImage(displayFrame.data, displayFrame.cols, displayFrame.rows,
//                      displayFrame.step, QImage::Format_RGB888);
//    displayImage(resultImage);
//}

//void Client::displayImage(const QImage &image)
//{
//    if (!image.isNull()) {
//        QPixmap pixmap = QPixmap::fromImage(image);
//        if (!pixmap.isNull()) {
//            videoLabel->setPixmap(pixmap.scaled(
//                videoLabel->size(),
//                Qt::KeepAspectRatio,
//                Qt::SmoothTransformation
//            ));
//        }
//    }
//}












//========================第一版代码==========================================

//#include "client.h"
//#include "ui_client.h"

//// Qt 核心模块
//#include <QHostAddress>
//#include <QMessageBox>
//#include <QImageReader>
//#include <QDebug>
//#include <QDataStream>
//#include <QFile>

//// 网络模块
//#include <QTcpSocket>
//#include <QAbstractSocket>

//Client::Client(QWidget *parent) :
//    QWidget(parent),
//    ui(new Ui::Client),
//    socket(new QTcpSocket(this)),
//    videoLabel(new QLabel(this)),
//    isReceiving(false),
//    imageBytes(0)
//{
//    ui->setupUi(this);

//    // UI设置
//    videoLabel->setAlignment(Qt::AlignCenter);
//    videoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    ui->videoLayout->addWidget(videoLabel);

//    setupConnections();
//}

//Client::~Client()
//{
//    socket->disconnectFromHost();
//    delete ui;
//}

//void Client::setupConnections()
//{
//    connect(ui->startButton, &QPushButton::clicked, this, &Client::on_startButton_clicked);
//    connect(ui->stopButton, &QPushButton::clicked, this, &Client::on_stopButton_clicked);
//    connect(socket, &QTcpSocket::readyRead, this, &Client::readFromServer);
//    connect(socket, &QTcpSocket::disconnected, this, [this](){
//        qDebug() << "Disconnected from server";
//        isReceiving = false;
//        ui->startButton->setEnabled(true);
//        ui->stopButton->setEnabled(false);
//    });
//    connect(socket, &QTcpSocket::connected, this, &Client::onSocketConnected);
//}

//void Client::on_startButton_clicked()
//{
//    if (socket->state() != QAbstractSocket::ConnectedState) {
//        socket->connectToHost("192.168.7.1", 8080);
//        ui->startButton->setEnabled(false); // 禁用按钮，防止重复点击
//    }
//}

//void Client::on_stopButton_clicked()
//{
//    if (socket->state() == QAbstractSocket::ConnectedState) {
//        socket->disconnectFromHost();
//    }
//    isReceiving = false;
//    ui->startButton->setEnabled(true);
//    ui->stopButton->setEnabled(false);
//}

//void Client::onSocketConnected()
//{
//    qDebug() << "Connected to server successfully";
//    isReceiving = true;
//    buffer.clear();
//    imageBytes = 0;
//    ui->stopButton->setEnabled(true);
//}

//void Client::onSocketError(QAbstractSocket::SocketError error)
//{
//    Q_UNUSED(error);
//    QMessageBox::critical(this, "连接错误",
//        QString("无法连接到服务器: %1").arg(socket->errorString()));
//    ui->startButton->setEnabled(true);
//}

//void Client::readFromServer()
//{
//    buffer.append(socket->readAll());

//    while (true) {
//        if (imageBytes == 0) {
//            if (buffer.size() < static_cast<int>(sizeof(quint32)))
//                return;

//            QDataStream stream(buffer);
//            stream >> imageBytes;
//            buffer.remove(0, sizeof(quint32));
//            qDebug() << "Expecting image size:" << imageBytes;
//        }

//        if (buffer.size() >= static_cast<int>(imageBytes)) {
//            QByteArray imageData = buffer.left(imageBytes);
//            buffer.remove(0, imageBytes);
//            imageBytes = 0;

//            QImage image;
//            if (image.loadFromData(imageData, "JPEG")) {
//                displayImage(image);
//            } else {
//                qDebug() << "Failed to decode image data";
//            }
//        } else {
//            break;
//        }
//    }
//}

//void Client::displayImage(const QImage &image)
//{
//    if (!image.isNull()) {
//        QPixmap pixmap = QPixmap::fromImage(image);
//        if (!pixmap.isNull()) {
//            videoLabel->setPixmap(pixmap.scaled(
//                videoLabel->size(),
//                Qt::KeepAspectRatio,
//                Qt::SmoothTransformation
//            ));
//        }
//    }
//}

//================================第一版代码===================================
