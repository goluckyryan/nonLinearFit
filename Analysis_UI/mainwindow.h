#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QVector>
#include "matrix.h"
#include "analysis.h"
#include "qcustomplot.h"
#include "constant.h"
#include "fileio.h"
#include "fitresult.h"
#include "bplot.h"
#include "fftplot.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void SetupPlots();

    void Plot(int graphID, QVector<double> x, QVector<double> y);
    void PlotFitFunc();
    void PlotContour(double offset);

    QVector<double> GetParametersFromLineText();
    void UpdateLineTextParameters(QVector<double> par, QVector<double> epar);


private slots:
    void Write2Log(QString str);

    void on_pushButton_OpenFile_clicked();

    void on_spinBox_y_valueChanged(int arg1);
    void on_spinBox_x_valueChanged(int arg1);

    void on_lineEdit_a_returnPressed();
    void on_lineEdit_Ta_returnPressed();
    void on_lineEdit_b_returnPressed();
    void on_lineEdit_Tb_returnPressed();
    void on_lineEdit_c_returnPressed();

    void on_pushButton_Fit_clicked();
    void on_pushButton_reset_clicked();
    void on_pushButton_save_clicked();
    void on_pushButton_FitAll_clicked();

    void on_checkBox_b_Tb_clicked(bool checked);
    void on_checkBox_c_clicked(bool checked);
    void on_actionFit_Result_triggered();
    void on_actionB_Plot_triggered();
    void on_actionFFTW_Plot_triggered();
    void on_actionSave_as_Single_X_CVS_triggered();

    void setDisabledPlanel();
    void setEnabledPlanel();

    void on_pushButton_RestoreData_clicked();
    void on_checkBox_MeanCorr_clicked(bool checked);
    void on_checkBox_BGsub_clicked(bool checked);
    void on_spinBox_BGIndex_valueChanged(int arg1);
    void on_spinBox_MovingAvg_valueChanged(int arg1);
    
    void on_verticalSlider_z_sliderMoved(int position);
    void on_verticalSlider_zOffset_sliderMoved(int position);


private:
    Ui::MainWindow *ui;
    FitResult * fitResultPlot;
    BPlot * bPlot;
    FFTPlot * fftPlot;

    QString Msg;

    QCustomPlot *plot;
    QCustomPlot *ctplot;
    QCPColorMap * colorMap;

    FileIO *file;
    Analysis *ana;

    bool savedSimplifiedtxt;

};

#endif // MAINWINDOW_H
