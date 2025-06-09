#include "cameraserver.h"
#include "ui_cameraserver.h"
#include <QDebug>


// Qt核心模块
#include <QBuffer>
#include <QMessageBox>
#include <QResizeEvent>
#include <QPixmap>
#include <QDataStream>
#include <QMutexLocker>

// 网络模块
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

CameraServer::CameraServer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CameraServer)
    , v4l2(new V4l2Api("/dev/video1", 4))
    , videoServer(new QTcpServer(this))
    , previewLabel(new QLabel(this))
{
    ui->setupUi(this);
    this->showFullScreen();

    qDebug() << "Application initialized";

    // UI设置-流畅
        previewLabel->setAlignment(Qt::AlignCenter);
        previewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        if (!ui->viewfinderWidget->layout()) {
            QVBoxLayout *layout = new QVBoxLayout(ui->viewfinderWidget);
            ui->viewfinderWidget->setLayout(layout);
        }
        ui->viewfinderWidget->layout()->addWidget(previewLabel);

    // ========== 信号槽连接 ==========
    connect(v4l2, &V4l2Api::sendImage, this, &CameraServer::displayImage);

    // ========== 网络服务器初始化 ==========
    connect(videoServer, &QTcpServer::newConnection, this, [this]() {
        qDebug() << "New connection arrived. Pending connections:"
                << videoServer->hasPendingConnections();

        while (videoServer->hasPendingConnections()) {
            QTcpSocket *client = videoServer->nextPendingConnection();
            QString clientInfo = QString("%1:%2").arg(client->peerAddress().toString())
                                                .arg(client->peerPort());
            {
                QMutexLocker locker(&clientMutex);
                clients.append(client);
                qDebug() << "Client connected:" << clientInfo
                        << "Total clients:" << clients.size();
            }

            connect(client, &QTcpSocket::disconnected, this, [this, client, clientInfo]() {
                QMutexLocker locker(&clientMutex);
                clients.removeAll(client);
                qDebug() << "Client disconnected:" << clientInfo
                        << "Remaining clients:" << clients.size();
                client->deleteLater();
            });
        }
    });


    qDebug() << "Network server initialized (not listening yet)";
}

CameraServer::~CameraServer()
{
    qDebug() << "Application shutdown started";
    delete v4l2;
    delete ui;
    qDebug() << "Resources released";
}

// ========== 按钮点击事件 ==========
void CameraServer::on_startButton_clicked()
{
    qDebug() << "Start button clicked";

    try {
        qDebug() << "Attempting to start server on 0.0.0.0:8080";

           if (!videoServer->listen(QHostAddress::Any, 8080)) {
               QString errorDetails = QString("Error code: %1\nError message: %2")
                                     .arg(videoServer->serverError())
                                     .arg(videoServer->errorString());
               qCritical() << "Server failed to start:\n" << errorDetails;

               QMessageBox::critical(this, "错误",
                   "无法启动视频服务器:\n" + errorDetails);
               return;
           }

           qDebug() << "Server successfully listening on:"
                   << videoServer->serverAddress().toString()
                   << "Port:" << videoServer->serverPort();

        ui->startButton->setEnabled(false);
        ui->stopButton->setEnabled(true);
        qDebug() << "UI state updated: Server running";

    } catch (...) {
        qCritical() << "Camera initialization failed";
        QMessageBox::critical(this, "错误", "摄像头初始化失败");
    }
}

void CameraServer::on_stopButton_clicked()
{
    qDebug() << "Stop button clicked";


    // 关闭客户端连接
    {
        QMutexLocker locker(&clientMutex);
        qDebug() << "Closing" << clients.size() << "client connections";

        for (QTcpSocket *client : clients) {
            QString clientInfo = QString("%1:%2").arg(client->peerAddress().toString())
                                                .arg(client->peerPort());
            qDebug() << "Disconnecting client:" << clientInfo;
            client->disconnectFromHost();
        }
        clients.clear();
    }

    // 关闭服务器
    qDebug() << "Closing server socket";
    videoServer->close();
    qDebug() << "Server listening status:" << videoServer->isListening();

    // 更新UI
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    qDebug() << "UI state updated: Server stopped";

    // 清空预览
    if (previewLabel) {
        previewLabel->clear();
        qDebug() << "Preview cleared";
    }
}

// ========== 图像处理 ==========
void CameraServer::displayImage(QImage image)
{
    if (image.isNull()) {
            qWarning() << "Received empty image frame";
            return;
        }

        if (!previewLabel) {
            qWarning() << "Preview label is not initialized";
            return;
        }

        // 获取全屏可用尺寸（排除可能的工具栏/状态栏）
        QSize screenSize = this->size();

        // 计算保持宽高比的最佳缩放尺寸
        QPixmap pixmap = QPixmap::fromImage(image);
        QPixmap scaledPixmap = pixmap.scaled(screenSize,
                                            Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation);

        // 设置预览标签
        previewLabel->setPixmap(scaledPixmap);
        previewLabel->setFixedSize(scaledPixmap.size());

        // 居中显示
        previewLabel->move((screenSize.width() - scaledPixmap.width())/2,
                          (screenSize.height() - scaledPixmap.height())/2);

    // 网络传输逻辑
    QByteArray jpegData;
    QBuffer buffer(&jpegData);
    if (buffer.open(QIODevice::WriteOnly)) {
        if (!image.save(&buffer, "JPEG", 80)) {
            qWarning() << "Failed to encode JPEG image";
        }
        buffer.close();
    }

    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream << quint32(jpegData.size());
    packet.append(jpegData);

    qDebug() << "Sending JPEG frame:"
            << "Size:" << jpegData.size()/1024 << "KB"
            << "Clients:" << clients.size();

    sendToAllClients(packet);
}

// ========== 网络辅助 ==========
void CameraServer::sendToAllClients(const QByteArray &data)
{
    QMutexLocker locker(&clientMutex);
    int successCount = 0;
    int totalClients = clients.size();

    for (QTcpSocket *client : clients) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            qint64 bytesWritten = client->write(data);
            if (bytesWritten == -1) {
                qWarning() << "Failed to write to client:"
                          << client->errorString();
            } else {
                successCount++;
                qDebug() << "Sent" << bytesWritten << "bytes to"
                        << client->peerAddress().toString();
            }
        }
    }

    qDebug() << "Frame delivery report:"
            << "Success:" << successCount
            << "Failed:" << (totalClients - successCount);
}

// ========== 窗口事件 ==========
void CameraServer::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    qDebug() << "Window resized to:" << event->size();

    if (previewLabel && previewLabel->pixmap()) {
        QSize newSize = ui->viewfinderWidget->size();
        qDebug() << "Adjusting preview to:" << newSize;
    }
}

void CameraServer::on_avButton_clicked()
{

    // 摄像头线程启动
    if (!v4l2->isRunning()) {
        qDebug() << "Starting camera thread";
        v4l2->start();
    }
}


void CameraServer::on_svButton_clicked()
{

    // 关闭摄像头线程
    qDebug() << "Requesting camera thread interruption";
    v4l2->requestInterruption();

    if (v4l2->isRunning()) {
        qDebug() << "Waiting for camera thread to finish (max 1 sec)";
        if (!v4l2->wait(1000)) {
            qWarning() << "Forcing camera thread termination";
            v4l2->terminate();
        }
    }
    qDebug() << "Camera thread status:" << v4l2->isRunning();

    this->hide();
    emit backForm();
}

