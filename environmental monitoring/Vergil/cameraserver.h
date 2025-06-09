#ifndef CAMERASERVER_H
#define CAMERASERVER_H

#include <QMainWindow>
#include <QLabel>

#include <QTcpServer>
#include <QTcpSocket>

#include <QList>
#include <QMutex>
#include <QMutexLocker>

#include "hal_mp1a_v4l2api.h"


namespace Ui {
class CameraServer;
}

class CameraServer : public QMainWindow
{
    Q_OBJECT

public:
    explicit CameraServer(QWidget *parent = nullptr);
    ~CameraServer();

signals:
    void backForm();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void displayImage(QImage image);

    void on_avButton_clicked();

    void on_svButton_clicked();

private:
    Ui::CameraServer *ui;
    V4l2Api *v4l2;
    QLabel *previewLabel;
    QTcpServer *videoServer;
    QList<QTcpSocket*> clients; // 客户端列表
    QMutex clientMutex; // 线程安全锁

    // 新增私有方法
    void sendToAllClients(const QByteArray &data);
};

#endif // CAMERASERVER_H
