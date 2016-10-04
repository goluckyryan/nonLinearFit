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
    void FillData(Analysis *ana);

    bool IsDataSizeFixed(){return fixedSize;}

    void PlotData();

private slots:
    void on_pushButton_clicked();

private:
    Ui::Dialog *ui;
    QCustomPlot * plot;

    int dataSize;
    QVector<double> *fitPar;
    QVector<double> *fitParError;
    double *SSR;

    double yMin, yMax;

    bool fixedSize;
};

#endif // DIALOG_H
