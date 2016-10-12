#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QVector>
#include <QFile>
#include "qcustomplot.h"
#include "constant.h"
#include "analysis.h"
#include "fileio.h"

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

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

private slots:

    void on_checkBox_a_clicked(bool checked);
    void on_checkBox_Ta_clicked(bool checked);
    void on_checkBox_b_clicked(bool checked);
    void on_checkBox_Tb_clicked(bool checked);
    void on_checkBox_c_clicked(bool checked);
    void on_checkBox_abc_clicked(bool checked);
    void on_checkBox_SSR_clicked(bool checked);

    void on_pushButton_Save_clicked();
    void on_pushButton_ResetScale_clicked();

private:
    Ui::Dialog *ui;
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

#endif // DIALOG_H
