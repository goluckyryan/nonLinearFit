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

    void SetData(FileIO *file);
    void SetFrequency();
    void ContourPlot();


signals:
    void PlotData();

private slots:
    void on_pushButton_CalFFT_clicked();

    void on_checkBox_RemoveConstant_clicked();

    void on_horizontalSlider_freqH_sliderMoved(int position);
    void on_lineEdit_freqH_editingFinished();

    void on_pushButton_ApplyFilter_clicked();

    void on_pushButton_FFTWBackward_clicked();

    void on_horizontalSlider_freqL_sliderMoved(int position);

    void on_lineEdit_freqL_editingFinished();

    void on_pushButton_clicked();

    void on_radioButton_LowPassSharp_clicked();

    void on_radioButton_BandPassSharp_clicked();

    void on_radioButton_LowPass_clicked();

    void on_radioButton_Guassian_clicked();

private:
    Ui::FFTPlot *ui;

    QCustomPlot * plot_A;
    QCustomPlot * plot_P;
    QCPColorMap * colorMap_A;
    QCPColorMap * colorMap_P;

    FileIO *file;

    bool filterChanged;
    bool filterApplied;

    int filterID;

    double ExtractNumber(QString str);
};

#endif // FFTPLOT_H
