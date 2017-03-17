#include "bplot.h"
#include "ui_bplot.h"

BPlot::BPlot(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BPlot)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window);

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
    //plot->axisRect()->setRangeDrag(Qt::Horizontal);
    //plot->axisRect()->setRangeZoom(Qt::Horizontal);
    plot->addGraph();
    plot->graph(0)->setPen(QPen(Qt::blue));
    plot->graph(0)->clearData();

    // indicator
    plot->addGraph();
    plot->graph(1)->setPen(QPen(Qt::gray));
    plot->graph(1)->clearData();

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

    plot->disconnect();
    connect(plot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(ShowPlotValue(QMouseEvent*)));
    connect(plot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(SetYStart(QMouseEvent*)));
    connect(plot, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(SetYEnd(QMouseEvent*)));    
    //connect(plot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(RemovePlotItems()));

    zeros.clear();
    peaks.clear();
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

    double xMin = 0, xMax = 0;
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

    yMin = 0;
    yMax = 0;
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

    //Integration of the plot
    double yRange_Int = 0;
    if( ui->checkBox_Integrate->isChecked()){

        if( file->IsYRevered() ){
            for(int i = y.size() - 2 ; i >= 0; i--){
                y[i] += y[i+1];
            }
        }else{
            for(int i = 1; i < y.size(); i++){
                y[i] += y[i-1];
            }
        }

        for(int i = 0; i < y.size(); i++){
            if( fabs(y[i]) > yRange_Int ) yRange_Int = fabs(y[i]);
        }
    }

    if( plotUnit != 0){
        plot->xAxis2->setTickLabels(false);
        plot->xAxis2->setTicks(false);
        plot->xAxis2->setLabel("");
    }else{
        plot->xAxis2->setTickLabels(true);
        plot->xAxis2->setTicks(true);
        plot->xAxis2->setLabel("y-Index");
    }

    plot->graph(0)->clearData();

    double yRange = 2*qMax(fabs(yMin), fabs(yMax));
    if( ui->checkBox_Integrate->isChecked()) yRange = yRange_Int * 1.3 ;
    plot->yAxis->setRange(-yRange, yRange);

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
    RemovePlotItems();
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
    RemovePlotItems();
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

    //========= PLot a line
    QVector<double> lineX, lineY;
    lineX.push_back(x);
    lineX.push_back(x);

    double yRange = plot->yAxis->range().maxRange;

    lineY.push_back(yRange);
    lineY.push_back(-yRange);

    plot->graph(1)->clearData();
    plot->graph(1)->addData(lineX, lineY);
    plot->replot();
}

void BPlot::FindPeak(QVector<double> x, QVector<double> y)
{
    if( x.size() < 3) return;
    //use first derivative
    QVector<double> xd, yd;
    int n = y.size();
    double dx = fabs(x[1] - x[0]);
    for( int i = 1; i < n-1; i++ ){
        xd.push_back(x[i]);
        yd.push_back((y[i+1]-y[i-1])/2/dx);
    }
    FindZeros("Peak(s)",xd, yd);

    //use weigthed quadratic fit to get the peak;
}

void BPlot::FindZeros(QString type,QVector<double> x, QVector<double> y)
{
    QVector<double> sols;

    qDebug() << x;

    int n = y.size();
    for( int i = 1; i < n; i++){
        double y1 = y[i-1];
        double y2 = y[i];
        double x1 = x[i-1];
        double x2 = x[i];

        if( y1*y2 <= 0){
            double x0 = x1 - y1*(x2-x1)/(y2-y1);
            qDebug("%d | (%f, %f), (%f, %f) = %f", i, x1, y1, x2, y2, x0);
            sols.push_back(x0);
            //check x0 appeard in zeros, if not , save;
            if( zeros.indexOf(x0)== -1 && type == "Zero(s)"){
                zeros.push_back(x0);
            }
            if( peaks.indexOf(x0)== -1 && type == "Peak(s)"){
                peaks.push_back(x0);
            }
        }
    }

    //qDebug() << zeros;
    //qDebug() << sols;

    QString msg ;
    if( sols.size() > 0){
        msg.sprintf("Find %s in (%7.4f, %7.4f) = %f", type.toStdString().c_str(), x[0],x[n-1], sols[0]);
        for(int i = 1; i < sols.size() ; i++){
            msg.append(" ," + QString::number(sols[i]));
        }
    }else{
        msg.sprintf("Find %s in (%7.4f, %7.4f) = No Zeros.", type.toStdString().c_str(), x[0],x[n-1]);
    }
    SendMsg(msg);
    ui->lineEdit_Msg->setText(msg);

    AddArrows();

}

void BPlot::SetYStart(QMouseEvent *mouse)
{
    QPoint pt = mouse->pos();
    double x = plot->xAxis->pixelToCoord(pt.rx());

    mouseYIndex1 = 0;
    switch (plotUnit) {
    case 1:
        mouseYIndex1 = file->GetYIndex_HV(x);
        break;
    case 2:
        mouseYIndex1 = file->GetYIndex_HV(file->Mag2HV(x));
        break;
    default:
        mouseYIndex1 = file->GetYIndex_CV(x);
        break;
    }
}

