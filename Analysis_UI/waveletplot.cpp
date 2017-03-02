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
    //plot->addGraph(); // for the W-space
    plot->xAxis->setLabel("time [us]");
    plot->yAxis->setLabel("Octave");

    colorMap = new QCPColorMap(plot->xAxis, plot->yAxis);
    colorMap->clearData();

    QCPColorScale *colorScale = new QCPColorScale(plot);
    plot->plotLayout()->addElement(0, 1, colorScale);
    colorScale->setType(QCPAxis::atRight);
    colorMap->setColorScale(colorScale);
    colorMap->setGradient(QCPColorGradient::gpJet );
}

WaveletPlot::~WaveletPlot()
{
    delete ui;
    delete colorMap;
    delete plot;
}

void WaveletPlot::SetData(FileIO *file)
{
    this->file = file;

    colorMap->clearData();

    QVector<double> test;

    for(int i = 0; i < qPow(2,10); i++){
        if( i < qPow(2,9) ){
            test.push_back(qCos(i/10.));
        }else{
            test.push_back(qCos(i/40.));
        }
    }

    //wavelet decomposition
    wave = new WaveletAnalysis(test);
    wave->Decompose();

    QVector<double> * ww = wave->GetW();

    //plot
    int nx = wave->GetSize();
    int ny = wave->GetM();
    colorMap->data()->setRange(QCPRange(1,nx), QCPRange(0,ny));
    colorMap->data()->setSize(nx, ny);


    QVector<double> temp;
    for( int s = 1; s < ny; s++){
        temp.clear();
        for( int k = 0; k < ww[s].size(); k++){

            double w = ww[s][k];

            for( int d = 0; d < qPow(2,s); d++){
                temp.push_back(w);
            }

        }

        for( int k = 0; k < temp.size(); k++){
            colorMap->data()->setCell(k,s, temp[k]);
        }

        //qDebug() << "s=" << s << "|" << temp.size() <<"|" << temp;
    }


    colorMap->rescaleDataRange();
    colorMap->rescaleAxes();
    plot->replot();

    ww = NULL;
    delete [] ww;

}

