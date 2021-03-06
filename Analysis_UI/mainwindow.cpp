/******************************
 * Program flow:
 * 1) on_pushButton_DataBase_clicked(), or
      on_pushButton_OpenFile_clicked(), then
      OpenFile()
 * 2) in OpenFile(),
      there is SetupPlots()
      then in on_check_BGsub_clicked(), which contain PlotAllPlots()
 * 3) User control
******************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    file(NULL)
{
    ui->setupUi(this);

    this->setWindowTitle("Analysis " + QSysInfo::buildCpuArchitecture());

    QDateTime date;
    ui->textEdit->append("### Date : " + date.currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));

    //this->showMaximized();
    int configFileFlag  = loadConfigurationFile();

    if(configFileFlag == 1) {
        QMessageBox msgBox;
        msgBox.setText("The configuration file not exist.\n"
                       "please check the ProgramConfiguration.ini exist on Desktop.");
        msgBox.exec();
    }else if(configFileFlag == 2){
        QMessageBox msgBox;
        msgBox.setText("The configuration file fail to open.");
        msgBox.exec();
    }else if(configFileFlag == 3){
        QMessageBox msgBox;
        msgBox.setText("Some items are missing in configuration file.");
        msgBox.exec();
    }

    dbWindow = new DataBaseWindow(this);
    dbWindow->setWindowModality(Qt::WindowModal);
    Write2Log(dbWindow->GetMsg());
    if( dbWindow->IsDBOpened() ){
        connect(dbWindow, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));
        connect(dbWindow, SIGNAL(ReturnFilePath(QString)), this, SLOT(OpenFile(QString)));
    }else{
        ui->pushButton_DataBase->setEnabled(false);
        QMessageBox msgBox;
        msgBox.setText("Database failed to load.");
        msgBox.exec();
    }

    fitResultPlot = new FitResult(this);
    connect(fitResultPlot, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));
    connect(fitResultPlot, SIGNAL(ChangeYIndex(int)), this, SLOT(on_spinBox_y_valueChanged(int)));

    bPlot = new BPlot(this);
    connect(bPlot, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));

    fftPlot = new FFTPlot(this);
    connect(fftPlot, SIGNAL(PlotData()), this, SLOT(PlotAllPlots()));
    connect(fftPlot, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));

    wPlot = new WaveletPlot(this);
    connect(wPlot, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)) );
    connect(wPlot, SIGNAL(Replot()), this, SLOT(PlotAllPlots()));

    savedSingleXCVS = 0;

    timePlot = ui->customPlot;
    timePlot->xAxis->setLabel("time [us]");
    timePlot->yAxis->setLabel("Voltage [a.u.]");
    timePlot->yAxis2->setVisible(true);
    timePlot->yAxis2->setTickLabels(false);
    timePlot->yAxis2->setTicks(true);
    timePlot->setInteraction(QCP::iRangeDrag,true);
    timePlot->setInteraction(QCP::iRangeZoom,true);
    timePlot->axisRect()->setRangeDrag(Qt::Horizontal);
    timePlot->axisRect()->setRangeZoom(Qt::Horizontal);
    timePlot->axisRect()->setRangeDragAxes(timePlot->xAxis, timePlot->yAxis);
    timePlot->axisRect()->setRangeZoomAxes(timePlot->xAxis, timePlot->yAxis);

    bFieldPlot = ui->customPlot_PlotB;
    bFieldPlot->xAxis2->setLabel("y-Index");
    bFieldPlot->xAxis2->setVisible(true);
    bFieldPlot->yAxis2->setVisible(true);
    bFieldPlot->yAxis2->setTickLabels(false);
    bFieldPlot->yAxis2->setTicks(true);
    bFieldPlot->setInteraction(QCP::iRangeDrag,true);
    bFieldPlot->setInteraction(QCP::iRangeZoom,true);
    bFieldPlot->axisRect()->setRangeDrag(Qt::Horizontal);
    bFieldPlot->axisRect()->setRangeZoom(Qt::Horizontal);
    bFieldPlot->addGraph();
    bFieldPlot->graph(0)->setPen(QPen(Qt::blue));
    bFieldPlot->addGraph();
    bFieldPlot->graph(1)->setPen(QPen(Qt::gray));
    bFieldPlot->addGraph();
    bFieldPlot->graph(2)->setPen(QPen(Qt::darkGreen)); // the y-index

    contourPlot = ui->customPlot_CT;
    contourPlot->axisRect()->setupFullAxesBox(true);
    contourPlot->xAxis->setLabel("time [us]");
    contourPlot->yAxis->setLabel("Ctrl. Vol. [V]");

    colorMap = new QCPColorMap(contourPlot->xAxis, contourPlot->yAxis);
    colorMap->clearData();

    //vertcal and horizontal lines on ctplot;
    contourPlot->addGraph();
    contourPlot->graph(0)->setPen(QPen(Qt::gray));
    contourPlot->graph(0)->clearData();
    contourPlot->addGraph();
    contourPlot->graph(1)->setPen(QPen(Qt::gray));
    contourPlot->graph(1)->clearData();

    //=================
    ana = new Analysis();
    connect(ana, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));

    setEnabledPlanel(0);

    statusBar()->showMessage("Please open a file.");

    Write2Log("Directory: " + DATA_PATH);

    controlPressed = false;
    allowTimePlot = false;
    allowBFieldPlot = false;

    helpDialog = new QDialog(this);
    HelpLabel = new QLabel();
    helpDialog->setWindowTitle("Help");

    QImage image(":/HelpPic/MainWindow1.PNG");
    picNumber = 1;
    HelpLabel->setPixmap(QPixmap::fromImage(image));

    QPushButton * next = new QPushButton("Next");
    connect(next, SIGNAL(pressed()) , this, SLOT(HelpPicNext()));

    QVBoxLayout *mainLayout = new QVBoxLayout(helpDialog);
    mainLayout->addWidget(HelpLabel);
    mainLayout->addWidget(next);

    ui->comboBox_fitFunctionType->addItem("exp + exp");
    ui->comboBox_fitFunctionType->addItem("exp * sin");
    ui->comboBox_fitFunctionType->addItem("load from file");
    ui->comboBox_fitFunctionType->setCurrentIndex(0);

}

MainWindow::~MainWindow(){
    delete ui;
    delete fitResultPlot;
    delete bPlot;
    delete fftPlot;
    delete wPlot;

    delete colorMap;
    delete timePlot;
    delete contourPlot;
    delete bFieldPlot;

    if( file != NULL) delete file;
    if( ana != NULL) delete ana;
    if( dbWindow!=NULL) delete dbWindow;
}

void MainWindow::SetupPlots()
{
    double xMin = file->GetXMin();
    double xMax = file->GetXMax();
    double yMin = file->GetYMin_CV();
    double yMax = file->GetYMax_CV();
    double zMin = file->GetZMin();
    double zMax = file->GetZMax();
    int zRange = qCeil(qMax(fabs(file->GetZMin()), fabs(file->GetZMax())));
    //========= contour Plot

    ui->verticalSlider_zOffset->setMinimum(-zRange);
    ui->verticalSlider_zOffset->setMaximum(zRange);
    ui->verticalSlider_zOffset->setValue(0);

    colorMap->data()->clear();
    int nx = qMin(file->GetXDataSize(), 1000); // limit the grid size.
    int ny = qMin(file->GetYDataSize(), 1000);
    if( file->HasBackGround() ) ny = ny -1;
    colorMap->data()->setSize(nx, ny);

    //colorMap->setInterpolate(true);
    //colorMap->setTightBoundary(false);
    //colorMap->setAntialiased(true);

    contourPlot->xAxis->setRange(xMin, xMax);
    contourPlot->yAxis->setRange(yMin, yMax);

    colorMap->data()->setRange(QCPRange(xMin, xMax), QCPRange(yMin, yMax));

    QCPColorScale *colorScale = new QCPColorScale(contourPlot);
    //ctplot->plotLayout()->clear();
    contourPlot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap->setColorScale(colorScale); // associate the color map with the color scale

    QCPColorGradient colorGrad;
    colorGrad.clearColorStops();
    colorGrad.setColorStopAt(0, QColor(0,0,255));
    colorGrad.setColorStopAt(0.5, QColor(255,255,255));
    colorGrad.setColorStopAt(1, QColor(255,0,0));
    colorGrad.setColorInterpolation(QCPColorGradient::ColorInterpolation::ciRGB);

    //colorMap->setGradient(QCPColorGradient::gpCandy ); //color scheme
    colorMap->setGradient( colorGrad ); //color scheme

    //colorMap->rescaleDataRange();
    colorMap->setDataRange(QCPRange(-zRange, zRange));
    ui->verticalSlider_z->setMinimum(1);
    ui->verticalSlider_z->setMaximum(zRange);
    ui->verticalSlider_z->setSingleStep(qCeil(zRange/100.));
    ui->verticalSlider_z->setValue(zRange);

    QCPMarginGroup *marginGroup = new QCPMarginGroup(contourPlot);
    contourPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    //================= Plot
    timePlot->xAxis->setRange(xMin, xMax);
    timePlot->xAxis2->setVisible(true);
    timePlot->xAxis2->setRange(0, file->GetXDataSize()-1);
    double zRange1 = qMax(fabs(zMax), fabs(zMin));
    timePlot->yAxis->setRange(-zRange1, zRange1);
    //plot->yAxis2->setRange(-zRange1, zRange1);

    int multi = file->GetMultiIndex();
    QString yLabel;
    yLabel.sprintf("Voltage [ 10^%d V]", -1 * multi);
    if( multi == 3) yLabel = "Voltage [mV]";
    if( multi == 6) yLabel = "Voltage [uV]";
    if( multi == 9) yLabel = "Voltage [nV]";
    timePlot->yAxis->setLabel( yLabel);

    bFieldPlot->yAxis->setLabel( "Integrated " + yLabel);
    bFieldPlot->xAxis->setRange(file->GetYMin_CV(), file->GetYMax_CV());
    bFieldPlot->xAxis2->setRangeReversed(file->IsYRevered());
    bFieldPlot->xAxis2->setRange(0, file->GetYDataSize()-1);
    if( file->HasBackGround() ) bFieldPlot->xAxis2->setRange(1,file->GetYDataSize()-1);

    allowTimePlot = false;
    if( file->HasBackGround()){
        ui->spinBox_y->setValue(1);
    }else{
        ui->spinBox_y->setValue(0);
    }
    allowTimePlot = true;

    //set connection
    timePlot->disconnect();
    connect(timePlot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(timePlotXAxisChanged(QCPRange)));
    connect(timePlot->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(timePlotChangeYAxis2Range(QCPRange)));
    connect(timePlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(ShowMousePositionInTimePlot(QMouseEvent*)));
    connect(timePlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(SetXIndexByMouseClick(QMouseEvent*)));

    contourPlot->disconnect();
    connect(contourPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(ShowMousePositionInContourPlot(QMouseEvent*)));
    connect(contourPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(SetYIndexByMouseClick(QMouseEvent*)));

    bFieldPlot->disconnect();
    connect(bFieldPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(bFieldPlotXAxisChanged(QCPRange)));
    connect(bFieldPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(bFieldPlotChangeYAxis2Range(QCPRange)));
    connect(bFieldPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(ShowMousePositionInBFieldPlot(QMouseEvent*)));
    connect(bFieldPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(SetYIndexByMouseClickInBFieldPlot(QMouseEvent*)));


}

void MainWindow::PlotTimePlot(int graphID, QVector<double> x, QVector<double> y){

    while( timePlot->graphCount() < graphID+1){
        timePlot->addGraph();
    }

    switch (graphID) {
    case 0: timePlot->graph(graphID)->setPen(QPen(Qt::blue)); break;
    case 1: timePlot->graph(graphID)->setPen(QPen(Qt::red)); break;
    case 2: timePlot->graph(graphID)->setPen(QPen(Qt::darkGreen)); break;
    case 3: timePlot->graph(graphID)->setPen(QPen(Qt::darkGreen)); break;
    case 4: timePlot->graph(graphID)->setPen(QPen(Qt::magenta)); break;
    case 5: timePlot->graph(graphID)->setPen(QPen(Qt::black)); break;
    }

    timePlot->graph(graphID)->clearData();
    timePlot->graph(graphID)->addData(x, y);
    timePlot->replot();

}

void MainWindow::PlotBFieldPlot()
{
    if (!allowBFieldPlot) return;
    QVector<double> x, y;
    QVector<double> xdata = file->GetDataSetX();

    int xStart = ui->spinBox_x1_B->value();
    int xEnd = ui->spinBox_x2_B->value();

    int nx = xdata.size();
    int ny = file->GetYDataSize();

    if( xStart >= nx || xEnd >= nx) return;
    if( xStart > xEnd ) return;

    double dx = xdata[nx-1] - xdata[nx-2];

    double xMin = 0, xMax = 0;
    int plotUnit = ui->comboBox_yLabelType->currentIndex();
    switch (plotUnit) {
    case 0:
        xMin = file->GetYMin_CV();
        xMax = file->GetYMax_CV();
        bFieldPlot->xAxis->setLabel("Ctrl. Vol. [V]");
        break;
    case 1:
        xMin = file->GetYMin_HV();
        xMax = file->GetYMax_HV();
        bFieldPlot->xAxis->setLabel("Hall Vol. [mV]");
        break;
    case 2:
        xMin = file->HV2Mag(file->GetYMin_HV());
        xMax = file->HV2Mag(file->GetYMax_HV());
        bFieldPlot->xAxis->setLabel("B-field [mT]");
        break;
    }
    bFieldPlot->xAxis->setRange(xMin, xMax);

    int yMin = 0;
    int yMax = 0;
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

    if( plotUnit != 0){
        bFieldPlot->xAxis2->setTickLabels(false);
        bFieldPlot->xAxis2->setTicks(false);
        bFieldPlot->xAxis2->setLabel("");
    }else{
        bFieldPlot->xAxis2->setTickLabels(true);
        bFieldPlot->xAxis2->setTicks(true);
        bFieldPlot->xAxis2->setLabel("y-Index");
    }

    bFieldPlot->graph(0)->clearData();

    double yRange = 2*qMax(fabs(yMin), fabs(yMax));
    bFieldPlot->yAxis->setRange(-yRange, yRange);

    bFieldPlot->graph(0)->addData(x,y);

    //bFieldPlot->rescaleAxes();
    bFieldPlot->replot();

}

void MainWindow::PlotBFieldXLine(double x)
{
    QVector<double> lineX, lineY;
    lineX.push_back(x);
    lineX.push_back(x);

    double yRange = bFieldPlot->yAxis->range().maxRange;

    lineY.push_back(yRange);
    lineY.push_back(-yRange);

    bFieldPlot->graph(1)->clearData();
    bFieldPlot->graph(1)->addData(lineX, lineY);
    bFieldPlot->replot();
}

void MainWindow::Write2Log(QString str){
    ui->textEdit->append(str);
    qDebug()<< str;
    ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
}

void MainWindow::PlotAllPlots()
{
    on_spinBox_y_valueChanged(ui->spinBox_y->value());
    PlotContourPlot(ui->verticalSlider_zOffset->value());
    on_spinBox_x1_B_valueChanged(bPlot->FindstartIndex(-1));
    bPlot->SetPlotUnit(ui->comboBox_yLabelType->currentIndex());
    bPlot->Plot();
}

void MainWindow::ChangeReactAxis(QCPAxis *axis)
{
    if( axis != timePlot->yAxis || axis != timePlot->yAxis2) return;
    timePlot->axisRect()->setRangeZoomAxes(timePlot->xAxis, axis);
}

void MainWindow::timePlotChangeYAxis2Range(QCPRange range)
{
    timePlot->yAxis2->setRange( range);
}

void MainWindow::ShowMousePositionInTimePlot(QMouseEvent *mouse)
{

    QPoint pt = mouse->pos();
    double x = timePlot->xAxis->pixelToCoord(pt.rx());
    double y = timePlot->yAxis->pixelToCoord(pt.ry());
    int xIndex = file->GetXIndex(x);

    QString msg;
    msg.sprintf("(x, y) = (%7.4f, %7.4f), x-index = %4d", x, y, xIndex);
    statusBar()->showMessage(msg);

}

void MainWindow::ShowMousePositionInContourPlot(QMouseEvent *mouse)
{
    QPoint pt = mouse->pos();
    double x = contourPlot->xAxis->pixelToCoord(pt.rx());
    double y = contourPlot->yAxis->pixelToCoord(pt.ry());

    int xIndex = file->GetXIndex(x);
    int yIndex = 0;
    switch (ui->comboBox_yLabelType->currentIndex()) {
    case 1:
        yIndex = file->GetYIndex_HV(y);
        break;
    case 2:
        yIndex = file->GetYIndex_HV(file->Mag2HV(y));
        break;
    default:
        yIndex = file->GetYIndex_CV(y);
        break;
    }

    QString msg;
    msg.sprintf("(x, y) = (%7.4f, %7.4f), index = (%4d, %4d)", x, y, xIndex, yIndex);
    statusBar()->showMessage(msg);

    //Plot a vertical line
    QVector<double> lineX, lineY;
    lineY.push_back(y);
    lineY.push_back(y);
    lineX.push_back(contourPlot->xAxis->range().lower);
    lineX.push_back(contourPlot->xAxis->range().upper);

    contourPlot->graph(0)->clearData();
    contourPlot->graph(0)->addData(lineX, lineY);

    lineX.clear();
    lineY.clear();
    lineX.push_back(x);
    lineX.push_back(x);
    lineY.push_back(contourPlot->yAxis->range().lower);
    lineY.push_back(contourPlot->yAxis->range().upper);

    contourPlot->graph(1)->clearData();
    contourPlot->graph(1)->addData(lineX, lineY);

    contourPlot->replot();

    PlotBFieldXLine(y);

}

void MainWindow::ShowMousePositionInBFieldPlot(QMouseEvent *mouse)
{
    QPoint pt = mouse->pos();
    double x = bFieldPlot->xAxis->pixelToCoord(pt.rx());
    double y = bFieldPlot->yAxis->pixelToCoord(pt.ry());

    int yIndex = 0;
    switch (ui->comboBox_yLabelType->currentIndex()) {
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
    statusBar()->showMessage(msg);

    //========= Plot a line
    PlotBFieldXLine(x);

    //========= Plot a line in contour
    QVector<double> lineX, lineY;
    lineY.push_back(x);
    lineY.push_back(x);
    lineX.push_back(contourPlot->xAxis->range().lower);
    lineX.push_back(contourPlot->xAxis->range().upper);

    contourPlot->graph(0)->clearData();
    contourPlot->graph(0)->addData(lineX, lineY);

    contourPlot->replot();

}

void MainWindow::SetYIndexByMouseClickInBFieldPlot(QMouseEvent *mouse)
{
    QPoint pt = mouse->pos();
    double y = bFieldPlot->xAxis->pixelToCoord(pt.rx());

    int yIndex = 0;
    switch (ui->comboBox_yLabelType->currentIndex()) {
    case 1:
        yIndex = file->GetYIndex_HV(y);
        break;
    case 2:
        yIndex = file->GetYIndex_HV(file->Mag2HV(y));
        break;
    default:
        yIndex = file->GetYIndex_CV(y);
        break;
    }

    ui->spinBox_y->setValue(yIndex);
}

void MainWindow::SetXIndexByMouseClick(QMouseEvent *mouse)
{

    if( controlPressed){
        QPoint pt = mouse->pos();
        double x = timePlot->xAxis->pixelToCoord(pt.rx());
        int xIndex = file->GetXIndex(x);
        if( mouse->button() == Qt::LeftButton){
            ui->spinBox_x->setValue(xIndex);
            if( xIndex > ui->spinBox_x2->value()){
                ui->spinBox_x2->setValue( ui->spinBox_x2->maximum());
            }
        }else if(mouse->button() == Qt::RightButton){
            if( xIndex < ui->spinBox_x->value()){
                xIndex = ui->spinBox_x2->maximum();
            }
            ui->spinBox_x2->setValue(xIndex);
        }
    }

}

void MainWindow::SetYIndexByMouseClick(QMouseEvent *mouse)
{
    QPoint pt = mouse->pos();
    double y = contourPlot->yAxis->pixelToCoord(pt.ry());

    int yIndex = 0;
    switch (ui->comboBox_yLabelType->currentIndex()) {
    case 1:
        yIndex = file->GetYIndex_HV(y);
        break;
    case 2:
        yIndex = file->GetYIndex_HV(file->Mag2HV(y));
        break;
    default:
        yIndex = file->GetYIndex_CV(y);
        break;
    }

    ui->spinBox_y->setValue(yIndex);
}

void MainWindow::OpenFile(QString fileName, int kind)
{
    QString msg = "Openning file : ";
    msg.append(fileName);
    Write2Log(msg);

    //======== If no file selected.
    if(fileName == "") {
        statusBar()->showMessage("No file was selected.");
        return;
    }

    //======== show file name
    statusBar()->showMessage("Selected a file.");
    ui->lineEdit->setText(fileName);

    //======== new FileIO
    if( file != NULL) delete file;
    file = new FileIO(fileName);
    connect(file, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));

    file->OpenHV2MagParametersFile();

    //======== read data structure according to the seleteced data format.
    if( kind == 1){
        file->OpenDoubleXCSVData();
    }else if( kind == 2){
        file->OpenTxtData_col();
    }else if( kind == 0){
        file->OpenTxtData_row();
    }
    //======== if some shit happen, abort.
    if(file->IsOpen() == 0) {
        Write2Log("Cannot open file.");
        return;
    }

    Write2Log("-------------------- data stored in memory");

    //========= once the file is opened, eanble planel, set checkBoxes to uncheck, etc...
    setEnabledPlanel(1);

    ui->checkBox_BGsub->setChecked(0);
    ui->checkBox_MeanCorr->setChecked(0);
    ui->spinBox_MovingAvg->setValue(-1);
    ui->verticalSlider_zOffset->setValue(0);
    ui->spinBox_y->setMinimum(0);
    ui->spinBox_y->setMaximum(file->GetYDataSize()-1);
    ui->spinBox_x->setMinimum(0);
    ui->spinBox_x->setMaximum(file->GetXDataSize()-1);
    ui->spinBox_x2->setMinimum(0);
    ui->spinBox_x2->setMaximum(file->GetXDataSize()-1);
    ui->spinBox_BGIndex->setMinimum(0);
    ui->spinBox_BGIndex->setMaximum(file->GetYDataSize()-1);

    //========= Reset Data in fitResultDialog
    fitResultPlot->ClearData();
    fitResultPlot->SetDataSize(file);
    fitResultPlot->SetFilePath(file->GetFilePath());

    //======== Plot B-plot, need to be done before PlotAllPlots()
    bPlot->SetData(file);

    ui->spinBox_x1_B->setMinimum(0);
    ui->spinBox_x1_B->setMaximum(file->GetXDataSize()-1);
    ui->spinBox_x2_B->setMinimum(0);
    ui->spinBox_x2_B->setMaximum(file->GetXDataSize()-1);

    allowBFieldPlot = false;
    ui->spinBox_x1_B->setValue(bPlot->FindstartIndex(-1));
    ui->spinBox_x2_B->setValue(bPlot->FindstartIndex(20));
    ui->lineEdit_x1_B->setText("-1 us");
    ui->lineEdit_x2_B->setText("20 us");
    allowBFieldPlot = true;

    //======== SetData to fftPlot
    fftPlot->SetData(file);

    //=========== Set up Plots
    SetupPlots();

    //========================= Plot
    if( file->HasBackGround()){
        on_checkBox_BGsub_clicked(true); // this contain PlotAllPlots()
    }else{
        on_checkBox_BGsub_clicked(false);
    }

    int xIndex = ana->FindXIndex(TIME1);
    ui->spinBox_x->setValue(xIndex);
    ui->spinBox_x2->setValue(ui->spinBox_x2->maximum());

}

void MainWindow::on_pushButton_OpenFile_clicked(){

    //close all planels
    if(fitResultPlot->isVisible()) fitResultPlot->hide();
    if(bPlot->isVisible()) bPlot->hide();
    if(fftPlot->isVisible()) fftPlot->hide();
    ui->checkBox_AutoFit->setChecked(false);
    savedSingleXCVS = 0;
    Write2Log("##########################################");


    QFileDialog fileDialog(this);
    QStringList filters;
    //======== set allowed data structure.
    filters << "Row-wise (*txt *dat *.*)" << "Double-X CSV(*.csv)" << "Col-wise (*.txt *.dat *.csv *.*)" ;
    fileDialog.setNameFilters(filters);
    fileDialog.setReadOnly(1);
    fileDialog.setDirectory(DATA_PATH);
    QString fileName;
    //======== Open read the first file
    if( fileDialog.exec()) {
        QStringList fileNames = fileDialog.selectedFiles();
        fileName = fileNames[0];
    }

    //======== read data structure according to the seleteced data format.
    int kind = 0;
    if( fileDialog.selectedNameFilter() == filters[1]){
        kind = 1;
    }else if(fileDialog.selectedNameFilter() == filters[2]){
        kind = 2;
    }else if(fileDialog.selectedNameFilter() == filters[0]){
        kind = 0;
    }

    if( fileDialog.selectedNameFilter() == filters[1]){
        ui->comboBox_yLabelType->setEnabled(0);
        ui->comboBox_yLabelType->setCurrentIndex(0);
    }

    OpenFile(fileName, kind);

}


void MainWindow::on_spinBox_y_valueChanged(int arg1)
{
    if( !allowTimePlot ) return;
    statusBar()->showMessage("Changed y-Index.");

    // this is for calling
    ui->spinBox_y->setValue(arg1);

    QString unitText = " V";
    double yValue = 0;
    switch (ui->comboBox_yLabelType->currentIndex()) {
    case 0:
        unitText = " V";
        yValue = file->GetDataY_CV(arg1);
        break;
    case 1:
        unitText = " mV";
        yValue = file->GetDataY_HV(arg1);
        break;
    case 2:
        unitText = " mT";
        yValue = file->GetDataY_HV(arg1);
        yValue = file->HV2Mag(yValue);
        break;
    }

    ui->lineEdit_y->setText(QString::number(yValue)+ unitText);
    ui->lineEdit_yName->setText(file->GetDataYName(arg1));

    ana->SetY(arg1, file->GetDataY_CV(arg1));
    ana->SetData(file->GetDataSetX(), file->GetDataSetZ(arg1));
    Write2Log(" y-Index = " + QString::number(arg1) + ", Sample S.D. : " + QString::number(sqrt(ana->GetSampleVariance())));
    PlotTimePlot(0, file->GetDataSetX(), file->GetDataSetZ(arg1));

    //--- set x scroll bar
    int rxMin = qRound(file->GetXMin());
    int rxMax = qRound(file->GetXMax());
    int delta = rxMax - rxMin;
    ui->horizontalScrollBar->setMinimum(rxMin + delta/2);
    ui->horizontalScrollBar->setMaximum(rxMin - delta/2);
    ui->horizontalScrollBar->setPageStep(delta);
    ui->horizontalScrollBar->setSingleStep(1);

    if( ui->checkBox_AutoFit->isChecked()) {
        on_pushButton_guessPars_clicked();
        on_pushButton_Fit_clicked();
    }else{
        //int xIndex = ana->FindXIndex( TIME1 );
        //ui->spinBox_x->setValue(xIndex);
        PlotFitFuncAndXLines();
    }

    // draw vertical line for y-index;
    bFieldPlot->graph(2)->clearData();
    QVector<double> line_x, line_y;
    line_x.push_back(yValue);
    line_x.push_back(yValue);
    line_y.push_back(bFieldPlot->yAxis->range().lower);
    line_y.push_back(bFieldPlot->yAxis->range().upper);
    bFieldPlot->graph(2)->addData(line_x,line_y);
    bFieldPlot->replot();
}

void MainWindow::on_spinBox_x_valueChanged(int arg1){
    statusBar()->showMessage("Changed x-Index Start.");
    ui->lineEdit_x->setText(QString::number(file->GetDataX(arg1))+ " us");
    ana->SetStartFitIndex(arg1);
    PlotFitFuncAndXLines();
}
void MainWindow::on_spinBox_x2_valueChanged(int arg1)
{
    statusBar()->showMessage("Changed x-Index End.");
    ui->lineEdit_x2->setText(QString::number(file->GetDataX(arg1))+ " us");
    ana->SetEndFitIndex(arg1);
    PlotFitFuncAndXLines();
}

void MainWindow::PlotFitFuncAndXLines(){

    if( file == NULL) return;

    QVector<double> par = GetParametersFromLineText();
    if( ui->comboBox_fitFunctionType->currentIndex() == 2){
        par = ana->GetParameters();
    }

    ana->CalFitData(par);
    PlotTimePlot(1, ana->GetData_x(), ana->GetFitData_y());

    //================Draw X-lines
    int xIndex1 = ui->spinBox_x->value();
    double x1 = file->GetDataX(xIndex1);
    int xIndex2 = ui->spinBox_x2->value();
    double x2 = file->GetDataX(xIndex2);
    QVector<double> xline_y, xline_x1, xline_x2;

    xline_y.push_back(timePlot->yAxis->range().lower * 3);
    xline_y.push_back(timePlot->yAxis->range().upper * 3);
    xline_x1.push_back(x1);
    xline_x1.push_back(x1);
    xline_x2.push_back(x2);
    xline_x2.push_back(x2);

    PlotTimePlot(2, xline_x1, xline_y);
    PlotTimePlot(3, xline_x2, xline_y);

}

void MainWindow::on_lineEdit_a_returnPressed(){
    statusBar()->showMessage("Changed parameter a.");
    PlotFitFuncAndXLines();
}

void MainWindow::on_lineEdit_Ta_returnPressed(){
    statusBar()->showMessage("Changed parameter Ta.");
    PlotFitFuncAndXLines();
}

void MainWindow::on_lineEdit_b_returnPressed(){
    statusBar()->showMessage("Changed parameter b.");
    PlotFitFuncAndXLines();
}

void MainWindow::on_lineEdit_Tb_returnPressed(){
    statusBar()->showMessage("Changed parameter Tb.");
    PlotFitFuncAndXLines();
}
void MainWindow::on_lineEdit_c_returnPressed()
{
    statusBar()->showMessage("Changed parameter c.");
    PlotFitFuncAndXLines();
}


void MainWindow::on_pushButton_Fit_clicked(){

    if( file == NULL) return;

    statusBar()->showMessage("Fitting.");

    bool gnu = ui->checkBox_GunFit->isChecked();

    if( savedSingleXCVS == 0 && gnu){
        file->SaveCSV(0,1); // save as single-X
        savedSingleXCVS = 1;
    }

    Msg.sprintf("=================================== %d, gnufit? %d", ana->GetYIndex(), gnu);
    Write2Log(Msg);

    QVector<double> par = GetParametersFromLineText();

    double lambda = ui->lineEdit_lambda->text().toDouble();
    int maxIter = ui->lineEdit_MaxIter->text().toInt();
    int torr = ui->lineEdit_TORR->text().toInt();
    //ana->Print();

    ana->Setlambda(lambda);
    ana->SetMaxInteration(maxIter);
    ana->SetTORR( pow(10,torr) );
    ana->NonLinearFit(par, gnu);

    //display result
    ana->PrintCoVarMatrix();
    Msg.sprintf("last Lambda : %e", ana->GetLambda());
    Write2Log(Msg);
    Write2Log("===============================");
    ana->PrintVector(ana->GetParameters(), "sol");
    ana->PrintVector(ana->GetParError(), "error");

    if( ana->GetFitFlag() & 4) ui->textEdit->setTextColor(QColor(255,0,0,255));
    ana->PrintVector(ana->GetSSRgrad(), "SSR grad");
    ui->textEdit->setTextColor(QColor(0,0,0,255)); // reset to black, no matter what.

    if( ana->GetFitFlag() & 8) ui->textEdit->setTextColor(QColor(255,0,0,255));
    ana->PrintVector(ana->GetParPValue(), "p-Value");
    ui->textEdit->setTextColor(QColor(0,0,0,255)); // reset to black, no matter what.


    Msg.sprintf("DF : %d, SSR: %f, delta: %e", ana->GetNDF(), ana->GetSSR(), ana->GetDelta() );
    Write2Log(Msg);
    double chisq = ana->GetFitVariance()/ana->GetSampleVariance();
    Msg.sprintf("Fit Variance : %f, Sample Variance : %f, Chi-sq/ndf : %5.3f", ana->GetFitVariance(), ana->GetSampleVariance(), chisq);
    Write2Log(Msg);

    if( ana->GetFitFlag() != 0) {
        ui->textEdit->setTextColor(QColor(255,0,0,255));
        Msg.sprintf("!!!!!!!!!!!!!! fit not good. Fit-Flag = %d.", ana->GetFitFlag());
        Write2Log(Msg);
        if( ana->GetFitFlag() & 4) {
            Write2Log(" ---- Grad(SSR) is too large.");
        }
        if( ana->GetFitFlag() & 8) {
            Write2Log(" ---- p-Value(s) is too large.");
        }
    }
    ui->textEdit->setTextColor(QColor(0,0,0,255));

    //Write2Log(ana->GetFitMsg());

    // update the parameter
    if(ui->comboBox_fitFunctionType->currentIndex() != 2){
        UpdateLineTextParameters(ana->GetParameters(), ana->GetParError());
        fitResultPlot->FillData(ana);
        fitResultPlot->PlotData();
    }

    PlotFitFuncAndXLines();
}

void MainWindow::on_pushButton_guessPars_clicked()
{
    if( file == NULL) return;
    statusBar()->showMessage("Reset parameters to default values.");

    double c = ana->GetSampleMean();
    ui->lineEdit_c->setText(QString::number(c));

    int funcType = ui->comboBox_fitFunctionType->currentIndex();

    double f1 = ana->GetDataYMax();
    double fm = ana->GetDataYMin();
    if( funcType == 0){
        double sBGvar = ana->GetSampleVariance();
        f1 = f1-c;
        fm = fm-c;
        if( fabs(f1) < fabs(fm)) {
            double tmp = f1;
            f1 = fm;
            fm = tmp;
        }

        fm = fm + sqrt(sBGvar);

        double Ta = ana->FindXFromYAfterTZero(0)*1./2.;
        QString x1str = ui->lineEdit_x->text();
        x1str.chop(2);
        double x1 = x1str.toDouble();
        double a = f1 * exp(x1 / Ta);

        qDebug() << "f1 = " << f1;
        qDebug() << "Ta = " << Ta;
        qDebug() << "fm = " << fm;
        qDebug() << "x1 = " << x1;

        ui->lineEdit_a->setText(QString::number(a));
        ui->lineEdit_b->setText(QString::number(fm*2));
        ui->lineEdit_Ta->setText(QString::number(Ta));
        ui->lineEdit_Tb->setText("80");

    }else if (funcType == 1){

        ui->lineEdit_a->setText(QString::number((f1-fm)/2));
        ui->lineEdit_b->setText("3");
        ui->lineEdit_Ta->setText("50");
        ui->lineEdit_Tb->setText("80");
    }

    ui->lineEdit_ea->setText("");
    ui->lineEdit_eTa->setText("");
    ui->lineEdit_eb->setText("");
    ui->lineEdit_eTb->setText("");
    ui->lineEdit_ec->setText("");
    ui->lineEdit_P->setText("");
    ui->lineEdit_eP->setText("");

    PlotFitFuncAndXLines();
}

void MainWindow::on_pushButton_save_clicked()
{
    if( file == NULL) return;
    statusBar()->showMessage("Save fitted parameters.");
    file->OpenSaveFileforFit();
    file->SaveFitResult(ana);
    file->CloseSaveFileforFit();
}

void MainWindow::on_pushButton_FitAll_clicked()
{
    if( file == NULL) return;
    statusBar()->showMessage("Fitting all data.");
    int n = file->GetYDataSize();
    QProgressDialog progress("Fitting...", "Abort", 0, n-1, this);
    progress.setWindowModality(Qt::WindowModal);
    QString str;
    int count = 0;

    int yStartIndex = 0;
    if( file->HasBackGround()) {
        //on_checkBox_BGsub_clicked(true);
        yStartIndex = 1;
    }


    QVector<QString> badFitdSSR;
    badFitdSSR.clear();

    file->OpenSaveFileforFit();

    for( int yIndex = yStartIndex; yIndex < n ; yIndex ++){
    //for( int yIndex = 100; yIndex < 300 ; yIndex ++){
        on_pushButton_guessPars_clicked();
        ui->spinBox_y->setValue(yIndex);
        on_pushButton_Fit_clicked();
        PlotFitFuncAndXLines();
        //Sleep(500);
        str.sprintf("Fitting #%d / %d , saved %d", yIndex + 1, n, count + 1);
        progress.setLabelText(str);
        progress.setValue(yIndex);
        if(progress.wasCanceled()) break;

        QVector<double> gradSSR = ana->GetSSRgrad();
        if( ana->GetFitFlag() !=0){
            int size = gradSSR.size();
            QString msg, tmp;
            msg.sprintf("%4d | fitFlag: %1d | dSSR : [ ", yIndex, ana->GetFitFlag());
            for(int i = 0; i < size - 1; i++){
                tmp.sprintf(" %7.3f,", gradSSR[i]);
                msg += tmp;
            }
            tmp.sprintf(" %7.3f]", gradSSR[size-1]);
            msg += tmp;
            badFitdSSR.push_back(msg);
        }


        file->SaveFitResult(ana);
        count ++;
    }

    file->CloseSaveFileforFit();

    Write2Log("======== Possible Bad Fit y-Index:");
    for(int i = 0; i < badFitdSSR.size(); i++ ){
        Write2Log(badFitdSSR[i]);
    }
    QString msg;
    msg.sprintf("======== number of bad fit : %d.", badFitdSSR.size());
    Write2Log(msg);
    Write2Log("=================== End of Fit ALL.");
}

void MainWindow::on_checkBox_b_Tb_clicked(bool checked)
{

    ui->lineEdit_b->setEnabled(checked);
    ui->lineEdit_Tb->setEnabled(checked);
    ui->lineEdit_P->setEnabled(checked);

    if( checked ){
        statusBar()->showMessage("Enable b and Tb.");
        if(ui->checkBox_c->isChecked() ){
            fitResultPlot->SetAvalibleData(5);
        }else{
            fitResultPlot->SetAvalibleData(4);
        }
    }else{
        statusBar()->showMessage("Disable b and Tb.");
        if(ui->checkBox_c->isChecked() ){
            fitResultPlot->SetAvalibleData(3);
        }else{
            fitResultPlot->SetAvalibleData(2);
        }
    }

}

void MainWindow::on_checkBox_c_clicked(bool checked)
{
    ui->lineEdit_c->setEnabled(checked);

    if( checked ){
        statusBar()->showMessage("Enable c.");
        if(ui->checkBox_b_Tb->isChecked() ){
            fitResultPlot->SetAvalibleData(5);
        }else{
            fitResultPlot->SetAvalibleData(3);
        }
    }else{
        statusBar()->showMessage("Disable c.");
        if(ui->checkBox_b_Tb->isChecked() ){
            fitResultPlot->SetAvalibleData(4);
        }else{
            fitResultPlot->SetAvalibleData(2);
        }
    }

}

QVector<double> MainWindow::GetParametersFromLineText()
{
    QVector<double> par;

    if(ui->comboBox_fitFunctionType->currentIndex() != 2){
        par.push_back(ui->lineEdit_a->text().toDouble());
        par.push_back(ui->lineEdit_Ta->text().toDouble());
        if( ui->checkBox_b_Tb->isChecked()){
            par.push_back(ui->lineEdit_b->text().toDouble());
            par.push_back(ui->lineEdit_Tb->text().toDouble());
        }
        if( ui->checkBox_c->isChecked()){
            par.push_back(ui->lineEdit_c->text().toDouble());
        }
    }else if(ui->comboBox_fitFunctionType->currentIndex() == 2){
        par = this->par;
    }

    return par;
}

void MainWindow::UpdateLineTextParameters(QVector<double> par, QVector<double> epar)
{
    double a = fabs(par[0]);
    double b = fabs(par[2]);
    double ea = epar[0];
    double eb = epar[2];

    double P = (a-b)/(a+b);
    double eP = 1/pow(a+b,2)*sqrt(pow(ea,2)+ pow(eb,2)) * sqrt(2*(pow(a,2)+pow(b,2)));


    ui->lineEdit_a ->setText(QString::number(par[0]));
    ui->lineEdit_Ta->setText(QString::number(par[1]));
    if( par.size() == 3){
        ui->lineEdit_c->setText(QString::number(par[2]));
        ui->lineEdit_b ->setText("");
        ui->lineEdit_Tb->setText("");
        ui->lineEdit_P->setText("");
    }
    if( par.size() == 4 ){
        ui->lineEdit_P->setText(QString::number(P));
        ui->lineEdit_b ->setText(QString::number(par[2]));
        ui->lineEdit_Tb->setText(QString::number(par[3]));
        ui->lineEdit_c->setText("");
    }
    if( par.size()== 5){
        ui->lineEdit_P->setText(QString::number(P));
        ui->lineEdit_b ->setText(QString::number(par[2]));
        ui->lineEdit_Tb->setText(QString::number(par[3]));
        ui->lineEdit_c->setText(QString::number(par[4]));
    }

    ui->lineEdit_ea ->setText(QString::number(epar[0]));
    ui->lineEdit_eTa->setText(QString::number(epar[1]));
    if( par.size() == 3){
        ui->lineEdit_ec->setText(QString::number(epar[2]));
        ui->lineEdit_eb ->setText("");
        ui->lineEdit_eTb->setText("");
        ui->lineEdit_eP->setText("");
    }
    if( par.size() == 4 ){
        ui->lineEdit_eP->setText(QString::number(eP));
        ui->lineEdit_eb ->setText(QString::number(epar[2]));
        ui->lineEdit_eTb->setText(QString::number(epar[3]));
        ui->lineEdit_ec->setText("");
    }
    if( par.size()== 5){
        ui->lineEdit_eP->setText(QString::number(eP));
        ui->lineEdit_eb ->setText(QString::number(epar[2]));
        ui->lineEdit_eTb->setText(QString::number(epar[3]));
        ui->lineEdit_ec->setText(QString::number(epar[4]));
    }

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QApplication::closeAllWindows();
    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent *key)
{
    if(key->key() == Qt::Key_Shift ){
        //qDebug() << "Shift pressed";
        timePlot->axisRect()->setRangeDrag(Qt::Vertical);
        timePlot->axisRect()->setRangeZoom(Qt::Vertical);
        bFieldPlot->axisRect()->setRangeDrag(Qt::Vertical);
        bFieldPlot->axisRect()->setRangeZoom(Qt::Vertical);
    }

    if(key->key() == Qt::Key_Control){
        controlPressed = 1;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *key)
{
    if(key->key() == Qt::Key_Shift ){
        timePlot->axisRect()->setRangeDrag(Qt::Horizontal);
        timePlot->axisRect()->setRangeZoom(Qt::Horizontal);
        bFieldPlot->axisRect()->setRangeDrag(Qt::Horizontal);
        bFieldPlot->axisRect()->setRangeZoom(Qt::Horizontal);
    }
    if(key->key() == Qt::Key_Control){
        //qDebug() << " Ctrl released";
        controlPressed = 0;
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData * mimeData = event->mimeData();
    QString filePath = mimeData->urls().at(0).toLocalFile();
    if(filePath.right(3) == "dat"){
        Write2Log("Open .dat file by Drag. Assume row-wise.");
        ui->lineEdit->setText(filePath);
        OpenFile(filePath, 0);
        return;
    }
    if(filePath.right(3) == "csv"){
        Write2Log("Open *.csv file by Drag. Assume double-X col-wise.");
        ui->lineEdit->setText(filePath);
        OpenFile(filePath, 1);
        return;
    }else{
        Write2Log("Unrecongized file format. Drag fail.");
        return;
    }

}

void MainWindow::PlotContourPlot(double offset)
{
    int yIndexStart = 0;
    if( file->HasBackGround()) yIndexStart = 1;

    int nx = file->GetXDataSize();
    int ny = file->GetYDataSize();

    for(int xIndex = 0; xIndex < nx; xIndex++){
        for(int yIndex = yIndexStart; yIndex < ny; yIndex++){
            double x = file->GetDataX(xIndex);
            double y , dy;
            switch (ui->comboBox_yLabelType->currentIndex()) {
            case 1:
                y = file->GetDataY_HV(yIndex);
                dy = file->GetYStep_HV();
                break;
            case 2:
                y = file->HV2Mag(file->GetDataY_HV(yIndex));
                dy = file->HV2Mag(file->GetYStep_HV());
                break;
            default:
                y = file->GetDataY_CV(yIndex);
                dy = 0;
                break;
            }
            double z = file->GetDataZ(xIndex, yIndex) + offset;

            colorMap->data()->setData(x, y, z);

            // this is need for plotting Hall Volatge, since not uniform;
            if( ui->comboBox_yLabelType->currentIndex() > 0 && yIndex < ny - 1){
                double zn = file->GetDataZ(xIndex, yIndex+1) + offset;
                colorMap->data()->setData(x, y+dy/2, (z + zn)/2);
            }
        }
    }

    //Clean line, otherwise, the plot range will mess up;
    contourPlot->graph(0)->clearData();
    contourPlot->graph(1)->clearData();

    contourPlot->rescaleAxes();

    contourPlot->replot();
}

void MainWindow::on_actionFit_Result_triggered()
{
    if(fitResultPlot->isHidden()){
        fitResultPlot->show();
        fitResultPlot->PlotData();
    }
}

void MainWindow::on_actionB_Plot_triggered()
{
    if(bPlot->isHidden()){
        bPlot->show();
        bPlot->SetPlotUnit(ui->comboBox_yLabelType->currentIndex());
        bPlot->Plot();
    }
}

void MainWindow::setEnabledPlanel(bool IO)
{
    ui->checkBox_AutoFit->setEnabled(IO);

    ui->actionB_Plot->setEnabled(IO);
    ui->actionFFTW_Plot->setEnabled(IO);
    ui->actionFit_Result->setEnabled(IO);
    ui->actionDWT_Plot->setEnabled(IO);

    ui->pushButton_Fit->setEnabled(IO);
    ui->pushButton_FitAll->setEnabled(IO);
    ui->pushButton_guessPars->setEnabled(IO);
    ui->pushButton_save->setEnabled(IO);
    ui->pushButton_RestoreData->setEnabled(IO);
    ui->spinBox_y->setEnabled(IO);
    ui->spinBox_BGIndex->setEnabled(IO);
    ui->spinBox_x->setEnabled(IO);
    ui->spinBox_x2->setEnabled(IO);
    ui->checkBox_MeanCorr->setEnabled(IO);
    ui->verticalSlider_zOffset->setEnabled(IO);
    ui->verticalSlider_z->setEnabled(IO);
    ui->checkBox_BGsub->setEnabled(IO);
    ui->spinBox_MovingAvg->setEnabled(IO);
    ui->comboBox_yLabelType->setEnabled(IO);

    ui->actionConvert_Origin_Data_as_Double_X_CVS->setEnabled(IO);
    ui->actionConvert_Origin_Data_as_Single_X_CVS->setEnabled(IO);
    ui->actionSave_as_Double_X_CSV->setEnabled(IO);
    ui->actionSave_as_Single_X_CSV->setEnabled(IO);
    ui->actionSave_data->setEnabled(IO);

    ui->actionSave_Plot_as_PDF->setEnabled(IO);
    ui->actionSave_Contour_Plot_as_PDF->setEnabled(IO);
    ui->actionSave_BFieldPlot_as_PDF->setEnabled(IO);
    ui->actionSave_Time_Plot_w_o_lines->setEnabled(IO);
    ui->actionSave_all_Plot->setEnabled(IO);

    ui->actionStatistics->setEnabled(IO);

    ui->spinBox_x1_B->setEnabled(IO);
    ui->spinBox_x2_B->setEnabled(IO);

    ui->comboBox_fitFunctionType->setEnabled(IO);
}

void MainWindow::on_checkBox_MeanCorr_clicked(bool checked)
{
    int id = 0;
    int bgIndex = -1;
    if( checked ) id = id + 2;
    if( ui->checkBox_BGsub->isChecked()) {
        id = id + 1;
        bgIndex = ui->spinBox_BGIndex->value();
    }

    file->ManipulateData(id, bgIndex);

    PlotAllPlots();

}

void MainWindow::on_checkBox_BGsub_clicked(bool checked)
{
    ui->spinBox_BGIndex->setEnabled(checked);
    ui->checkBox_BGsub->setChecked(checked);
    int bgYIndex = ui->spinBox_BGIndex->value();

    int id = 0;

    if( checked ) id = id + 1;
    if( ui->checkBox_MeanCorr->isChecked()) id = id + 2;

    file->ManipulateData(id, bgYIndex);

    PlotAllPlots();

}

void MainWindow::on_spinBox_BGIndex_valueChanged()
{
    on_checkBox_BGsub_clicked(1);
}

void MainWindow::on_verticalSlider_z_sliderMoved(int position)
{
    colorMap->setDataRange(QCPRange(-position, position));
    contourPlot->replot();
}

void MainWindow::on_actionFFTW_Plot_triggered()
{
    if(fftPlot->isHidden()){
        fftPlot->show();
    }

}

void MainWindow::on_pushButton_RestoreData_clicked()
{
    //file->RestoreData();
    if( file->HasBackGround()){
        file->ManipulateData(1, ui->spinBox_BGIndex->value());
        ui->checkBox_BGsub->setChecked(1);
    }else{
        file->ManipulateData(0);
        ui->checkBox_BGsub->setChecked(0);
    }

    ui->checkBox_MeanCorr->setChecked(0);
    ui->verticalSlider_zOffset->setValue(0);
    ui->spinBox_MovingAvg->setValue(-1);

    PlotAllPlots();
}

void MainWindow::on_verticalSlider_zOffset_sliderMoved(int position)
{
    PlotContourPlot(position);
}

void MainWindow::on_spinBox_MovingAvg_valueChanged(int arg1)
{
    int id = 0;
    int bgIndex = -1;

    if( arg1 > -1) {
        id = id + 4;
    }

    if( ui->checkBox_BGsub->isChecked()){
        id = id + 1;
        bgIndex = ui->spinBox_BGIndex->value();
    }

    ui->checkBox_MeanCorr->setChecked(1);
    id = id + 2;

    file->ManipulateData(id, bgIndex, arg1);

    PlotAllPlots();
}

void MainWindow::on_actionSave_as_Single_X_CSV_triggered()
{
    file->SaveCSV(0,0); // single-X
}

void MainWindow::on_actionSave_as_Double_X_CSV_triggered()
{
    file->SaveCSV(1,0); // double-X
    savedSingleXCVS = 1;
}


void MainWindow::on_comboBox_yLabelType_currentIndexChanged(int index)
{
    double xMin = file->GetXMin();
    double xMax = file->GetXMax();
    double yMin = file->GetYMin_CV();
    double yMax = file->GetYMax_CV();
    QString unitText = " V";
    int yIndex = ui->spinBox_y->value();
    if(index == 0){ //Control Volatge
        contourPlot->yAxis->setLabel("Ctrl. Vol. [V]");
        contourPlot->yAxis->setRange(yMin, yMax);
        unitText = " V";
        ui->lineEdit_y->setText(QString::number(file->GetDataY_CV(yIndex))+ unitText);

    }else if(index == 1){ // Hall Volatage
        contourPlot->yAxis->setLabel("Hall Vol. [mV]");
        yMin = file->GetYMin_HV();
        yMax = file->GetYMax_HV();
        contourPlot->yAxis->setRange(yMin, yMax);
        unitText = " mV";
        ui->lineEdit_y->setText(QString::number(file->GetDataY_HV(yIndex))+ unitText);

    }else if(index == 2){ // Magnetic field [T]
        contourPlot->yAxis->setLabel("B-field [mT]");
        yMin = file->HV2Mag(file->GetYMin_HV());
        yMax = file->HV2Mag(file->GetYMax_HV());
        contourPlot->yAxis->setRange(yMin, yMax);
        unitText = " mT";
        ui->lineEdit_y->setText(QString::number(file->HV2Mag(file->GetDataY_HV(yIndex)))+ unitText);

    }

    bPlot->SetPlotUnit(ui->comboBox_yLabelType->currentIndex());
    bPlot->Plot();
    PlotBFieldPlot();

    colorMap->data()->setRange(QCPRange(xMin, xMax), QCPRange(yMin, yMax));

    PlotContourPlot(ui->verticalSlider_zOffset->value());
    //ctplot->replot();

    fitResultPlot->SetPlotUnit(ui->comboBox_yLabelType->currentIndex());
    fitResultPlot->PlotData();

}

void MainWindow::on_actionConvert_Origin_Data_as_Single_X_CVS_triggered()
{
    file->SaveCSV(0,1); // single-X
    savedSingleXCVS = 1;
}

void MainWindow::on_actionConvert_Origin_Data_as_Double_X_CVS_triggered()
{
    file->SaveCSV(1,1); // double-X
}

void MainWindow::on_actionSave_Plot_as_PDF_triggered()
{
    QFileDialog fileDialog(this);
    QStringList nameFilterList = {"pdf (*.pdf)", "PNG (*.png)"};
    fileDialog.setNameFilters(nameFilterList);
    fileDialog.setDirectory(DESKTOP_PATH);
    fileDialog.setReadOnly(0);
    QString fileName;
    if( fileDialog.exec()){
        fileName = fileDialog.selectedFiles()[0];
    }

    bool ok = false;
    int ph = timePlot->geometry().height();
    int pw = timePlot->geometry().width();
    if( fileDialog.selectedNameFilter() == nameFilterList[0]){
        if( fileName.right(4) != ".pdf" ) fileName.append(".pdf");
        ok = timePlot->savePdf(fileName, pw, ph );
    }else{
        if( fileName.right(4) != ".png" ) fileName.append(".png");
        ok = timePlot->savePng(fileName, pw, ph );

    }

    if( ok ){
        Write2Log("Saved Time-Plot as " + fileName);
    }else{
        Write2Log("Save Failed.");
    }
}

void MainWindow::on_actionSave_Contour_Plot_as_PDF_triggered()
{
    QFileDialog fileDialog(this);
    QStringList nameFilterList = {"pdf (*.pdf)", "PNG (*.png)"};
    fileDialog.setNameFilters(nameFilterList);
    fileDialog.setDirectory(DESKTOP_PATH);
    fileDialog.setReadOnly(0);
    QString fileName;
    if( fileDialog.exec()){
        fileName = fileDialog.selectedFiles()[0];
    }

    bool ok = false;
    int ph = contourPlot->geometry().height();
    int pw = contourPlot->geometry().width();
    //clear the lines;
    contourPlot->graph(0)->clearData();
    contourPlot->graph(1)->clearData();

    if( fileDialog.selectedNameFilter() == nameFilterList[0]){
        if( fileName.right(4) != ".pdf" ) fileName.append(".pdf");
        ok = contourPlot->savePdf(fileName, pw, ph );
    }else{
        if( fileName.right(4) != ".png" ) fileName.append(".png");
        ok = contourPlot->savePng(fileName, pw, ph );
    }

    if( ok ){
        Write2Log("Saved Contour-Plot as " + fileName);
    }else{
        Write2Log("Save Failed.");
    }
}

void MainWindow::on_actionSave_data_triggered()
{
    file->SaveTxtData_row();
}

void MainWindow::timePlotXAxisChanged(QCPRange range)
{
    if( file == NULL ) return;
    double xMin = file->GetXMin();
    double xMax = file->GetXMax();
    //regulate the xAxis
    if( range.upper > xMax){
        timePlot->xAxis->setRangeUpper(xMax);
        range = timePlot->xAxis->range();
    }

    if( range.lower < xMin){
        timePlot->xAxis->setRangeLower(xMin);
        range = timePlot->xAxis->range();
    }
    //set scroll bar
    int rxMin = qRound(file->GetXMin());
    int rxMax = qRound(file->GetXMax());
    int delta = qRound(range.size());
    //qDebug() << delta << "," << rxMin + delta/2 << "," << rxMax - delta/2;
    ui->horizontalScrollBar->setMinimum(rxMin + delta/2);
    ui->horizontalScrollBar->setMaximum(rxMax - delta/2);
    ui->horizontalScrollBar->setPageStep(delta);
    ui->horizontalScrollBar->setValue(qRound(range.center()));

    //Set xAxis2
    timePlot->xAxis2->setRangeLower(ana->FindXIndex(timePlot->xAxis->range().lower));
    timePlot->xAxis2->setRangeUpper(ana->FindXIndex(timePlot->xAxis->range().upper));
}

void MainWindow::bFieldPlotXAxisChanged(QCPRange range)
{
    if( file == NULL ) return;
    double xMin = 0;
    double xMax = 0;
    int plotUnit = ui->comboBox_yLabelType->currentIndex();
    switch (plotUnit) {
    case 0:
        xMin = file->GetYMin_CV();
        xMax = file->GetYMax_CV();
        break;
    case 1:
        xMin = file->GetYMin_HV();
        xMax = file->GetYMax_HV();
        break;
    case 2:
        xMin = file->HV2Mag(file->GetYMin_HV());
        xMax = file->HV2Mag(file->GetYMax_HV());
        break;
    }
    //regulate the xAxis
    if( range.upper > xMax){
        bFieldPlot->xAxis->setRangeUpper(xMax);
        range = bFieldPlot->xAxis->range();
    }

    if( range.lower < xMin){
        bFieldPlot->xAxis->setRangeLower(xMin);
        range = bFieldPlot->xAxis->range();
    }

    // change xAxis2
    if(plotUnit == 0){
        if(file->IsYRevered()){
            bFieldPlot->xAxis2->setRangeUpper(file->FindIndex(file->GetDataSetY(), 0.01, range.lower, 0));
            bFieldPlot->xAxis2->setRangeLower(file->FindIndex(file->GetDataSetY(), 0.01, range.upper, 1));
        }else{
            //bFieldPlot->xAxis2->setRangeUpper();
        }
    }
}

void MainWindow::bFieldPlotChangeYAxis2Range(QCPRange range)
{
    bFieldPlot->yAxis2->setRange( range);
}

void MainWindow::on_horizontalScrollBar_sliderMoved(int position)
{
    //qDebug() << "bar: " << position;
    timePlot->xAxis->setRange(position, timePlot->xAxis->range().size(), Qt::AlignCenter);
    timePlot->replot();
}

void MainWindow::on_actionDWT_Plot_triggered()
{
    if(wPlot->isHidden()){
        wPlot->show();
        int yIndex = ui->spinBox_y->value();
        wPlot->SetData(file, yIndex);
    }
}

void MainWindow::on_pushButton_DataBase_clicked()
{
    if(dbWindow->isHidden()){
        dbWindow->show();
    }
}

void MainWindow::on_spinBox_x1_B_valueChanged(int arg1)
{
    QVector<double> xData = this->file->GetDataSetX();

    if( arg1 >= xData.length() ) return;
    int arg2 = ui->spinBox_x2_B->value();
    if( arg2 >= xData.length() ) return;

    double xStart = xData[arg1];
    double xEnd = xData[arg2];
    ui->lineEdit_x1_B->setText(QString::number(xStart)+" us");
    ui->spinBox_x2_B->setMinimum(arg1);

    QString yLabel;
    yLabel.sprintf("Integrated Vol. x = (%4.1f, %4.1f) us [a.u.]", xStart, xEnd);
    bFieldPlot->yAxis->setLabel(yLabel);

    PlotBFieldPlot();
}

void MainWindow::on_spinBox_x2_B_valueChanged(int arg1)
{
    QVector<double> xData = this->file->GetDataSetX();

    if( arg1 >= xData.length() ) return;
    int arg2 = ui->spinBox_x1_B->value();
    if( arg2 >= xData.length() ) return;

    double xStart = xData[arg2];
    double xEnd = xData[arg1];
    ui->lineEdit_x2_B->setText(QString::number(xEnd)+" us");
    ui->spinBox_x1_B->setMaximum(arg1);

    QString yLabel;
    yLabel.sprintf("Integrated Vol. x = (%4.1f, %4.1f) us [a.u.]", xStart, xEnd);
    bFieldPlot->yAxis->setLabel(yLabel);

    PlotBFieldPlot();
}

void MainWindow::on_actionSave_BFieldPlot_as_PDF_triggered()
{
    QFileDialog fileDialog(this);
    QStringList nameFilterList = {"pdf (*.pdf)", "PNG (*.png)"};
    fileDialog.setNameFilters(nameFilterList);
    fileDialog.setDirectory(DESKTOP_PATH);
    fileDialog.setReadOnly(0);
    QString fileName;
    if( fileDialog.exec()){
        fileName = fileDialog.selectedFiles()[0];
    }


    bool ok = false;
    int ph = bFieldPlot->geometry().height();
    int pw = bFieldPlot->geometry().width();
    //clear the lines;
    bFieldPlot->graph(1)->clearData();
    bFieldPlot->graph(2)->clearData();

    if( fileDialog.selectedNameFilter() == nameFilterList[0]){
        if( fileName.right(4) != ".pdf" ) fileName.append(".pdf");
        ok = bFieldPlot->savePdf(fileName, pw, ph );
    }else{
        if( fileName.right(4) != ".png" ) fileName.append(".png");
        ok = bFieldPlot->savePng(fileName, pw, ph );
    }

    if( ok ){
        Write2Log("Saved B-field Plot as " + fileName);
    }else{
        Write2Log("Save Failed.");
    }
}

void MainWindow::on_actionHelp_triggered()
{
    if( helpDialog->isHidden() ){
        helpDialog->show();
    }
}

void MainWindow::HelpPicNext()
{
    if( picNumber == 6){
        QImage image(":/HelpPic/MainWindow1.PNG");
        HelpLabel->setPixmap(QPixmap::fromImage(image));
        picNumber = 1;
        return;
    }

    if( picNumber == 1){
        QImage image(":/HelpPic/MainWindow2.PNG");
        HelpLabel->setPixmap(QPixmap::fromImage(image));
        picNumber ++;
        return;
    }

    if( picNumber == 2){
        QImage image(":/HelpPic/MainWindow3.PNG");
        HelpLabel->setPixmap(QPixmap::fromImage(image));
        picNumber ++;
        return;
    }

    if( picNumber == 3){
        QImage image(":/HelpPic/FFTW1.PNG");
        HelpLabel->setPixmap(QPixmap::fromImage(image));
        picNumber ++;
        return;
    }

    if( picNumber == 4){
        QImage image(":/HelpPic/FFTW2.PNG");
        HelpLabel->setPixmap(QPixmap::fromImage(image));
        picNumber ++;
        return;
    }

    if( picNumber == 5){
        QImage image(":/HelpPic/DWT.PNG");
        HelpLabel->setPixmap(QPixmap::fromImage(image));
        picNumber ++;
        return;
    }
}

int MainWindow::loadConfigurationFile()
{
    QString path = DESKTOP_PATH + "/ProgramsConfiguration.ini";
    if( QFile::exists(path) ){
        Write2Log("Configuration file found :" + path);
    }else{
        Write2Log("Configuration not found. | " + path);
        return 1;
    }

    QFile configFile(path);
    configFile.open(QIODevice::ReadOnly);
    if( configFile.isOpen() ){
        Write2Log("Configuration file openned.");
    }else{
        Write2Log("Configuration file fail to open.");
        return 2;
    }

    QTextStream stream(&configFile);
    QString line;
    QStringList lineList;

    int itemCount = 0;

    while(stream.readLineInto(&line) ){
        if( line.left(1) == "#" ) continue;
        lineList = line.split(" ");
        //qDebug() << lineList[0] << ", " << lineList[lineList.size()-1];
        if( lineList[0] == "DATA_PATH") {
            DATA_PATH = lineList[lineList.size()-1];
            itemCount ++;
        }
        if( lineList[0] == "DB_PATH") {
            DB_PATH = lineList[lineList.size()-1];
            itemCount ++;
        }
        if( lineList[0] == "HALL_PATH") {
            HALL_PATH = lineList[lineList.size()-1];
            itemCount ++;
        }
        if( lineList[0] == "LOG_PATH") {
            LOG_PATH = lineList[lineList.size()-1];
            itemCount ++;
        }
        if( lineList[0] == "ChemicalPicture_PATH") {
            CHEMICAL_PIC_PATH = lineList[lineList.size()-1];
            itemCount ++;
        }
        if( lineList[0] == "SamplePicture_PATH") {
            SAMPLE_PIC_PATH = lineList[lineList.size()-1];
            itemCount ++;
        }
    }

    if( itemCount != 6){
        return 3;
    }
    //qDebug() << DATA_PATH;
    //qDebug() << CHEMICAL_PIC_PATH;
    //qDebug() << SAMPLE_PIC_PATH;
    //qDebug() << HALL_PATH;
    //qDebug() << LOG_PATH;
    //qDebug() << DB_PATH;

    configFile.close();

    return 0;
}

void MainWindow::on_actionStatistics_triggered()
{
    double sBGmean = ana->GetSampleMean();
    double sBGvar = ana->GetSampleVariance();

    int xStart = ui->spinBox_x->value();
    int xEnd = ui->spinBox_x2->value();

    QVector<double> mv = ana->MeanAndvariance(xStart, xEnd);

    QVector<double> x = ana->GetData_x();
    QVector<double> y = ana->GetData_y();
    double integrate = 0;
    for( int i = xStart ; i <= xEnd ; i++){
        integrate += y[i];
    }
    double dx = x[xStart+1] - x[xStart];
    integrate = integrate * dx;

    double yMax = ana->GetDataYMax();
    double yMin = ana->GetDataYMin();

    //cal absolute max of BG data
    int xBG = ana->FindXIndex(TIME2);
    double bgMax = 0;
    for(int i = 0; i < xBG; i++){
        if( qAbs(y[i]) > bgMax) bgMax = qAbs(y[i]);
    }

    QMessageBox msgBox;
    QString msg;
    msg.sprintf("Statistics of y-Index : %d", ui->spinBox_y->value());
    msgBox.setWindowTitle(msg);
    msg.sprintf("y-Max     : %7.3f \n"
                "y-Min     : %7.3f \n"
                "===  BG    (%6.2f, %6.2f) us\n"
                "BG mean   : %7.3f \n"
                "BG s.d.   : %7.3f \n"
                "BG |max|  : %7.3f \n"
                "=== Sample (%6.2f, %6.2f) us\n"
                "mean      : %7.3f \n"
                "s.d.      : %7.3f \n"
                "Integrate : %7.3f",
                yMax, yMin,
                x[0], TIME2,
                sBGmean, sqrt(sBGvar), bgMax,
                x[xStart], x[xEnd],
                mv[0], sqrt(mv[1]),
                integrate);

    QFont font("Courier New", 14);
    msgBox.setFont(font);
    msgBox.setText(msg);
    msgBox.exec();

}

void MainWindow::on_actionSave_Message_triggered()
{
    QString filePath = QFileDialog::getSaveFileName(this,
                                                  "Save Message to TXT",
                                                  DESKTOP_PATH,
                                                  "TEXT (*.txt)");

    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly)){
        QMessageBox msgBox;
        msgBox.setText("Fail to open file:\n" + filePath);
        msgBox.exec();
        return;
    }

    QTextStream stream(&file);
    stream << ui->textEdit->toPlainText();

    file.close();

}

void MainWindow::on_actionOpen_in_File_Explorer_triggered()
{
    if( file != NULL ) {
        QDesktopServices::openUrl(file->GetFileDirectory());
    }else{
        QDesktopServices::openUrl( DATA_PATH );
    }
}

void MainWindow::on_actionSave_Time_Plot_w_o_lines_triggered()
{
    QFileDialog fileDialog(this);
    QStringList nameFilterList = {"pdf (*.pdf)", "PNG (*.png)"};
    fileDialog.setNameFilters(nameFilterList);
    fileDialog.setDirectory(DESKTOP_PATH);
    fileDialog.setReadOnly(0);
    QString fileName;
    if( fileDialog.exec()){
        fileName = fileDialog.selectedFiles()[0];
    }

    bool ok = false;
    int ph = timePlot->geometry().height();
    int pw = timePlot->geometry().width();
    timePlot->graph(1)->clearData();
    timePlot->graph(2)->clearData();
    timePlot->graph(3)->clearData();
    if( fileDialog.selectedNameFilter() == nameFilterList[0]){
        if( fileName.right(4) != ".pdf" ) fileName.append(".pdf");
        ok = timePlot->savePdf(fileName, pw, ph );
    }else{
        if( fileName.right(4) != ".png" ) fileName.append(".png");
        ok = timePlot->savePng(fileName, pw, ph );
    }

    if( ok ){
        Write2Log("Saved Time-Plot as " + fileName);
    }else{
        Write2Log("Save Failed.");
    }

}

void MainWindow::on_actionSave_all_Plot_triggered()
{
    // Plot all plots in a Dialog
    QDialog * testDialog = new QDialog(this);
    testDialog->resize(600,400);

    QCustomPlot * allPlots = new QCustomPlot();
    allPlots->plotLayout()->clear();
    allPlots->setInteraction(QCP::iRangeZoom,true);
    allPlots->setInteraction(QCP::iRangeDrag,true);

    QCPAxisRect * bPlot = new QCPAxisRect(allPlots);
    allPlots->plotLayout()->addElement(0,0,bPlot);
    bPlot->setupFullAxesBox(true);
    bPlot->axis(QCPAxis::atBottom)->setRangeReversed(true);
    bPlot->axis(QCPAxis::atLeft)->setTickLabels(false);
    //bPlot->axis(QCPAxis::atRight)->setTickLabels(true);

    QCPAxisRect * cPlot = new QCPAxisRect(allPlots);
    QCPColorMap * cMap = new QCPColorMap(cPlot->axis(QCPAxis::atBottom), cPlot->axis(QCPAxis::atLeft));
    //cPlot->setupFullAxesBox(true);
    allPlots->plotLayout()->addElement(0,1,cPlot);

    qDebug() << cPlot->layer()->children();

    int size = cPlot->layer()->children().size();
    qDebug() << cPlot->layer()->children().at(size-1);
    qDebug() << cPlot->layer()->children().at(size-1)->parentLayerable();

    //allPlots->moveLayer(cPlot->layer()->children().at(1)->layer(), cMap->layer());
    //cPlot->setLayer(cPlot->layer()->children().at(1)->layer());

    //cPlot->axis(QCPAxis::atBottom)->setLabel("time [us]");
    //cPlot->axis(QCPAxis::atLeft)->setLabel("Ctrl. Vol. [V]");

    QCPAxisRect * tPlot = new QCPAxisRect(allPlots);
    allPlots->plotLayout()->addElement(1,1,tPlot);
    tPlot->setupFullAxesBox(true);

    //add Graph
    allPlots->addGraph(bPlot->axis(QCPAxis::atLeft), bPlot->axis(QCPAxis::atBottom));
    allPlots->addGraph(tPlot->axis(QCPAxis::atBottom), tPlot->axis(QCPAxis::atLeft));
    allPlots->addGraph(cPlot->axis(QCPAxis::atBottom), cPlot->axis(QCPAxis::atLeft));


    // set color Scale
    QCPColorScale *colorScale = new QCPColorScale(allPlots);
    allPlots->plotLayout()->addElement(0, 2, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    cMap->setColorScale(colorScale); // associate the color map with the color scale
    QCPColorGradient colorGrad;
    colorGrad.clearColorStops();
    colorGrad.setColorStopAt(0, QColor(0,0,255));
    colorGrad.setColorStopAt(0.5, QColor(255,255,255));
    colorGrad.setColorStopAt(1, QColor(255,0,0));
    colorGrad.setColorInterpolation(QCPColorGradient::ColorInterpolation::ciRGB);
    cMap->setGradient( colorGrad ); //color scheme

    //set margin
    QCPMarginGroup * group = new QCPMarginGroup(allPlots);
    bPlot->setMarginGroup(QCP::msLeft|QCP::msRight, group);
    cPlot->setMarginGroup(QCP::msLeft|QCP::msRight|QCP::msTop|QCP::msBottom, group);
    tPlot->setMarginGroup(QCP::msTop|QCP::msBottom, group);

    //fill data
    allPlots->graph(0)->setData(bFieldPlot->graph(0)->data(), true);
    allPlots->graph(1)->setData(timePlot->graph(0)->data(), true);
    cMap->setData(colorMap->data(),true);

    //rescale axis
    cMap->rescaleDataRange();
    cMap->rescaleAxes();

    allPlots->rescaleAxes();
    allPlots->replot();

    //add to Dialog
    QVBoxLayout * mainLayout = new QVBoxLayout;
    mainLayout->addWidget(allPlots);
    //mainLayout->addWidget(closeButton);
    testDialog->setLayout(mainLayout);
    testDialog->show();

    //show message, ask for save

    // open file Dialog

    // save
}

void MainWindow::on_comboBox_fitFunctionType_currentIndexChanged(int index)
{
    if(ana == NULL) return;
    ana->setFunctionType(index);
    if( index == 0){
        ui->lineEdit_a->setEnabled( true);
        ui->lineEdit_Ta->setEnabled(true);
        ui->lineEdit_b->setEnabled( true);
        ui->lineEdit_Tb->setEnabled(true);
        ui->lineEdit_c->setEnabled( true);

        ui->checkBox_b_Tb->setEnabled(true);
        ui->checkBox_c->setEnabled(true);
        Write2Log("Set fitting function be : a * Exp(-x/Ta) + b * Exp(-x/Tb) + c");
        ui->lineEdit_P->setText("");
        ui->lineEdit_eP->setText("");
        ui->lineEdit_P->setEnabled(true);

        on_pushButton_guessPars_clicked();
    }else if( index == 1){
        ui->lineEdit_a->setEnabled( true);
        ui->lineEdit_Ta->setEnabled(true);
        ui->lineEdit_b->setEnabled( true);
        ui->lineEdit_Tb->setEnabled(true);
        ui->lineEdit_c->setEnabled( true);

        ui->checkBox_b_Tb->setChecked(true);
        ui->checkBox_b_Tb->setEnabled(false);
        ui->checkBox_c->setChecked(true);
        ui->checkBox_c->setEnabled(false);
        fitResultPlot->SetAvalibleData(5);
        Write2Log("Set fitting function be : a * Exp(-x/Ta) * sin(b + 2*pi * x/Tb) + c");
        ui->lineEdit_P->setText("NaN");
        ui->lineEdit_eP->setText("NaN");
        ui->lineEdit_P->setEnabled(false);

        on_pushButton_guessPars_clicked();
    }else if( index == 2 ){
        // see on_comboBox_fitFunctionType_activated();
    }
}

QString MainWindow::replaceMathExpression(QString func)
{
    QString new_func = func;

    new_func.replace("exp", "Math.exp", Qt::CaseSensitive);
    new_func.replace("sin", "Math.sin", Qt::CaseSensitive);
    new_func.replace("cos", "Math.cos", Qt::CaseSensitive);
    new_func.replace("tan", "Math.tan", Qt::CaseSensitive);
    new_func.replace("sqrt", "Math.sqrt", Qt::CaseSensitive);
    new_func.replace("pow", "Math.pow", Qt::CaseSensitive);

    return new_func;
}

void MainWindow::on_comboBox_fitFunctionType_activated(int index)
{
    if(ana == NULL) return;
    if(index == 2){
        ana->setFunctionType(index);

        //disable lineEdit for parameter;
        ui->lineEdit_a->setEnabled(false);
        ui->lineEdit_Ta->setEnabled(false);
        ui->lineEdit_b->setEnabled(false);
        ui->lineEdit_Tb->setEnabled(false);
        ui->lineEdit_c->setEnabled(false);
        ui->checkBox_b_Tb->setEnabled(false);
        ui->checkBox_c->setEnabled(false);

        ui->lineEdit_a->setText("---");
        ui->lineEdit_Ta->setText("---");
        ui->lineEdit_b->setText("---");
        ui->lineEdit_Tb->setText("---");
        ui->lineEdit_c->setText("---");

        //disable some plots;
        ui->actionFit_Result->setEnabled(false);

        // Open file
        QString fileName = QFileDialog::getOpenFileName(this, "Open function definition file");
        QString expression_str = "";
        QStringList funGrad_temp, funGrad_exp_str, funGrad_str;
        int parNumber = 0;
        par.clear();

        Write2Log("set custom fit function: ");

        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            Write2Log(" --- cancelled.");
            ui->comboBox_fitFunctionType->setCurrentIndex(0);
            return;
        }

        QTextStream in(&file);
        while (!in.atEnd()){
            QString line = in.readLine();

            if(line.left(1)== "#") continue;

            // function definition
            if(line.left(8) == "function"){
                int n = line.indexOf("=") + 1;
                expression_str = line.remove(0,n);

                //Checking parameter in function is same as par
                for( int i = 0; i <= 9; i++){
                    QString pX = "p" + QString::number(i);
                    if( expression_str.indexOf(pX) >= 0 ) {
                        parNumber ++;
                    }else{
                        break;
                    }
                }

                continue;
            }

            // parameter
            if(line.left(1) == "p"){
                if( par.size() == parNumber ) continue;
                int n = line.indexOf("=") + 1;
                par.push_back( line.remove(0,n).toDouble());
            }

            // gradiant
            if(line.left(1) == "g"){
                if( funGrad_temp.size() == parNumber ) break;
                int n = line.indexOf("=") + 1;
                funGrad_temp.push_back( line.remove(0,n) );
            }

        }

        file.close();

        Write2Log("  function = " + expression_str);
        fitResultPlot->SetAvalibleData(par.size()); //TODO , already stop the filling of fit result
        //Write2Log("parameter size : " + QString::number(parNumber));
        QString msg = "Initial parameter : (";
        for(int i = 0; i < parNumber - 1; i++){
            msg += QString::number(par[i]);
            msg += ", ";
        }
        msg += QString::number(par[parNumber-1]);
        msg += ")";
        Write2Log(msg);
        Write2Log("Gradient functions :");
        for(int i = 0; i < parNumber ; i++){
            QString gy = "g" + QString::number(i) + " = ";
            Write2Log( gy  + funGrad_temp.at(i));
        }

        //replace math expression
        expression_str = replaceMathExpression(expression_str);
        for(int i = 0; i < parNumber; i++){
            funGrad_exp_str.push_back( replaceMathExpression(funGrad_temp.at(i)));
        }

        qDebug() << expression_str;
        qDebug() << funGrad_exp_str;

        ana->setFunctionExpression((expression_str));
        ana->setFunctionGradExpression(funGrad_exp_str);

        //qDebug() << funGrad_str;

        ana->CalFitData(par);
        PlotTimePlot(1, ana->GetData_x(), ana->GetFitData_y());
        //PlotFitFuncAndXLines();
    }
}
