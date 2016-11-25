#include "bplot.h"
#include "ui_bplot.h"

BPlot::BPlot(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BPlot)
{
    ui->setupUi(this);

    plot = ui->plotArea;
    plot->xAxis->setLabel("Ctrl. Vol. [V]");
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

    plotUnit = 0;

}

BPlot::~BPlot()
{
    delete ui;
    delete plot;

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

    x.clear();
    y.clear();

    if(file->IsYRevered()) {
        plot->xAxis2->setRangeReversed(1);
    }else{
        plot->xAxis2->setRangeReversed(0);
    }

    n = file->GetDataSetSize();
    plot->xAxis->setRange(file->GetYMin_CV(), file->GetYMax_CV());
    plot->xAxis2->setRange(0,n-1);
    if( file->HasBackGround() ) plot->xAxis2->setRange(1,n-1);

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

    //qDebug() << xStart << ", " << xEnd << ", " << n;

    if( xStart >= n || xEnd >= n) return;
    if( xStart > xEnd ) return;

    double dx = xdata[n] - xdata[n-1];

    double yMin, yMax;
    double xMin, xMax;
    switch (plotUnit) {
    case 0:
        xMin = file->GetYMin_CV();
        xMax = file->GetYMax_CV();
        plot->xAxis->setLabel("Ctrl. Vol. [V]");
        break;
    case 1:
        xMin = file->GetYMin_HV();
        xMax = file->GetYMax_HV();
        plot->xAxis->setLabel("Hall Vol. [mV]");
        break;
    case 2:
        xMin = file->HV2Mag(file->GetYMin_HV());
        xMax = file->HV2Mag(file->GetYMax_HV());
        plot->xAxis->setLabel("B-field [mT]");
        break;
    }
    plot->xAxis->setRange(xMin, xMax);

    int startI = 0;
    if( file->HasBackGround() ) startI = 1;
    for( int i = startI; i < n; i++){
        double xValue = 0;
        switch (plotUnit) {
        case 0:xValue = file->GetDataY_CV(i);break;
        case 1:xValue = file->GetDataY_HV(i);break;
        case 2:xValue = file->HV2Mag(file->GetDataY_HV(i));break;
        }

        x.push_back(xValue);
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
    QVector<double> xData = this->file->GetDataSetX();
    if( arg1 >= xData.length() ) return;
    ui->lineEdit_StartValue->setText(QString::number(xData[arg1])+" us");

    Plot();
}

void BPlot::on_spinBox_End_valueChanged(int arg1)
{
    QVector<double> xData = file->GetDataSetX();
    if( arg1 >= xData.length() ) return;
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

