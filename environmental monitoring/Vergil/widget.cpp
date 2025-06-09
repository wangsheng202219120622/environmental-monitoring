#include "widget.h"
#include "ui_widget.h"
#include <QDebug>


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    ,label_data(new QLabel(this))
    ,cameraserver(new CameraServer(this)), isOn1(false), isOn3(false), isOn5(false),bee(false),fstate(0)
{
    ui->setupUi(this);
    this->showFullScreen();
    //设备控制按钮初始值设置
     ui->pushButton->setStyleSheet(
    "QPushButton {"
    "font: 21pt 'Ubuntu';"
    "color: rgb(239, 41, 41);"
    "background-color: rgb(136, 138, 133);"
    "border-radius: 20px;"
    "}"
         );
ui->pushButton_3->setStyleSheet(
    "QPushButton {"
    "font: 21pt 'Ubuntu';"
    "color: rgb(239, 41, 41);"
    "background-color: rgb(136, 138, 133);"
    "border-radius: 20px;"
    "}"
         );
ui->pushButton_5->setStyleSheet(
    "QPushButton {"
    "font: 21pt 'Ubuntu';"
    "color: rgb(239, 41, 41);"
    "background-color: rgb(136, 138, 133);"
    "border-radius: 20px;"
    "}"
         );
ui->BeeButton_on->setStyleSheet(
    "QPushButton {"
    "font: 21pt 'Ubuntu';"
    "color: rgb(239, 41, 41);"
    "background-color: rgb(136, 138, 133);"
    "border-radius: 20px;"
    "}"
         );
ui->FanButton_1->setStyleSheet(
     "QPushButton {"
     "font: 21pt 'Ubuntu';"
     "color: rgb(239, 41, 41);"
     "background-color: rgb(136, 138, 133);"
     "border-radius: 20px;"
     "}"
     "QPushButton:hover {"
     "border-radius: 20px;"
     "}"
 );
    //设备初始值设置
    updateButtonStyle(ui->pushButton,  ui->label_35,false);
    updateButtonStyle(ui->pushButton_3, ui->label_36,false);
    updateButtonStyle(ui->pushButton_5, ui->label_37,false);
    updateBeeButtonStyle(ui->BeeButton_on, ui->label_18,false);
    updateFanButtonStyle(ui->FanButton_1, ui->label_21,0);



    //传感器
      light = new fsmpLight;
      temphum = new fsmpTempHum;

    //设备
    led=new fsmpLeds;
    fan=new fsmpFan;
    beeper=new fsmpBeeper;

    //监控
    cameraserver->hide();
    connect(cameraserver,&CameraServer::backForm,this,&Widget::show);

    time = new QTimer(this);
    // 初始化定时器，用于定时更新传感器数据
    sensorUpdateTimer = new QTimer(this);

    //获取实时时间
    connect(sensorUpdateTimer, &QTimer::timeout, this, &Widget::lightShow);
    connect(sensorUpdateTimer, &QTimer::timeout, this, &Widget::temperatureShow);
    connect(sensorUpdateTimer, &QTimer::timeout, this, &Widget::humidityShow);
    connect(sensorUpdateTimer, &QTimer::timeout, this, &Widget::updateTime);
    connect(ui->slider_Ctrol, &QSlider::valueChanged, [=](int value){

    });

    sensorUpdateTimer->start(1000);  // 每 1 秒更新一次

    // 超时则启动提交函数，达成自动检测目的
    connect(time, &QTimer::timeout, this, &Widget::on_btn_LEDsubmit_clicked);
    connect(time, &QTimer::timeout, this, &Widget::on_btn_FANsubmit_clicked);

    // 初始化智能模式状态为关闭
    m_smartModeEnabled = false;

    // 初始化 slider 和 textEdit 的默认值
    ui->textEdit->setPlainText("50");

    ui->textEdit_2->setPlainText("30");

    ui->textEdit_3->setPlainText("10");

    ui->textEdit_4->setPlainText("25");

    ui->textEdit_Hum->setPlainText("0");

    // 设置QTextEdit与slider的关联
    setupTextEditConnections();

    // 连接slider的valueChanged信号到自定义槽函数
    connect(ui->slider_Ctrol, &QSlider::valueChanged, this, &Widget::onSliderValueChanged);


}

Widget::~Widget()
{
    delete ui;
    delete sensorUpdateTimer;
    delete light;
    delete temphum;
}

/********环境监测***************/

void Widget::lightShow()
{
    double num = light->getValue();

    // ui界面lcdnumber显示光照数字
    ui->lcdNumber->display(num);
    qDebug() << "light:" << num;
}

void Widget::temperatureShow()
{
    double num = temphum->temperature();

    // ui界面lcdnumber显示光照数字
    ui->lcdNumber_2->display(num);
    qDebug() << "temperature:" << num;
}

