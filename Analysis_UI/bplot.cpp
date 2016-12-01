#include "bplot.h"
#include "ui_bplot.h"

BPlot::BPlot(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BPlot)
{
    ui->setupUi(this);

    plot = ui->plotArea;
    plot->xAxis->setLabel("Ctrl. Vol. [V]");
    plot->yAxis->setLabel("Integrated value x=(,) [a.u.]");
    plot->xAxis2->setLabel("y-Index");
    plot->xAxis2->setVisible(true);
    plot->yAxis2->setVisible(true);
    plot->yAxis2->setTickLabels(false);
    plot->yAxis2->setTicks(false);
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

    QString yLabel = "Integrated value x = (-1, 20) us  [a.u.]";
    plot->yAxis->setLabel(yLabel);

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

    plot->graph(0)->clearData();

    connect(plot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(ShowPlotValue(QMouseEvent*)));

}

void BPlot::Plot()
{
    if( file == NULL) return;
    x.clear();
    y.clear();

    QVector<double> xdata = file->GetDataSetX();

    int xStart = ui->spinBox_Start->value();
    int xEnd = ui->spinBox_End->value();

    int nx = xdata.size();
    int ny = file->GetDataSetSize();

    //qDebug() << xStart << ", " << xEnd << ", " << nx ;

    if( xStart >= nx || xEnd >= nx) return;
    if( xStart > xEnd ) return;

    double dx = xdata[nx-1] - xdata[nx-2];

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
    for( int i = startI; i < ny; i++){
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
    int arg2 = ui->spinBox_End->value();
    if( arg2 >= xData.length() ) return;

    double xStart = xData[arg1];
    double xEnd = xData[arg2];
    ui->lineEdit_StartValue->setText(QString::number(xStart)+" us");
    ui->spinBox_End->setMinimum(arg1);

    QString yLabel;
    yLabel.sprintf("Integrated value x = (%4.1f, %4.1f) us [a.u.]", xStart, xEnd);
    plot->yAxis->setLabel(yLabel);

    Plot();
}

void BPlot::on_spinBox_End_valueChanged(int arg1)
{
    QVector<double> xData = this->file->GetDataSetX();

    if( arg1 >= xData.length() ) return;
    int arg2 = ui->spinBox_Start->value();
    if( arg2 >= xData.length() ) return;

    double xStart = xData[arg2];
    double xEnd = xData[arg1];
    ui->lineEdit_EndValue->setText(QString::number(xEnd)+" us");
    ui->spinBox_Start->setMaximum(arg1);

    QString yLabel;
    yLabel.sprintf("Integrated value x = (%4.1f, %4.1f) us [a.u.]", xStart, xEnd);
    plot->yAxis->setLabel(yLabel);

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
    for(int i = 0; i < x.size() ; i++){
        str.sprintf("%15.4f, %15.4f\n", x[i], y[i]);
        stream << str;
    }

    saveFile.close();

    str.sprintf("Save B-Plot to %s", filePath.toStdString().c_str());
    SendMsg(str);

}

void BPlot::ShowPlotValue(QMouseEvent *mouse)
{
    QPoint pt = mouse->pos();
    double x = plot->xAxis->pixelToCoord(pt.rx());
    double y = plot->yAxis->pixelToCoord(pt.ry());

    int yIndex = 0;
    switch (plotUnit) {
    case 1:
        yIndex = file->GetYIndex_HV(x);
        break;
    case 2:
        yIndex = file->GetYIndex_HV(file->Mag2HV(x));
        break;
    default:
        yIndex = file->GetYIndex_CV(x);
        break;
    }

    QString msg;
    msg.sprintf("(x, y) = (%7.4f, %7.4f), y-index = %4d", x, y, yIndex);
    ui->lineEdit_Msg->setText(msg);
}


void BPlot::on_pushButton_Print_clicked()
{
    QFileDialog fileDialog(this);
    fileDialog.setNameFilter("pdf (*pdf)");
    fileDialog.setDirectory(DESKTOP_PATH);
    fileDialog.setReadOnly(0);
    QString fileName;
    if( fileDialog.exec()){
        fileName = fileDialog.selectedFiles()[0];
    }

    if( fileName.right(4) != ".pdf" ) fileName.append(".pdf");

    int ph = plot->geometry().height();
    int pw = plot->geometry().width();

    bool ok = plot->savePdf(fileName, pw, ph );

    if( ok ){
        SendMsg("Saved B-Plot as " + fileName);
    }else{
        SendMsg("Save Failed.");
    }
}
