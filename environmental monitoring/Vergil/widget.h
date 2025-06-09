#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>


#include "fsmpLight.h"  //光
#include "fsmpTempHum.h"    //温湿度
#include "fsmpLed.h"
#include "fsmpFan.h"
#include "fsmpBeeper.h"
#include <QTimer>       //定时器类
#include <QDateTime>    //日期
#include <QLabel>       //日期


#include"cameraserver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:

    //光、温湿度
    void lightShow();
    void temperatureShow();
    void humidityShow();

    //日期
    void updateTime();

    /*********阈值设置与智能监控**********/

    void on_btn_LEDsubmit_clicked();

    void on_btn_FANsubmit_clicked();

    void on_radioButton_SmartOn_clicked();

    void on_radioButton_SmartOff_clicked();

//    void on_btn_HUMsubmit_clicked();
    /***********************************/

    /**********设备控制与监控跳转***********/

    void on_pushButton_clicked();

    void on_FanButton_1_clicked();

    void on_BeeButton_on_clicked();

    void on_MonButton_clicked();

    void on_CloseButton_clicked();

    void on_pushButton_3_clicked();


    void on_pushButton_5_clicked();


    /********************************/

private:
    bool m_smartModeEnabled;

    QTextEdit* m_currentTextEdit = nullptr;  // 跟踪当前获得焦点的QTextEdit
    bool eventFilter(QObject* obj, QEvent* event) override;  // 事件过滤器

    void setupTextEditConnections();
    void onSliderValueChanged(int value);

    Ui::Widget *ui;

    //光、温湿度
    fsmpLight *light;
    fsmpTempHum *temperature;
    fsmpTempHum *humidity;
    fsmpTempHum *temphum;

    //设备
    fsmpLeds *led;
    fsmpFan *fan;
    fsmpBeeper *beeper;

    //定时器
    QTimer *time;
    QTimer *sensorUpdateTimer;

    //日期
    QLabel *label_data;
   // QTimer *timer;

   //监控
    CameraServer *cameraserver;

    //判断

        bool isOn1;
        bool isOn3;
        bool isOn5;
        bool bee;
        int fstate;
        void updateButtonStyle(QPushButton* button, QLabel* label,bool state);
        void updateBeeButtonStyle(QPushButton* button, QLabel* label,bool state);
        void updateFanButtonStyle(QPushButton* button, QLabel* lable, int state);


};
#endif // WIDGET_H
