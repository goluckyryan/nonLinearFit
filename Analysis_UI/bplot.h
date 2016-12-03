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

    void ShowPlotValue(QMouseEvent* mousePos1);

    void FindPeak(QVector<double> x, QVector<double> y);
    void FindZeros(QString type, QVector<double> x, QVector<double> y);

    void SetYStart(QMouseEvent * mouse);
    void SetYEnd(QMouseEvent * mouse);

    void RemovePlotItems(){
        plot->clearItems();
        zeros.clear();
        peaks.clear();
        plot->replot();
    }

    void AddArrows();
    void on_pushButton_ClearArrows_clicked();

private:
    Ui::BPlot *ui;

    QCustomPlot * plot;

    FileIO *file;

    QVector<double> x, y;
    double yMin, yMax;
    int plotUnit;

    QVector<double> zeros;
    QVector<double> peaks;

    int mouseYIndex1;
    int mouseYIndex2;

};

#endif // BPLOT_H
