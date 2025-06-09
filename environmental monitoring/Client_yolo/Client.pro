QT       += core gui

QT       += core gui widgets multimedia multimediawidgets network


# Opencv4.8.0
INCLUDEPATH += /usr/local/include/opencv4
LIBS += -L/usr/local/lib \
        -lopencv_core \
        -lopencv_imgproc \
        -lopencv_highgui \
        -lopencv_videoio \
        -lopencv_imgcodecs \
        -lopencv_dnn \
        -lopencv_calib3d \
        -lopencv_features2d \
        -lopencv_flann

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    inference.cpp \
    main.cpp \
    client.cpp

HEADERS += \
    client.h \
    inference.h

FORMS += \
    client.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    yolov8m.onnx \
    yolov8n.onnx \
    yolov8s.onnx
