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
    void SetPlots();
    void ContourPlot();

public slots:

signals:
    void PlotData();

private slots:
    void EnablePlanels(bool IO);

    void on_pushButton_CalFFT_clicked();

    void on_checkBox_RemoveConstant_clicked();
    void on_checkBox_Reverse_clicked(bool checked);

    void on_horizontalSlider_freqH_valueChanged(int value);
    void on_horizontalSlider_freqL_valueChanged(int value);
    void on_lineEdit_freqH_editingFinished();
    void on_lineEdit_freqL_editingFinished();

    void on_pushButton_ResetPlot_clicked();
    void on_pushButton_ApplyFilter_clicked();
    void on_pushButton_FFTWBackward_clicked();

    void on_radioButton_LowPassSharp_clicked();
    void on_radioButton_LowPass_clicked();
    void on_radioButton_BandPassSharp_clicked();
    void on_radioButton_Guassian_clicked();

    void on_checkBox_LogY_clicked(bool checked);

    void on_checkBox_toAP_clicked(bool checked);

private:
    Ui::FFTPlot *ui;

    QCustomPlot * plot_X;
    QCustomPlot * plot_A;
    QCustomPlot * plot_P;
    QCPColorMap * colorMap_A;
    QCPColorMap * colorMap_P;

    FileIO *file;

    QVector<double> filterFunc;

    bool filterChanged;
    bool filterApplied;
    bool enableSliderFunction;

    int filterID;

    double ExtractNumber(QString str);
};

#endif // FFTPLOT_H
