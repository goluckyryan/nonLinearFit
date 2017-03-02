#ifndef WAVELETPLOT_H
#define WAVELETPLOT_H

#include <QMainWindow>
#include "waveletanalysis.h"
#include "qcustomplot.h"
#include "fileio.h"
#include <QVector>

namespace Ui {
class WaveletPlot;
}

class WaveletPlot : public QMainWindow
{
    Q_OBJECT

public:
    explicit WaveletPlot(QWidget *parent = 0);
    ~WaveletPlot();

    void SetData(FileIO * file);

private:
    Ui::WaveletPlot *ui;

    QCustomPlot * plot;

    WaveletAnalysis * wave;

    FileIO *file;
};

#endif // WAVELETPLOT_H
