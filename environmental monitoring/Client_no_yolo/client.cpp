#include "client.h"
#include "ui_client.h"

// Qt 核心模块
#include <QHostAddress>
#include <QMessageBox>
#include <QImageReader>
#include <QDebug>
#include <QDataStream>
#include <QFile>

// 网络模块
#include <QTcpSocket>
#include <QAbstractSocket>

Client::Client(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Client),
    socket(new QTcpSocket(this)),
    videoLabel(new QLabel(this)),
    isReceiving(false),
    imageBytes(0)
{
    ui->setupUi(this);

    // UI设置
    videoLabel->setAlignment(Qt::AlignCenter);
    videoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->videoLayout->addWidget(videoLabel);

    setupConnections();
}

Client::~Client()
{
    socket->disconnectFromHost();
    delete ui;
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
}

void Client::on_startButton_clicked()
{
    if (socket->state() != QAbstractSocket::ConnectedState) {
        socket->connectToHost("192.168.7.1", 8080);
        ui->startButton->setEnabled(false); // 禁用按钮，防止重复点击
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
                displayImage(image);
            } else {
                qDebug() << "Failed to decode image data";
            }
        } else {
            break;
        }
    }
}

void Client::displayImage(const QImage &image)
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
