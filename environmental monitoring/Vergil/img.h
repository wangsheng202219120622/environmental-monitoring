#ifndef IMG_H
#define IMG_H

#include <QWidget>

namespace Ui {
class Img;
}

class Img : public QWidget
{
    Q_OBJECT

public:
    explicit Img(QWidget *parent = nullptr);
    ~Img();

private:
    Ui::Img *ui;
};

#endif // IMG_H
