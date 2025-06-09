


//=================================第三版



#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QTcpSocket>
#include <QImage>
#include <QLabel>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "inference.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Client; }
QT_END_NAMESPACE

class FrameProcessor : public QObject
{
    Q_OBJECT
public:
    explicit FrameProcessor(Inference* inference, QObject* parent = nullptr);
    ~FrameProcessor();

public slots:
    void processFrame(const QImage &frame);

signals:
    void frameProcessed(const QImage &result);

private:
    Inference* m_inference;
    QMutex m_mutex;
};

class Client : public QWidget
{
    Q_OBJECT

public:
    explicit Client(QWidget *parent = nullptr);
    ~Client();

private slots:
    void readFromServer();
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void onSocketConnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void displayProcessedFrame(const QImage &image);

private:
    Ui::Client *ui;
    QTcpSocket *socket;
    QLabel *videoLabel;
    QByteArray buffer;
    quint32 imageBytes;
    bool isReceiving;

    Inference *inference;
    QThread *processorThread;
    FrameProcessor *frameProcessor;

    void setupConnections();
    void cleanup();
};

#endif // CLIENT_H






//======================================第二版代码-可以检测-较慢======================

//#ifndef CLIENT_H
//#define CLIENT_H

//#include <QWidget>
//#include <QTcpSocket>
//#include <QImage>
//#include <QLabel>
//#include "inference.h" // 引入 Inference 类

//QT_BEGIN_NAMESPACE
//namespace Ui { class Client; }
//QT_END_NAMESPACE

//class Client : public QWidget
//{
//    Q_OBJECT

//public:
//    explicit Client(QWidget *parent = nullptr);
//    ~Client();

//private slots:
//    void readFromServer();
//    void on_startButton_clicked();
//    void on_stopButton_clicked();
//    void onSocketConnected();
//    void onSocketError(QAbstractSocket::SocketError error);

//private:
//    Ui::Client *ui;
//    QTcpSocket *socket;
//    QLabel *videoLabel;
//    QByteArray buffer;
//    quint32 imageBytes;
//    bool isReceiving;

//    Inference *inference; // 增加 Inference 对象
//    void displayImage(const QImage &image);
//    void setupConnections();
//    void processFrame(const QImage &image); // 新增处理帧的方法
//};

//#endif // CLIENT_H




//==============================第一版代码===========================
//#ifndef CLIENT_H
//#define CLIENT_H

//#include <QWidget>
//#include <QTcpSocket>
//#include <QImage>
//#include <QLabel>

//QT_BEGIN_NAMESPACE
//namespace Ui { class Client; }
//QT_END_NAMESPACE

//class Client : public QWidget
//{
//    Q_OBJECT

//public:
//    explicit Client(QWidget *parent = nullptr);
//    ~Client();

//private slots:
//    void readFromServer();
//    void on_startButton_clicked();
//    void on_stopButton_clicked();
//    void onSocketConnected();
//    void onSocketError(QAbstractSocket::SocketError error);

//private:
//    Ui::Client *ui;
//    QTcpSocket *socket;
//    QLabel *videoLabel;
//    QByteArray buffer;
//    quint32 imageBytes;
//    bool isReceiving;

//    void displayImage(const QImage &image);
//    void setupConnections();
//};

//#endif // CLIENT_H
//==========================================================================
