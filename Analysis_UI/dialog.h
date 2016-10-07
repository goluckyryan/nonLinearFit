#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QVector>
#include "qcustomplot.h"
#include "analysis.h"

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
    void SetDataSize(int n);
    void SetAvalibleData(int n);

    void FillData(Analysis *ana);

    bool IsDataSizeFixed(){return fixedSize;}

    void PlotData();
    void PlotSingleData(int plotID);

signals:
    void SendMsg(QString str);

private slots:

    void on_checkBox_a_clicked(bool checked);
    void on_checkBox_Ta_clicked(bool checked);
    void on_checkBox_b_clicked(bool checked);
    void on_checkBox_Tb_clicked(bool checked);
    void on_checkBox_c_clicked(bool checked);

    void on_checkBox_abc_clicked(bool checked);

private:
    Ui::Dialog *ui;
    QCustomPlot * plot;

    int dataSize;
    int parSize;
    QVector<double> *fitPar;
    QVector<double> *fitParError;
    double *SSR;

    bool fixedSize;
};

#endif // DIALOG_H
