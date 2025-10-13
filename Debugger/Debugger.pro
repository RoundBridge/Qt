QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    actuator.cpp \
    attitude.cpp \
    controller.cpp \
    crc.c \
    end.cpp \
    joint.cpp \
    link.cpp \
    main.cpp \
    mainwindow.cpp \
    operate.cpp \
    operate_joint.cpp \
    state.cpp

HEADERS += \
    actuator.h \
    attitude.h \
    common.h \
    controller.h \
    crc.h \
    end.h \
    joint.h \
    link.h \
    mainwindow.h \
    operate.h \
    operate_joint.h \
    state.h

FORMS += \
    mainwindow.ui \
    operate.ui \
    operate_joint.ui \
    state.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

QT += network
