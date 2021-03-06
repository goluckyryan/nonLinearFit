#-------------------------------------------------
#
# Project created by QtCreator 2016-09-17T07:14:00
#
#-------------------------------------------------

QT       += core gui sql script

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = Analysis_UI
TEMPLATE = app

RC_ICONS += icon.ico

win32 {
    ## Windows common build here

    !contains(QMAKE_TARGET.arch, x86_64) {
        ## Windows x86 (32bit) specific build here
        LIBS += "$$PWD/libfftw3-3.dll"
        message("x86 build")

    } else {
        ## Windows x64 (64bit) specific build here
        LIBS += "$$PWD/libfftw3-3_x64.lib"
        message("x86_64 build")

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
    waveletplot.cpp \
    openfiledelegate.cpp \
    dateformatdelegate.cpp \
    databasewindow.cpp \
    constant.cpp

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
    waveletplot.h \
    openfiledelegate.h \
    dateformatdelegate.h \
    databasewindow.h

FORMS    += mainwindow.ui \
    bplot.ui \
    fitresult.ui \
    fftplot.ui \
    waveletplot.ui \
    databasewindow.ui

DISTFILES += \
    icon.ico

RESOURCES += \
    resource.qrc
