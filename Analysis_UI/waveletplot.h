#ifndef WAVELETPLOT_H
#define WAVELETPLOT_H

#include <QMainWindow>
#include "waveletanalysis.h"
#include "qcustomplot.h"
#include "fileio.h"
#include <QVector>

namespace Ui {
class WaveletPlot;
}

class WaveletPlot : public QMainWindow
{
    Q_OBJECT

public:
    explicit WaveletPlot(QWidget *parent = 0);
    ~WaveletPlot();

    void SetData(FileIO *file, int yIndex);
    void PlotWV();
    void PlotReconstructedData(bool Original = 0);
    void PlotWVoct(int s);

signals:
    void SendMsg(QString msg);
    void Replot();

private slots:

    void on_verticalSlider_valueChanged(int value);
    void on_verticalSlider_Scale_valueChanged(int value);

    void on_ApplyHT_clicked();

    void ShowMousePosition(QMouseEvent* mouse);
    void SetLineByMouseClick(QMouseEvent* mouse);

    void on_pushButton_Clean_clicked();

    void on_lineEdit_sLimit_editingFinished();
    void on_lineEdit_HT_editingFinished();

    void on_comboBox_Wavelet_currentIndexChanged(int index);
    void on_spinBox_WaveletIndex_valueChanged(int arg1);
    void on_comboBox_Thresholding_currentIndexChanged(int index);

private:
    Ui::WaveletPlot *ui;

    QCustomPlot * plot_W;
    QCPColorMap * colorMap_W;

    QCustomPlot * plot_V;
    QCPColorMap * colorMap_V;

    QCustomPlot * plot;
    QCustomPlot * plot_Woct;
    QCustomPlot * plot_Voct;

    WaveletAnalysis * wave;

    FileIO *file;
    int yIndex;

    bool enableVerticalBar;

    double x1, x2;

};

#endif // WAVELETPLOT_H
