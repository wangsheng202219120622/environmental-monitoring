#include "img.h"
#include "ui_img.h"

Img::Img(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Img)
{
    ui->setupUi(this);
}

Img::~Img()
{
    delete ui;
}
