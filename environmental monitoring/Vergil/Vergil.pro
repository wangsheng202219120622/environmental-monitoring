QT       += core gui
QT       += core gui widgets multimedia multimediawidgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cameraserver.cpp \
    hal_mp1a_v4l2api.cpp \
    main.cpp \
    widget.cpp

HEADERS += \
    cameraserver.h \
    fsmpBeeper.h \
    fsmpFan.h \
    fsmpLed.h \
    fsmpLight.h \
    fsmpTempHum.h \
    hal_mp1a_v4l2api.h \
    widget.h

FORMS += \
    cameraserver.ui \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    img.qrc

DISTFILES += \
    pop.jpeg
