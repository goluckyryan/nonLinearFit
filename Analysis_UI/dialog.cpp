#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    fitPar(NULL),
    fitParError(NULL),
    SSR(NULL)
{
    ui->setupUi(this);
    plot = ui->widget;

    fixedSize = 0;
}

Dialog::~Dialog()
{
    delete ui;
    delete plot;
    if( fitPar != NULL) delete fitPar;
    if( fitParError != NULL) delete fitParError;
    if( SSR != NULL) delete SSR;
}

void Dialog::ClearData()
{
    fixedSize = 0;
    if( fitPar != NULL) delete fitPar;
    if( fitParError != NULL) delete fitParError;
    if( SSR != NULL) delete SSR;

    fitPar = NULL;
    fitParError = NULL;
    SSR = NULL;

    yMin = 0;
    yMax = 0;

}

void Dialog::SetDataSize(int n)
{
    if( fixedSize == 1) return;

    dataSize = n;
    fitPar = new QVector<double> [n];
    fitParError = new QVector<double> [n];
    SSR = new double [n];

    fixedSize = 1;
}

void Dialog::FillData(Analysis *ana)
{
    int yIndex = ana->GetYIndex();
    fitPar[yIndex] = ana->GetParameters();
    fitParError[yIndex] = ana->GetParError();
    SSR[yIndex] = ana->GetSSR();

    //qDebug() << fitPar[yIndex];
    //Find yMin, yMax;
    if(fitPar[yIndex][0] > yMax) yMax = fitPar[yIndex][0];
    if(fitPar[yIndex][0] < yMin) yMin = fitPar[yIndex][0];
}

void Dialog::PlotData()
{
    if( fixedSize == 0) return;

    if( plot->graphCount() == 0) {
        plot->addGraph();
        plot->xAxis->setLabel("yIndex");
        plot->yAxis->setLabel("a");
    }

    int graphID = 0;

    switch (graphID) {
    case 0: plot->graph(graphID)->setPen(QPen(Qt::blue)); break;
    case 1: plot->graph(graphID)->setPen(QPen(Qt::red)); break;
    case 2: plot->graph(graphID)->setPen(QPen(Qt::green)); break;
    case 3: plot->graph(graphID)->setPen(QPen(Qt::magenta)); break;
    case 4: plot->graph(graphID)->setPen(QPen(Qt::black)); break;
    }

    plot->graph(0)->clearData();
    plot->graph(0)->setLineStyle(QCPGraph::lsNone);
    plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle , 3));
    plot->graph(0)->setErrorType(QCPGraph::etValue);
    plot->graph(0)->setErrorPen(QPen(Qt::blue));

    QVector<double> x, y, ye;
    for(int i = 0; i < dataSize; i++){
        x.push_back(i);
        if( fitPar[i].size() == 0){
            y.push_back(0);
            ye.push_back(0);
        }else{
            y.push_back(fitPar[i][0]);
            ye.push_back(fitParError[i][0]);
        }
    }

    plot->graph(0)->setDataValueError(x,y, ye);
    plot->xAxis->setRange(x[0],x[dataSize-1]);
    plot->yAxis->setRange(yMin*1.2, yMax*1.2);
    plot->replot();

}

void Dialog::on_pushButton_clicked()
{
    PlotData();
}
