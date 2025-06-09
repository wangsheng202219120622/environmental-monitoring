#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QTcpSocket>
#include <QImage>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class Client; }
QT_END_NAMESPACE

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

private:
    Ui::Client *ui;
    QTcpSocket *socket;
    QLabel *videoLabel;
    QByteArray buffer;
    quint32 imageBytes;
    bool isReceiving;

    void displayImage(const QImage &image);
    void setupConnections();
};

#endif // CLIENT_H
