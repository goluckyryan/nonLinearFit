#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QTextStream>
#include <QFile>
#include <QFileDialog>
#include <QVector>
#include <QDebug>
#include <QScrollBar>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void OpenRow(QString fileName);


public slots:
    void Write2Log(QString str);
private slots:

    void on_pushButton_Open_clicked();
    void on_pushButton_Convert_clicked();

    double FindMax(QVector<double> vec);
    double FindMin(QVector<double> vec);
    double ExtractYValue(QString str);
private:
    Ui::MainWindow *ui;

    QVector<double> xData, yData;
    QVector<double> *zData;

    int xSize, ySize;

};

#endif // MAINWINDOW_H
