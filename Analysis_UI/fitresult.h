#ifndef FITRESULT_H
#define FITRESULT_H

#include <QDialog>
#include <QVector>
#include <QFile>
#include "qcustomplot.h"
#include "constant.h"
#include "analysis.h"
#include "fileio.h"

namespace Ui {
class FitResult;
}

class FitResult : public QDialog
{
    Q_OBJECT

public:
    explicit FitResult(QWidget *parent = 0);
    ~FitResult();

    void ClearData();
    void SetDataSize(FileIO *file);
    void SetAvalibleData(int n);
    void FillData(Analysis *ana);
    bool IsDataSizeFixed(){return fixedSize;}
    void PlotData();
    void PlotSingleData(int plotID);
    QVector<double> ReSizeVector(QVector<double> vec);
    void SetFilePath(QString str){filePath = str;}

signals:
    void SendMsg(QString str);

public slots:
    void on_pushButton_SavePlot_clicked();

private slots:
    void on_checkBox_a_clicked(bool checked);
    void on_checkBox_Ta_clicked(bool checked);
    void on_checkBox_b_clicked(bool checked);
    void on_checkBox_Tb_clicked(bool checked);
    void on_checkBox_c_clicked(bool checked);
    void on_checkBox_P_clicked(bool checked);
    void on_checkBox_SSR_clicked(bool checked);

    void on_pushButton_Save_clicked();
    void on_pushButton_ResetScale_clicked();

private:
    Ui::FitResult *ui;
    QCustomPlot * plot;

    QString filePath;

    int dataSize;
    int parSize;
    QVector<double> yValue;
    QVector<double> *fitPar;
    QVector<double> *fitParError;
    double *SSR;

    bool fixedSize;
};

#endif // FITRESULT_H