void Widget::humidityShow()
{
    double num = temphum->humidity();

    // ui界面lcdnumber显示光照数字
    ui->lcdNumber_3->display(num);
    qDebug() << "humidity:" << num;
}

void Widget::updateTime()
{
    // 获取当前时间
    QDateTime current = QDateTime::currentDateTime();
    //修改时间误差
    current = current.addYears(1);
    current =current.addDays(16);
    current =current.addSecs(48633);//13h-30m-33s

    // 设置标签的文本为当前时间
    ui->label_data->setText(current.toString("yyyy-MM-dd HH:mm:ss"));

}
/*******************************/

/*************设备控制************/
void Widget::on_MonButton_clicked()
{
    cameraserver->show();
    this->hide();
}

void Widget::on_pushButton_clicked()
{
    isOn1 = !isOn1;
    if(isOn1)
    {
        led->on(fsmpLeds::LED1);
    }
    else
    {
        led->off(fsmpLeds::LED1);
    }
    updateButtonStyle(ui->pushButton,ui->label_35, isOn1);
}


void Widget::on_pushButton_3_clicked()
{
    isOn3 = !isOn3;
    if(isOn3)
    {
        led->on(fsmpLeds::LED2);
    }
    else
    {
        led->off(fsmpLeds::LED2);
    }
    updateButtonStyle(ui->pushButton_3, ui->label_36, isOn3);
}


void Widget::on_pushButton_5_clicked()
{
    isOn5 = !isOn5;
    if(isOn5)
    {
        led->on(fsmpLeds::LED3);
    }
    else
    {
        led->off(fsmpLeds::LED3);
    }
    updateButtonStyle(ui->pushButton_5, ui->label_37, isOn5);
}


void Widget::on_FanButton_1_clicked()
{

    fstate = fstate + 1; // 循环状态 0, 1, 2, 3
    if(fstate==4)
    {
        fstate=0;
    }
    updateFanButtonStyle(ui->FanButton_1,ui->label_21,fstate);
    if(fstate==1)
    {
        fan->setSpeed(100);
        fan->start();
    }
    else if(fstate==2)
    {
        fan->setSpeed(180);
        fan->start();
    }
    else if(fstate==3)
    {
        fan->setSpeed(254);
        fan->start();

    }
    else if(fstate==0)
    {
      fan->stop();

    }

}


void Widget::on_BeeButton_on_clicked()
{
    beeper->setRate(1000);
    beeper->start();

    bee= !bee;
    if(bee)
    {
        beeper->setRate(1000);
        beeper->start();
    }
    else
    {
        beeper->stop();
    }
    updateBeeButtonStyle(ui->BeeButton_on, ui->label_18, bee);
}



void Widget::on_CloseButton_clicked()
{
    this->close();
}


/********************状态判断******************************/
void Widget::updateButtonStyle(QPushButton* button,QLabel* label, bool state)
{
    if (state) {
        button->setStyleSheet(button->styleSheet() + "QPushButton { border-image: url(:/12.jpg); }");
       label->setStyleSheet(label->styleSheet() + "QLabel { border-image: url(:/led1.jpg); }");
    } else {
        button->setStyleSheet(button->styleSheet() + "QPushButton { border-image: url(:/13.jpg); }");
         label->setStyleSheet(label->styleSheet() + "QLabel { border-image: url(:/led0.jpg); }");
    }
}

void Widget::updateBeeButtonStyle(QPushButton* button,QLabel* label, bool state)
{
    if (state) {
        button->setStyleSheet(button->styleSheet() + "QPushButton { border-image: url(:/12.jpg); }");
       label->setStyleSheet(label->styleSheet() + "QLabel { border-image: url(:/bee1.jpg); }");
    } else {
        button->setStyleSheet(button->styleSheet() + "QPushButton { border-image: url(:/13.jpg); }");
         label->setStyleSheet(label->styleSheet() + "QLabel { border-image: url(:/bee0.jpg); }");
    }
}

void Widget::updateFanButtonStyle(QPushButton* button, QLabel* label, int state)
{
    switch (state) {
        case 0:
            button->setStyleSheet(button->styleSheet() + "QPushButton { border-image: url(:/0.jpg); }");
            label->setStyleSheet(label->styleSheet() + "QLabel{ border-image: url(:/fan0.jpg); }");
            break;
        case 1:
            button->setStyleSheet(button->styleSheet() + "QPushButton { border-image: url(:/1.jpg); }");
             label->setStyleSheet(label->styleSheet() + "QLabel{ border-image: url(:/fan1.jpg); }");
            break;
        case 2:
            button->setStyleSheet(button->styleSheet() + "QPushButton { border-image: url(:/2.jpg); }");
             label->setStyleSheet(label->styleSheet() + "QLabel{ border-image: url(:/fan2.jpg); }");
            break;
        case 3:
            button->setStyleSheet(button->styleSheet() + "QPushButton { border-image: url(:/3.jpg); }");
            label->setStyleSheet(label->styleSheet() + "QLabel{ border-image: url(:/fan3.jpg); }");
            break;
    }
}



