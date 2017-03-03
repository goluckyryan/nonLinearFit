#include "waveletplot.h"
#include "ui_waveletplot.h"

WaveletPlot::WaveletPlot(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WaveletPlot)
{
    ui->setupUi(this);

    file = NULL;
    wave = NULL;

    plot_W = ui->plot_W;
    plot_W->xAxis->setLabel("time [us]");
    plot_W->yAxis->setLabel("Octave");
    plot_W->yAxis->setAutoTickStep(false);
    plot_W->yAxis->setTickStep(1.);

    plot_V = ui->plot_V;
    plot_V->xAxis->setLabel("time [us]");
    plot_V->yAxis->setLabel("Octave");
    plot_V->yAxis->setAutoTickStep(false);
    plot_V->yAxis->setTickStep(1.);

    colorMap_W = new QCPColorMap(plot_W->xAxis, plot_W->yAxis);
    colorMap_W->clearData();
    colorMap_V = new QCPColorMap(plot_V->xAxis, plot_V->yAxis);
    colorMap_V->clearData();

    QCPColorScale *colorScale_W = new QCPColorScale(plot_W);
    plot_W->plotLayout()->addElement(0, 1, colorScale_W);
    colorScale_W->setType(QCPAxis::atRight);
    colorMap_W->setColorScale(colorScale_W);
    colorMap_W->setGradient(QCPColorGradient::gpJet );

    QCPColorScale *colorScale_V = new QCPColorScale(plot_V);
    plot_V->plotLayout()->addElement(0, 1, colorScale_V);
    colorScale_V->setType(QCPAxis::atRight);
    colorMap_V->setColorScale(colorScale_V);
    colorMap_V->setGradient(QCPColorGradient::gpJet );

    plot = ui->plot;
    plot->addGraph();
    plot->graph(0)->setPen(QPen(Qt::blue));
    plot->xAxis->setLabel("time [us]");
    plot->graph(0)->clearData();

}

WaveletPlot::~WaveletPlot()
{
    delete ui;
    delete colorMap_W;
    delete plot_W;
    delete colorMap_V;
    delete plot_V;
    delete plot;

    file = NULL;

    delete wave;
}

void WaveletPlot::SetData(FileIO *file, int yIndex)
{
    this->file = file;
    colorMap_W->clearData();

    QVector<double> y = file->GetDataSetZ(yIndex);
    QVector<double> x = file->GetDataSetX();

    plot->graph(0)->clearData();
    plot->graph(0)->addData(x,y);
    plot->rescaleAxes();
    plot->replot();

    QString msg;
    msg.sprintf("==== Discrete Wavelet Analysis (Haar wavelet), y-Index = %d.", yIndex );
    SendMsg(msg);

    //wavelet decomposition
    wave = new WaveletAnalysis(y);
    SendMsg(wave->GetMsg());
    wave->Decompose();
    SendMsg(wave->GetMsg());

    QVector<double> * w = wave->GetW();
    QVector<double> * v = wave->GetV();

    //plot
    int nx = wave->GetSize();
    int ny = wave->GetM();
    colorMap_W->data()->setRange(QCPRange(x[0],x[nx-1]), QCPRange(0,-ny));
    colorMap_W->data()->setSize(nx, ny);
    colorMap_V->data()->setRange(QCPRange(x[0],x[nx-1]), QCPRange(0,-ny));
    colorMap_V->data()->setSize(nx, ny);


    QVector<double> temp_W, temp_V;
    for( int s = 1; s < ny; s++){
        //Filling space
        temp_W.clear();
        temp_V.clear();
        for( int k = 0; k < w[s].size(); k++){
            for( int d = 0; d < qPow(2,s); d++){
                temp_W.push_back( qAbs(w[s][k]) );
                temp_V.push_back( qAbs(v[s][k]) );
            }
        }

        for( int k = 0; k < temp_W.size(); k++){
            colorMap_W->data()->setCell(k,ny-s, temp_W[k]);
            colorMap_V->data()->setCell(k,ny-s, temp_V[k]);
        }
    }

    colorMap_W->rescaleDataRange();
    colorMap_W->rescaleAxes();
    plot_W->replot();

    colorMap_V->rescaleDataRange();
    colorMap_V->rescaleAxes();
    plot_V->replot();

    w = NULL;
    delete [] w;
    v = NULL;
    delete [] v;

}

