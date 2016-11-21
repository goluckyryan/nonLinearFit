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
    plot->xAxis2->setLabel("y-Index");
    plot->xAxis2->setVisible(true);
    plot->setInteraction(QCP::iRangeDrag,true);
    plot->setInteraction(QCP::iRangeZoom,true);
    plot->axisRect()->setRangeDrag(Qt::Vertical);
    plot->axisRect()->setRangeZoom(Qt::Vertical);
    plot->addGraph();
    plot->graph(0)->setPen(QPen(Qt::blue));
    plot->graph(0)->clearData();

    meanCorr = 0;
}

BPlot::~BPlot()
{
    delete ui;
    delete plot;

    //if( file != NULL) delete file;
}

void BPlot::SetData(FileIO *file)
{
    if( file == NULL) return;

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

    if(file->IsYRevered()) {
        plot->xAxis2->setRangeReversed(1);
    }else{
        plot->xAxis2->setRangeReversed(0);
    }

    n = file->GetDataSetSize();
    //QVector<double> yValue = file->GetDataSetY();
    //plot->xAxis->setRange(yValue[0], yValue[n-1]);
    plot->xAxis->setRange(file->GetYMin(), file->GetYMax());
    plot->xAxis2->setRange(0,n-1);
    if( file->HasBackGround() ) plot->xAxis2->setRange(1,n-1);

    x.clear();
    y.clear();
}

void BPlot::Plot()
{
    if( file == NULL) return;
    x.clear();
    y.clear();

    QVector<double> xdata = file->GetDataSetX();
    int n = file->GetDataSetSize();

    int xStart = ui->spinBox_Start->value();
    int xEnd = ui->spinBox_End->value();

    double dx = xdata[xEnd]-xdata[xEnd-1];

    double yMin, yMax;
    int startI = 0;
    if( file->HasBackGround() ) startI = 1;
    for( int i = startI; i < n; i++){
        x.push_back(file->GetDataY(i));
        //integrated
        QVector<double> zdata = file->GetDataSetZ(i);
        double sum = 0;
        double mean = 0 ;
        if( meanCorr ) mean = file->GetDataMeanZ(i);

        for(int j = xStart; j <= xEnd ; j++){
            sum += zdata[j] - mean;
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
    ui->lineEdit_StartValue->setText(QString::number(xData[arg1])+" us");

    Plot();
}

void BPlot::on_spinBox_End_valueChanged(int arg1)
{
    QVector<double> xData = file->GetDataSetX();
    ui->lineEdit_EndValue->setText(QString::number(xData[arg1])+ " us");

    Plot();
}

void BPlot::on_pushButton_clicked()
{
    QString filePath = file->GetFilePath();
    filePath.chop(4);
    filePath.append("_BPlot.dat");
    QFile saveFile(filePath);
    saveFile.open(QIODevice::WriteOnly);
    QTextStream stream(&saveFile);
    QString str;

    //header
    int xEnd = ui->spinBox_End->value();
    int xStart = ui->spinBox_Start->value();
    QVector<double> xData = this->file->GetDataSetX();
    str.sprintf("#xStart : %4d (%8f us) \n", xStart, xData[xStart]);
    stream << str;
    str.sprintf("#xEnd   : %4d (%8f us) \n", xEnd, xData[xEnd]);
    stream << str;
    str.sprintf("%15s, %15s\n", "B-field [mV]", "Int. Value [a.u.]");
    stream << str;

    //fill data;
    int n = file->GetDataSetSize();
    int startI = 0;
    if(file->HasBackGround()) startI = 1;
    for(int i = startI; i < n ; i++){
        str.sprintf("%15.4f, %15.4f\n", x[i], y[i]);
        stream << str;
    }

    saveFile.close();

    str.sprintf("Save B-Plot to %s", filePath.toStdString().c_str());
    SendMsg(str);

}

void BPlot::SetMeanCorr(bool checked)
{
    meanCorr = checked;
    Plot();
}
