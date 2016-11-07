#include "bplot.h"
#include "ui_bplot.h"

BPlot::BPlot(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BPlot)
{
    ui->setupUi(this);

    plot = ui->plotArea;
    plot->xAxis->setLabel("B-field [mV]");
    plot->yAxis->setLabel("Integrated value [a.u.]");
    plot->setInteraction(QCP::iRangeDrag,true);
    plot->setInteraction(QCP::iRangeZoom,true);
    plot->axisRect()->setRangeDrag(Qt::Vertical);
    plot->axisRect()->setRangeZoom(Qt::Vertical);
    plot->addGraph();
    plot->graph(0)->setPen(QPen(Qt::blue));
    plot->graph(0)->clearData();
}

BPlot::~BPlot()
{
    delete ui;
    delete plot;

    //delete file;
}

BPlot::SetData(FileIO *file)
{
    this->file = file;

    int n = file->GetDataSize();

    ui->spinBox_Start->setMinimum(0);
    ui->spinBox_End->setMinimum(0);
    ui->spinBox_Start->setMaximum(n-1);
    ui->spinBox_End->setMaximum(n-1);

    QVector<double> xdata = file->GetDataSetX();

    int xStart = FindstartIndex(xdata, -1);
    int xEnd = FindstartIndex(xdata, 20);

    ui->spinBox_Start->setValue(xStart);
    ui->spinBox_End->setValue(xEnd);
    //ui->lineEdit_StartValue->setText(QString::number(xdata[xStart]));
    //ui->lineEdit_EndValue->setText(QString::number(xdata[xEnd]));

}

BPlot::Plot()
{
    QVector<double> xdata = file->GetDataSetX();
    int n = file->GetDataSetSize();

    int xStart = ui->spinBox_Start->value();
    int xEnd = ui->spinBox_End->value();

    double dx = xdata[xEnd]-xdata[xEnd-1];

    QVector<double> x, y;
    x = file->GetDataSetY();
    double yMin, yMax;
    for( int i = 0; i < n; i++){

        //integrated
        QVector<double> zdata = file->GetDataSetZ(i);
        double sum = 0;
        for(int j = xStart; j <= xEnd ; j++){
            sum += zdata[j];
        }
        sum = sum*dx;

        if( i == 0) {
            yMin = sum;
            yMax = sum;
        }
        if( yMin > sum ) yMin = sum;
        if( yMax < sum ) yMax = sum;

        y.push_back(sum);

    }

    plot->graph(0)->clearData();
    plot->xAxis->setRange(x[0], x[n-1]);
    plot->yAxis->setRange(yMin, yMax);

    plot->graph(0)->addData(x,y);

    plot->replot();

}

int BPlot::FindstartIndex(QVector<double> xdata, double goal)
{
    int xIndex = 0;
    for(int i = 0; i < xdata.size() ; i++){
        if( xdata[i] >= goal){
            xIndex = i;
            break;
        }
    }
    return xIndex;
}

void BPlot::on_spinBox_Start_valueChanged(int arg1)
{
    QVector<double> xData = file->GetDataSetX();
    ui->lineEdit_StartValue->setText(QString::number(xData[arg1]));

    Plot();
}

void BPlot::on_spinBox_End_valueChanged(int arg1)
{
    QVector<double> xData = file->GetDataSetX();
    ui->lineEdit_EndValue->setText(QString::number(xData[arg1]));

    Plot();
}
