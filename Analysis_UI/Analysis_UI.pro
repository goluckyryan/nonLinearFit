#-------------------------------------------------
#
# Project created by QtCreator 2016-09-17T07:14:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = Analysis_UI
TEMPLATE = app

win32:RC_ICONS += icon.ico

SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    matrix.cpp \
    analysis.cpp \
    fileio.cpp \
    dialog.cpp \
    bplot.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    matrix.h \
    analysis.h \
    fileio.h \
    constant.h \
    dialog.h \
    bplot.h

FORMS    += mainwindow.ui \
    dialog.ui \
    bplot.ui

DISTFILES += \
    icon.ico
