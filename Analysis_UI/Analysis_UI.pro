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

win32 {

    ## Windows common build here

    !contains(QMAKE_TARGET.arch, x86_64) {
        message("x86 build")
        LIBS += "$$PWD/libfftw3-3.dll"
        ## Windows x86 (32bit) specific build here

    } else {
        message("x86_64 build")
        LIBS += "$$PWD/libfftw3-3_x64.lib"
        ## Windows x64 (64bit) specific build here

    }
}

unix:LIBS += "/usr/local/lib/libfftw3.a"

SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    matrix.cpp \
    analysis.cpp \
    fileio.cpp \
    bplot.cpp \
    fitresult.cpp \
    fftplot.cpp \
    waveletanalysis.cpp \
    waveletplot.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    matrix.h \
    analysis.h \
    fileio.h \
    constant.h \
    bplot.h \
    fitresult.h \
    fftw3.h \
    fftplot.h \
    waveletanalysis.h \
    waveletplot.h

FORMS    += mainwindow.ui \
    bplot.ui \
    fitresult.ui \
    fftplot.ui \
    waveletplot.ui

DISTFILES += \
    icon.ico
