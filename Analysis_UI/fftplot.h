#ifndef FFTPLOT_H
#define FFTPLOT_H

#include <QDialog>
#include "qcustomplot.h"
#include "fileio.h"

namespace Ui {
class FFTPlot;
}

class FFTPlot : public QDialog
{
    Q_OBJECT

public:
    explicit FFTPlot(QWidget *parent = 0);
    ~FFTPlot();

    void SetFrequency(double xMin, double xMax, double yMin, double yMax);
    void ContourPlot(int nx, int ny, QVector<double> *dataA, QVector<double> *dataP);

signals:
    void CalFFTWForward();
    void CalFFTWBackward();
    void RemoveYConstant();
    void MovingAvgOnFFTW();
    void ApplyFilters(int filterID);

private slots:
    void on_pushButton_CalFFT_clicked();

private:
    Ui::FFTPlot *ui;

    QCustomPlot * plot_A;
    QCustomPlot * plot_P;
    QCPColorMap * colorMap_A;
    QCPColorMap * colorMap_P;
};

#endif // FFTPLOT_H
