#include "waveletplot.h"
#include "ui_waveletplot.h"

WaveletPlot::WaveletPlot(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WaveletPlot)
{
    ui->setupUi(this);

    wave = NULL;
    file = NULL;

    plot = ui->plot;
    plot->addGraph(); // for the W-space
    plot->xAxis->setLabel("time [us]");
    plot->yAxis->setLabel("Octave");
}

WaveletPlot::~WaveletPlot()
{
    delete ui;
    delete plot;
}

void WaveletPlot::SetData(FileIO *file)
{
    this->file = file;

    QVector<double> test;

    wave = new WaveletAnalysis(test);


}
