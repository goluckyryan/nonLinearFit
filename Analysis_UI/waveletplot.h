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
    void PlotWVoctave(int octave);
    void PlotEnergy();

signals:
    void SendMsg(QString msg);
    void Replot();
    //void ApplyWTtoALL();

private slots:
    void plotXAxisChanged(QCPRange range);
    void ShowMousePosition(QMouseEvent* mouse);
    void SetLineByMouseClick(QMouseEvent* mouse);

    void on_verticalSlider_Threshold_valueChanged(int value);
    void on_verticalSlider_Octave_valueChanged(int value);

    void on_pushButton_ApplyThreshold_clicked();
    void on_pushButton_Clean_clicked();

    void on_lineEdit_Octave_editingFinished();
    void on_lineEdit_Threshold_editingFinished();

    void on_comboBox_Wavelet_currentIndexChanged(int index);
    void on_spinBox_WaveletIndex_valueChanged(int arg1);
    void on_comboBox_Thresholding_currentIndexChanged(int index);

    void on_lineEdit_x1_editingFinished();
    void on_lineEdit_x2_editingFinished();

    void on_pushButton_ApplyToAll_clicked();

    void on_checkBox_normalized_clicked(bool checked);

    void on_pushButton_PlotID_clicked();

private:
    Ui::WaveletPlot *ui;

    QCustomPlot * plot_W;
    QCPColorMap * colorMap_W;

    QCustomPlot * plot_V;
    QCPColorMap * colorMap_V;

    QCustomPlot * plot;
    QCustomPlot * plot_Woct;
    QCustomPlot * plot_Voct;
    QCustomPlot * plot_energy;

    WaveletAnalysis * wave;

    FileIO *file;
    int yIndex;

    bool enableVerticalBar;
    bool enableSpinBoxWaveletIndex;
    bool enableControl;

    double x1, x2;

};

#endif // WAVELETPLOT_H
