#ifndef BPLOT_H
#define BPLOT_H

#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include "qcustomplot.h"
#include "fileio.h"

namespace Ui {
class BPlot;
}

class BPlot : public QDialog
{
    Q_OBJECT

public:
    explicit BPlot(QWidget *parent = 0);
    ~BPlot();

    void SetData(FileIO *file);
    void SetPlotUnit(int unit){ this->plotUnit = unit;}
    void SetupPlot(double xMin, double xMax);
    void Plot();
    int FindstartIndex(QVector<double> xdata, double goal);    

signals:
    void SendMsg(QString msg);

public slots:
    void on_pushButton_Print_clicked();

private slots:
    void on_spinBox_Start_valueChanged(int arg1);
    void on_spinBox_End_valueChanged(int arg1);
    void on_pushButton_clicked();

    void ShowPlotValue(QMouseEvent* mouse);


private:
    Ui::BPlot *ui;

    QCustomPlot * plot;

    FileIO *file;

    QVector<double> x, y;
    int plotUnit;

};

#endif // BPLOT_H
