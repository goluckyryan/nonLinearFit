#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include "matrix.h"
#include "analysis.h"
#include "qcustomplot.h"
#include "fileio.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void Write2Log(QString str);

private:
    Ui::MainWindow *ui;

    QString Msg;

};

#endif // MAINWINDOW_H