void BPlot::SetYEnd(QMouseEvent *mouse)
{
    QPoint pt = mouse->pos();
    double x = plot->xAxis->pixelToCoord(pt.rx());

    mouseYIndex2 = 0;
    switch (plotUnit) {
    case 1:
        mouseYIndex2 = file->GetYIndex_HV(x);
        break;
    case 2:
        mouseYIndex2 = file->GetYIndex_HV(file->Mag2HV(x));
        break;
    default:
        mouseYIndex2 = file->GetYIndex_CV(x);
        break;
    }

    //========== Create Find vector;
    QVector<double> tempX, tempY;
    if( mouseYIndex1 > mouseYIndex2) {
        int temp = mouseYIndex2;
        mouseYIndex2 = mouseYIndex1;
        mouseYIndex1 = temp;
    }
    qDebug("(%d, %d)", mouseYIndex1, mouseYIndex2);
    for(int i = mouseYIndex1 ; i <= mouseYIndex2; i++){
        tempX.push_back(this->x[i]);
        tempY.push_back(this->y[i]);
    }

    if( mouse->button() == Qt::LeftButton){
        FindZeros("Zero(s)", tempX, tempY);
    }

    if( mouse->button() == Qt::RightButton){
        FindPeak(tempX, tempY);
    }

}

void BPlot::AddArrows()
{
    if( zeros.size()>0){
        for(int i = 0; i < zeros.size(); i++){
            //check if the zeros is shown
            bool skipFlag = 0;
            for( int j = 0; j < plot->itemCount(); j++){
                if( j % 2 == 1) continue;
                double test = ((plot->item(j)->positions())[0])->coords().rx();
                if( test == zeros[i] ) skipFlag = 1;
                //if( fabs(test - zeros[i]) < 0.001 ) skipFlag = 1;
            }

            if( skipFlag ) continue;

            QCPItemText *textLabel = new QCPItemText(plot);
            plot->addItem(textLabel);
            textLabel->setPositionAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
            textLabel->position->setCoords(zeros[i], yMax/2);
            QString tmp; tmp.sprintf("%7.3f", zeros[i]);
            textLabel->setText(tmp);
            textLabel->setRotation(-90);

            QCPItemLine *arrow = new QCPItemLine(plot);
            plot->addItem(arrow);
            arrow->start->setParentAnchor(textLabel->left);
            arrow->end->setCoords(zeros[i], 0);
            arrow->setHead(QCPLineEnding::esSpikeArrow);
            plot->replot();
        }
    }

    if( peaks.size()>0){
        for(int i = 0; i < peaks.size(); i++){
            //check if the zeros is shown
            bool skipFlag = 0;
            for( int j = 0; j < plot->itemCount(); j++){
                if( j % 2 == 1) continue;
                double test = ((plot->item(j)->positions())[0])->coords().rx();
                if( test == peaks[i] ) skipFlag = 1;
                //if( fabs(test - zeros[i]) < 0.001 ) skipFlag = 1;
            }

            if( skipFlag ) continue;

            double yOffset = 0;
            double dx = fabs(x[1] - x[0]);
            for( int k = 0; k < x.size(); k++){
                if( fabs(x[k] - peaks[i]) < dx/2 ){
                    yOffset = y[k];
                    break;
                }
            }

            QCPItemText *textLabel = new QCPItemText(plot);
            plot->addItem(textLabel);
            textLabel->setPositionAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
            if( yOffset >= 0){
                textLabel->position->setCoords(peaks[i], yOffset + yMax/2);
            }else{
                textLabel->position->setCoords(peaks[i], yOffset - yMax/2);
            }
            QString tmp; tmp.sprintf("%7.3f", peaks[i]);
            textLabel->setText(tmp);
            textLabel->setRotation(-90);

            QCPItemLine *arrow = new QCPItemLine(plot);
            plot->addItem(arrow);
            if( yOffset >= 0 ){
                arrow->start->setParentAnchor(textLabel->left);
            }else{
                arrow->start->setParentAnchor(textLabel->right);
            }
            arrow->end->setCoords(peaks[i], yOffset);
            arrow->setHead(QCPLineEnding::esSpikeArrow);
            plot->replot();
        }
    }

    qDebug() << "items count : " << plot->itemCount();
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

    //Clean the line
    plot->graph(1)->clearData();

    bool ok = plot->savePdf(fileName, pw, ph );

    if( ok ){
        SendMsg("Saved B-Plot as " + fileName);
    }else{
        SendMsg("Save Failed.");
    }
}

void BPlot::on_pushButton_ClearArrows_clicked()
{
    RemovePlotItems();
}

void BPlot::on_checkBox_Integrate_clicked()
{
    Plot();
}