/*****************************/

/************阈值设定与智能控制******************/

void Widget::setupTextEditConnections()
{
    // 为所有需要关联的QTextEdit安装事件过滤器
    ui->textEdit->installEventFilter(this);
    ui->textEdit_2->installEventFilter(this);
    ui->textEdit_3->installEventFilter(this);
    ui->textEdit_4->installEventFilter(this);
    ui->textEdit_Hum->installEventFilter(this);
}

// 事件过滤器实现
bool Widget::eventFilter(QObject* obj, QEvent* event)
{
    // 检查是否是QTextEdit的焦点事件
    if (event->type() == QEvent::FocusIn) {
        // 将发送事件的对象转换为QTextEdit
        QTextEdit* textEdit = qobject_cast<QTextEdit*>(obj);
        if (textEdit) {
            // 更新当前焦点控件
            m_currentTextEdit = textEdit;

            // 更新slider的值为当前文本框的值
            bool ok;
            int value = textEdit->toPlainText().toInt(&ok);
            if (ok) {
                ui->slider_Ctrol->setValue(value);
            }
        }
    }
    // 将事件继续传递给原始接收者
    return QObject::eventFilter(obj, event);
}

// 处理slider值变化的槽函数
void Widget::onSliderValueChanged(int value)
{
    // 如果有当前焦点的QTextEdit，则更新其值
    if (m_currentTextEdit) {
        m_currentTextEdit->setPlainText(QString::number(value));
    }
}


// LED光照强度 提交
void Widget::on_btn_LEDsubmit_clicked()
{
    // 获取三级光照强度
    double power_first_light = ui->textEdit->toPlainText().toDouble();
    double power_seconde_light = ui->textEdit_2->toPlainText().toDouble();
    double power_third_light = ui->textEdit_3->toPlainText().toDouble();

    qDebug() << "power_first_light = " << power_first_light
             << ", power_seconde_light = " << power_seconde_light
             << ", power_third_light = " << power_third_light;
    // 只有当总开关开启时才控制LED
    if (m_smartModeEnabled)
    {
        // 获得当前光照强度
        double cur_light = light->getValue();
        if (cur_light < power_third_light)
        {
            led->on(fsmpLeds::LED1);
            led->on(fsmpLeds::LED2);
            led->on(fsmpLeds::LED3);
        }
        else if (cur_light < power_seconde_light)
        {
            led->on(fsmpLeds::LED1);
            led->on(fsmpLeds::LED2);
            led->off(fsmpLeds::LED3);
        }
        else if (cur_light < power_first_light)
        {
            led->on(fsmpLeds::LED1);
            led->off(fsmpLeds::LED2);
            led->off(fsmpLeds::LED3);
        }
        else
        {
            led->off(fsmpLeds::LED1);
            led->off(fsmpLeds::LED2);
            led->off(fsmpLeds::LED3);
        }
    }
}

// FAN开启温度 提交
void Widget::on_btn_FANsubmit_clicked()
{
    // 获取设定温度与当前温度
    double set_tem = ui->textEdit_4->toPlainText().toDouble();
    double cur_tem = temphum->temperature();
    qDebug() << "set_tem = " << set_tem << "cur_tem = " << cur_tem;
    // 如果总开关开启,就启动
    if (m_smartModeEnabled)
    {
        // 如果当前温度高于设定温度，就开启风扇（默认100转速)
        if (set_tem < cur_tem)
        {
            fan->setSpeed(100);
            fan->start();
        }
        else fan->stop();
    }

    double cur_hum = temphum->humidity();
    double set_hum = ui->textEdit_Hum->toPlainText().toDouble();
    qDebug() << "set_hum = " << set_hum << "cur_hum = " << cur_hum;
    // 如果总开关开启，则检测
    if (m_smartModeEnabled)
    {
        if (cur_hum > set_hum){
            beeper->setRate(1000);
            beeper->start(); }
        else beeper->stop();
    }
}

//void Widget::on_btn_HUMsubmit_clicked()
//{

//}

// 智能检测开启
void Widget::on_radioButton_SmartOn_clicked()
{

    // 智能检测总开关开启
    m_smartModeEnabled = true;
    // 启动定时器，设置定时器的时长为 5000ms
    time->start(1000);
}

// 智能检测关闭
void Widget::on_radioButton_SmartOff_clicked()
{

    // 停止定时器，智能检测总开关关闭
    time->stop();
    m_smartModeEnabled = false;
    // LED灯关闭
    led->off(fsmpLeds::LED1);
    led->off(fsmpLeds::LED2);
    led->off(fsmpLeds::LED3);
    // 风扇关闭
    fan->stop();
    // 蜂鸣器关闭
    beeper->stop();
}


/********************************************/
