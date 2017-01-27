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
    void RescalePlots();
    void CalFilterFunctionX();
    void CalFilterFunctionY();

public slots:

signals:
    void PlotData();
    void SendMsg(QString msg);

private slots:
    void EnablePlanels(bool IO);

    void on_pushButton_CalFFT_clicked();

    void on_checkBox_RemoveConstant_clicked();

    void on_checkBox_Reverse_clicked(bool checked, bool plot = 1);
    void on_checkBox_EnableY_clicked(bool checked, bool plot = 1);

    void on_horizontalSlider_freqH_valueChanged(int value);
    void on_horizontalSlider_freqL_valueChanged(int value);
    void on_lineEdit_XfreqH_editingFinished();
    void on_lineEdit_XfreqL_editingFinished();

    void on_pushButton_ResetPlot_clicked();
    void on_pushButton_ApplyFilter_clicked();
    void on_pushButton_FFTWBackward_clicked();

    void on_radioButton_LowPassSharp_clicked(bool plot = 1);
    void on_radioButton_LowPass_clicked();
    void on_radioButton_BandPassSharp_clicked();
    void on_radioButton_Guassian_clicked();

    void on_checkBox_LogY_clicked(bool checked);
    void on_checkBox_toAP_clicked();

    void on_verticalSlider_freqH_valueChanged(int value);
    void on_verticalSlider_freqL_valueChanged(int value);
    void on_lineEdit_YfreqH_editingFinished();
    void on_lineEdit_YfreqL_editingFinished();

    void GetPosOnPlotA(QMouseEvent * mouse);
    void GetPosOnPlotP(QMouseEvent * mouse);
    void GetPosOnPlotX(QMouseEvent * mouse);
    void ShowLinesOnPlotA(QPoint pt);
    void ShowLinesOnPlotP(QPoint pt);
    void ShowLinesOnPlotX(QPoint pt);

private:
    Ui::FFTPlot *ui;

    QCustomPlot * plot_X;
    QCustomPlot * plot_A;
    QCustomPlot * plot_P;
    QCPColorMap * colorMap_A;
    QCPColorMap * colorMap_P;

    FileIO *file;

    double xMin, xMax;
    double yMin, yMax;

    int nx, ny;

    QVector<double> filterFuncX;
    QVector<double> filterFuncY;

    bool filterChanged;
    bool filterApplied;
    bool enableSliderFunction;
    bool needToCalFilterFuncX;
    bool needToCalFilterFuncY;

    int filterID;

    double ExtractNumber(QString str);
};

#endif // FFTPLOT_H
