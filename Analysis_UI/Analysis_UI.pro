#-------------------------------------------------
#
# Project created by QtCreator 2016-09-17T07:14:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = Analysis_UI
TEMPLATE = app

RC_ICONS += icon.ico
win32:LIBS += "$$PWD/libfftw3-3.dll"
#unix:INCLUDEPATH += "/usr/local/lib"
unix:LIBS += "/usr/local/lib/libfftw3.a"

SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    matrix.cpp \
    analysis.cpp \
    fileio.cpp \
    bplot.cpp \
    fitresult.cpp \
    fftplot.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    matrix.h \
    analysis.h \
    fileio.h \
    constant.h \
    bplot.h \
    fitresult.h \
    fftw3.h \
    fftplot.h

FORMS    += mainwindow.ui \
    bplot.ui \
    fitresult.ui \
    fftplot.ui

DISTFILES += \
    icon.ico
