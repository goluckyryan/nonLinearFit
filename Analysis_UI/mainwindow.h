#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QVector>
#include <QKeyEvent>
#include "matrix.h"
#include "analysis.h"
#include "qcustomplot.h"
#include "constant.h"
#include "fileio.h"
#include "fitresult.h"
#include "bplot.h"
#include "fftplot.h"
#include "waveletplot.h"
#include "databasewindow.h"

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
    void PlotTimePlot(int graphID, QVector<double> x, QVector<double> y);
    void PlotBFieldPlot();
    void PlotBFieldXLine(double x);
    void PlotFitFuncAndXLines();
    void PlotContourPlot(double offset);

    QVector<double> GetParametersFromLineText();
    void UpdateLineTextParameters(QVector<double> par, QVector<double> epar);

private slots:
    void keyPressEvent(QKeyEvent *key);
    void keyReleaseEvent(QKeyEvent *key);

    void Write2Log(QString str);
    void setEnabledPlanel(bool IO);

    void PlotAllPlots();

    void on_pushButton_DataBase_clicked();
    void on_pushButton_OpenFile_clicked();
    void OpenFile(QString fileName, int kind = 0);

    void ChangeReactAxis(QCPAxis * axis);
    void ShowMousePositionInTimePlot(QMouseEvent * mouse);
    void ShowMousePositionInContourPlot(QMouseEvent * mouse);
    void SetXIndexByMouseClick(QMouseEvent * mouse);
    void SetYIndexByMouseClick(QMouseEvent * mouse);
    void on_horizontalScrollBar_sliderMoved(int position);
    void timePlotXAxisChanged(QCPRange range);
    void timePlotChangeYAxis2Range(QCPRange range);

    void bFieldPlotXAxisChanged(QCPRange range);
    void bFieldPlotChangeYAxis2Range(QCPRange range);
    void ShowMousePositionInBFieldPlot(QMouseEvent * mouse);
    void SetYIndexByMouseClickInBFieldPlot(QMouseEvent * mouse);

    void on_spinBox_y_valueChanged(int arg1);
    void on_spinBox_x_valueChanged(int arg1);
    void on_spinBox_x2_valueChanged(int arg1);

    void on_lineEdit_a_returnPressed();
    void on_lineEdit_Ta_returnPressed();
    void on_lineEdit_b_returnPressed();
    void on_lineEdit_Tb_returnPressed();
    void on_lineEdit_c_returnPressed();

    void on_pushButton_Fit_clicked();
    void on_pushButton_resetPars_clicked();
    void on_pushButton_save_clicked();
    void on_pushButton_FitAll_clicked();

    void on_checkBox_b_Tb_clicked(bool checked);
    void on_checkBox_c_clicked(bool checked);

    void on_actionFit_Result_triggered();
    void on_actionB_Plot_triggered();
    void on_actionFFTW_Plot_triggered();
    void on_actionDWT_Plot_triggered();

    void on_actionSave_data_triggered();
    void on_actionSave_as_Single_X_CSV_triggered();
    void on_actionSave_as_Double_X_CSV_triggered();
    void on_actionConvert_Origin_Data_as_Single_X_CVS_triggered();
    void on_actionConvert_Origin_Data_as_Double_X_CVS_triggered();

    void on_pushButton_RestoreData_clicked();
    void on_checkBox_MeanCorr_clicked(bool checked);
    void on_checkBox_BGsub_clicked(bool checked);
    void on_spinBox_BGIndex_valueChanged();
    void on_spinBox_MovingAvg_valueChanged(int arg1);
    
    void on_verticalSlider_z_sliderMoved(int position);
    void on_verticalSlider_zOffset_sliderMoved(int position);

    void on_comboBox_yLabelType_currentIndexChanged(int index);

    void on_actionSave_Plot_as_PDF_triggered();
    void on_actionSave_Contour_Plot_as_PDF_triggered();
    void on_actionSave_BFieldPlot_as_PDF_triggered();

    void on_spinBox_x1_B_valueChanged(int arg1);
    void on_spinBox_x2_B_valueChanged(int arg1);


    void on_actionHelp_triggered();

    void HelpPicNext();

private:
    Ui::MainWindow *ui;
    FitResult * fitResultPlot;
    BPlot * bPlot;
    FFTPlot * fftPlot;
    WaveletPlot * wPlot;

    QString Msg;

    QCustomPlot *timePlot;
    QCustomPlot *contourPlot;
    QCPColorMap * colorMap;
    QCustomPlot *bFieldPlot;

    FileIO *file;
    Analysis *ana;
    DataBaseWindow * dbWindow;

    bool savedSingleXCVS;
    bool allowTimePlot;
    bool allowBFieldPlot;
    bool controlPressed;

    QDialog * helpDialog;
    int picNumber;
    QLabel * HelpLabel;

};

#endif // MAINWINDOW_H
