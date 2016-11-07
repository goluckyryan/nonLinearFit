#ifndef BPLOT_H
#define BPLOT_H

#include <QDialog>
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

    SetData(FileIO *file);
    Plot();
    int FindstartIndex(QVector<double> xdata, double goal);

private slots:
    void on_spinBox_Start_valueChanged(int arg1);

    void on_spinBox_End_valueChanged(int arg1);

private:
    Ui::BPlot *ui;

    QCustomPlot * plot;

    FileIO *file;

};

#endif // BPLOT_H
