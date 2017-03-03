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
    colorMap_V = new QCPColorMap(plot_V->xAxis, plot_V->yAxis);
    colorMap_W->clearData();
    colorMap_V->clearData();
    colorMap_W->setInterpolate(false);
    colorMap_V->setInterpolate(false);

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
    plot->addGraph(); //original data
    plot->addGraph(); //denoised data
    plot->graph(0)->setPen(QPen(Qt::blue));
    plot->graph(1)->setPen(QPen(Qt::red));
    plot->xAxis->setLabel("time [us]");
    plot->graph(0)->clearData();

    enableVerticalBar = 0;

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

    //QVector<double> y;
    //for( int i = 0; i < qPow(2,5); i++){
    //    y.push_back(qCos(i/5.));
    //}

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

    //setPlot
    int nx = wave->GetSize();
    int ny = wave->GetM();
    colorMap_W->data()->setRange(QCPRange(x[0],x[nx-1]), QCPRange(0,-ny));
    colorMap_W->data()->setSize(nx, ny+1);
    colorMap_V->data()->setRange(QCPRange(x[0],x[nx-1]), QCPRange(0,-ny));
    colorMap_V->data()->setSize(nx, ny+1);

    PlotWV();

    int zMax = qCeil(wave->GetWAbsMax())*100;
    ui->verticalSlider->setSingleStep(1);
    ui->verticalSlider->setRange(0, zMax);
    ui->verticalSlider->setValue(0);
    ui->lineEdit_HT->setText("0.0");

    ui->verticalSlider_Scale->setSingleStep(1);
    ui->verticalSlider_Scale->setRange(-ny+1, 0);
    ui->verticalSlider_Scale->setValue(0);
    ui->lineEdit_sLimit->setText("0");

    enableVerticalBar = 1;

    plot->graph(1)->clearData();
    plot->graph(1)->addData(x, y);
    plot->rescaleAxes();
    plot->replot();

}

void WaveletPlot::PlotWV()
{
    QVector<double> * w = wave->GetW();
    QVector<double> * v = wave->GetV();

    int ny = wave->GetM();
    //plot
    QVector<double> temp_W, temp_V;
    for( int s = 1; s < ny ; s++){
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


void WaveletPlot::on_verticalSlider_valueChanged(int value)
{
    if( enableVerticalBar){
        wave->RestoreData();

        ui->lineEdit_HT->setText(QString::number(value/100.));
        int sLimit = ui->verticalSlider_Scale->value();
        ui->lineEdit_sLimit->setText(QString::number(sLimit));

        if( !(sLimit == 0 || value == 0.)){
            //qDebug() << "cal." << sLimit << "," << value;
            wave->HardThresholding(value/100., sLimit);
            SendMsg(wave->GetMsg());

            wave->Recontruct();
            SendMsg(wave->GetMsg());

            QVector<double> v0 = wave->GetVoct(0);

            PlotWV();

            plot->graph(1)->clearData();
            plot->graph(1)->addData(file->GetDataSetX(), v0);
            plot->rescaleAxes();
            plot->replot();
        }
    }
}

void WaveletPlot::on_verticalSlider_Scale_valueChanged(int value)
{
    int val = ui->verticalSlider->value();
    on_verticalSlider_valueChanged(val);
}
