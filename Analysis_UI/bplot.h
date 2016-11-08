#ifndef BPLOT_H
#define BPLOT_H

#include <QDialog>
#include <QFile>
#include "qcustomplot.h"
#include "fileio.h"

namespace Ui {
class BPlot;
}

class BPlot : public QDialog
{
    Q_OBJECT

public:
    explicit BPlot(QWidget *parent = 0);
    ~BPlot();

    void SetData(FileIO *file);
    void Plot();
    int FindstartIndex(QVector<double> xdata, double goal);    
    void SetMeanCorr(bool checked);

signals:
    void SendMsg(QString msg);

private slots:
    void on_spinBox_Start_valueChanged(int arg1);
    void on_spinBox_End_valueChanged(int arg1);
    void on_pushButton_clicked();


private:
    Ui::BPlot *ui;

    QCustomPlot * plot;

    FileIO *file;

    QVector<double> x, y;

    bool meanCorr;

};

#endif // BPLOT_H
