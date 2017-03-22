#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    file(NULL)
{

    ui->setupUi(this);

    dbWindow = new DataBaseWindow();
    connect(dbWindow, SIGNAL(ReturnFilePath(QString)), this, SLOT(OpenFile(QString)));

    fitResultPlot = new FitResult(this);
    connect(fitResultPlot, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));

    bPlot = new BPlot(this);
    connect(bPlot, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));

    fftPlot = new FFTPlot(this);
    connect(fftPlot, SIGNAL(PlotData()), this, SLOT(RePlotPlots()));
    connect(fftPlot, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));

    wPlot = new WaveletPlot(this);
    connect(wPlot, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)) );
    connect(wPlot, SIGNAL(Replot()), this, SLOT(RePlotPlots()));

    savedSingleXCVS = 0;

    plot = ui->customPlot;
    plot->xAxis->setLabel("time [us]");
    plot->yAxis->setLabel("Voltage [a.u.]");
    plot->yAxis2->setVisible(true);
    plot->yAxis2->setTickLabels(false);
    plot->yAxis2->setTicks(true);
    plot->setInteraction(QCP::iRangeDrag,true);
    plot->setInteraction(QCP::iRangeZoom,true);
    //plot->axisRect()->setRangeDrag(Qt::Vertical);
    //plot->axisRect()->setRangeZoom(Qt::Vertical);
    plot->axisRect()->setRangeDrag(Qt::Horizontal);
    plot->axisRect()->setRangeZoom(Qt::Horizontal);
    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));

    plot->axisRect()->setRangeDragAxes(plot->xAxis, plot->yAxis);
    plot->axisRect()->setRangeZoomAxes(plot->xAxis, plot->yAxis);

    //this sync yaxis2 range as yaxis
    connect(plot->yAxis, SIGNAL(rangeChanged(QCPRange)), this , SLOT(ChangeYAxis2Range(QCPRange)));

    //this can change the react axis
    //connect(plot, SIGNAL(axisClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)), this, SLOT(ChangeReactAxis(QCPAxis*)));

    ctplot = ui->customPlot_CT;
    ctplot->axisRect()->setupFullAxesBox(true);
    ctplot->xAxis->setLabel("time [us]");
    ctplot->yAxis->setLabel("Ctrl. Vol. [V]");

    //ctplot->setInteraction(QCP::iRangeZoom,true);
    //ctplot->yAxis2->setVisible(1);
    //ctplot->yAxis2->setLabel("index");

    colorMap = new QCPColorMap(ctplot->xAxis, ctplot->yAxis);
    colorMap->clearData();

    //vertcal and horizontal lines on ctplot;
    ctplot->addGraph();
    ctplot->graph(0)->setPen(QPen(Qt::gray));
    ctplot->graph(0)->clearData();
    ctplot->addGraph();
    ctplot->graph(1)->setPen(QPen(Qt::gray));
    ctplot->graph(1)->clearData();


    ana = new Analysis();
    connect(ana, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));

    setEnabledPlanel(0);

    statusBar()->showMessage("Please open a file.");

    Write2Log("Directory: " + DATA_PATH);

    controlPressed = 0;

}

MainWindow::~MainWindow(){
    delete ui;
    delete fitResultPlot;
    delete bPlot;
    delete fftPlot;
    delete wPlot;

    delete colorMap;
    delete plot;
    delete ctplot;

    if( file != NULL) delete file;
    if( ana != NULL) delete ana;

    delete dbWindow;

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
    int nx = file->GetDataSize();
    int ny = file->GetDataSetSize();
    if( file->HasBackGround() ) ny = ny -1;
    colorMap->data()->setSize(nx, ny);
    //colorMap->setInterpolate(true);
    //colorMap->setTightBoundary(false);
    //colorMap->setAntialiased(true);

    ctplot->xAxis->setRange(xMin, xMax);
    ctplot->yAxis->setRange(yMin, yMax);

    colorMap->data()->setRange(QCPRange(xMin, xMax), QCPRange(yMin, yMax));

    QCPColorScale *colorScale = new QCPColorScale(ctplot);
    //ctplot->plotLayout()->clear();
    ctplot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
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

    QCPMarginGroup *marginGroup = new QCPMarginGroup(ctplot);
    ctplot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    //================= PLot
    plot->xAxis->setRange(xMin, xMax);
    plot->xAxis2->setVisible(true);
    plot->xAxis2->setRange(0, file->GetDataSize());
    double zRange1 = qMax(fabs(zMax), fabs(zMin));
    plot->yAxis->setRange(-zRange1, zRange1);
    //plot->yAxis2->setRange(-zRange1, zRange1);

    int multi = file->GetMultiIndex();
    QString yLabel;
    yLabel.sprintf("Voltage [ 10^%d V]", -1 * multi);
    if( multi == 3) yLabel = "Voltage [mV]";
    if( multi == 6) yLabel = "Voltage [uV]";
    if( multi == 9) yLabel = "Voltage [nV]";
    plot->yAxis->setLabel(yLabel);

    if( file->HasBackGround()){
        ui->spinBox_y->setValue(1);
    }else{
        ui->spinBox_y->setValue(0);
    }

}

void MainWindow::Plot(int graphID, QVector<double> x, QVector<double> y){

    while( plot->graphCount() < graphID+1){
        plot->addGraph();
    }

    switch (graphID) {
    case 0: plot->graph(graphID)->setPen(QPen(Qt::blue)); break;
    case 1: plot->graph(graphID)->setPen(QPen(Qt::red)); break;
    case 2: plot->graph(graphID)->setPen(QPen(Qt::darkGreen)); break;
    case 3: plot->graph(graphID)->setPen(QPen(Qt::darkGreen)); break;
    case 4: plot->graph(graphID)->setPen(QPen(Qt::magenta)); break;
    case 5: plot->graph(graphID)->setPen(QPen(Qt::black)); break;
    }

    plot->graph(graphID)->clearData();

    plot->graph(graphID)->addData(x, y);

    plot->replot();

}


void MainWindow::Write2Log(QString str){
    ui->textEdit->append(str);
    qDebug()<< str;
    ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
}

void MainWindow::RePlotPlots()
{
    on_spinBox_y_valueChanged(ui->spinBox_y->value());
    PlotContour(ui->verticalSlider_zOffset->value());
    bPlot->SetPlotUnit(ui->comboBox_yLabelType->currentIndex());
    bPlot->Plot();
}

void MainWindow::ChangeReactAxis(QCPAxis *axis)
{
    if( axis != plot->yAxis || axis != plot->yAxis2) return;
    plot->axisRect()->setRangeZoomAxes(plot->xAxis, axis);
}

void MainWindow::ChangeYAxis2Range(QCPRange range)
{
    plot->yAxis2->setRange( range);
}

void MainWindow::ShowMousePositionInPlot(QMouseEvent *mouse)
{

    QPoint pt = mouse->pos();
    double x = plot->xAxis->pixelToCoord(pt.rx());
    double y = plot->yAxis->pixelToCoord(pt.ry());
    int xIndex = file->GetXIndex(x);

    QString msg;
    msg.sprintf("(x, y) = (%7.4f, %7.4f), x-index = %4d", x, y, xIndex);
    statusBar()->showMessage(msg);

}

void MainWindow::ShowMousePositionInCTPlot(QMouseEvent *mouse)
{
    QPoint pt = mouse->pos();
    double x = ctplot->xAxis->pixelToCoord(pt.rx());
    double y = ctplot->yAxis->pixelToCoord(pt.ry());

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
    lineX.push_back(ctplot->xAxis->range().lower);
    lineX.push_back(ctplot->xAxis->range().upper);

    ctplot->graph(0)->clearData();
    ctplot->graph(0)->addData(lineX, lineY);

    lineX.clear();
    lineY.clear();
    lineX.push_back(x);
    lineX.push_back(x);
    lineY.push_back(ctplot->yAxis->range().lower);
    lineY.push_back(ctplot->yAxis->range().upper);

    ctplot->graph(1)->clearData();
    ctplot->graph(1)->addData(lineX, lineY);

    ctplot->replot();



}

void MainWindow::SetXIndexByMouseClick(QMouseEvent *mouse)
{
    QPoint pt = mouse->pos();
    double x = plot->xAxis->pixelToCoord(pt.rx());
    int xIndex = file->GetXIndex(x);

    if( controlPressed){
        if( mouse->button() == Qt::LeftButton){
            ui->spinBox_x->setValue(xIndex);
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
    double y = ctplot->yAxis->pixelToCoord(pt.ry());

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
        file->OpenCSVData();
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
    ui->spinBox_y->setMaximum(file->GetDataSetSize()-1);
    ui->spinBox_x->setMinimum(0);
    ui->spinBox_x->setMaximum(file->GetDataSize()-1);
    ui->spinBox_x2->setMinimum(0);
    ui->spinBox_x2->setMaximum(file->GetDataSize()-1);
    ui->spinBox_BGIndex->setMinimum(0);
    ui->spinBox_BGIndex->setMaximum(file->GetDataSetSize()-1);

    plot->disconnect();
    ctplot->disconnect();
    connect(plot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(ShowMousePositionInPlot(QMouseEvent*)));
    connect(plot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(SetXIndexByMouseClick(QMouseEvent*)));
    connect(ctplot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(ShowMousePositionInCTPlot(QMouseEvent*)));
    connect(ctplot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(SetYIndexByMouseClick(QMouseEvent*)));
    //========= Reset Data in fitResultDialog
    fitResultPlot->ClearData();
    fitResultPlot->SetDataSize(file);
    fitResultPlot->SetFilePath(file->GetFilePath());

    //======== Plot B-plot
    bPlot->SetData(file);

    //======== SetData to fftPlot
    fftPlot->SetData(file);

    //=========== Set up Plots
    SetupPlots();

    //========================= Plot
    if( file->HasBackGround()){
        on_checkBox_BGsub_clicked(true);
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


void MainWindow::on_spinBox_y_valueChanged(int arg1){

    statusBar()->showMessage("Changed y-Index.");

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

    Plot(0, file->GetDataSetX(), file->GetDataSetZ(arg1));

    //--- set x scroll bar
    int rxMin = qRound(file->GetXMin());
    int rxMax = qRound(file->GetXMax());
    int delta = rxMax - rxMin;
    ui->horizontalScrollBar->setMinimum(rxMin + delta/2);
    ui->horizontalScrollBar->setMaximum(rxMin - delta/2);
    ui->horizontalScrollBar->setPageStep(delta);
    ui->horizontalScrollBar->setSingleStep(1);

    ana->SetData(file->GetDataSetX(), file->GetDataSetZ(arg1));
    ana->SetY(arg1, file->GetDataY_CV(arg1));
    if( ui->checkBox_AutoFit->isChecked()) {
        on_pushButton_reset_clicked();
        on_pushButton_Fit_clicked();
    }else{
        PlotFitFunc();
    }

    int x1 = ana->FindXIndex(TIME2);
    ana->MeanAndvariance(0, x1);
}

void MainWindow::on_spinBox_x_valueChanged(int arg1){
    statusBar()->showMessage("Changed x-Index Start.");
    ui->lineEdit_x->setText(QString::number(file->GetDataX(arg1))+ " us");
    ana->SetStartFitIndex(arg1);
    PlotFitFunc();
}
void MainWindow::on_spinBox_x2_valueChanged(int arg1)
{
    statusBar()->showMessage("Changed x-Index End.");
    ui->lineEdit_x2->setText(QString::number(file->GetDataX(arg1))+ " us");
    ana->SetEndFitIndex(arg1);
    PlotFitFunc();
}

void MainWindow::PlotFitFunc(){

    if( file == NULL) return;

    QVector<double> par = GetParametersFromLineText();

    ana->CalFitData(par);
    Plot(1, ana->GetData_x(), ana->GetFitData_y());


    //================Draw X-lines
    int xIndex1 = ui->spinBox_x->value();
    double x1 = file->GetDataX(xIndex1);
    int xIndex2 = ui->spinBox_x2->value();
    double x2 = file->GetDataX(xIndex2);
    QVector<double> xline_y, xline_x1, xline_x2;

    xline_y.push_back(plot->yAxis->range().lower * 3);
    xline_y.push_back(plot->yAxis->range().upper * 3);
    xline_x1.push_back(x1);
    xline_x1.push_back(x1);
    xline_x2.push_back(x2);
    xline_x2.push_back(x2);

    Plot(2, xline_x1, xline_y);
    Plot(3, xline_x2, xline_y);

}

void MainWindow::on_lineEdit_a_returnPressed(){
    statusBar()->showMessage("Changed parameter a.");
    PlotFitFunc();
}

void MainWindow::on_lineEdit_Ta_returnPressed(){
    statusBar()->showMessage("Changed parameter Ta.");
    PlotFitFunc();
}

void MainWindow::on_lineEdit_b_returnPressed(){
    statusBar()->showMessage("Changed parameter b.");
    PlotFitFunc();
}

void MainWindow::on_lineEdit_Tb_returnPressed(){
    statusBar()->showMessage("Changed parameter Tb.");
    PlotFitFunc();
}
void MainWindow::on_lineEdit_c_returnPressed()
{
    statusBar()->showMessage("Changed parameter c.");
    PlotFitFunc();
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
    ana->PrintVector(ana->GetParameters(), "sol");
    ana->PrintVector(ana->GetParError(), "error");
    //ana->PrintVector(ana->GetParPValue(), "p-Value");
    QVector<double> gradSSR = ana->GetSSRgrad();

    bool redFlag = 0; // reset redFlag
    for(int i = 0 ; i < gradSSR.size(); i++){
        redFlag |= std::abs(gradSSR[i]) > 0.02;
    }
    if( redFlag) ui->textEdit->setTextColor(QColor(255,0,0,255));
    ana->PrintVector(ana->GetSSRgrad(), "SSR grad");
    if( redFlag) ui->textEdit->setTextColor(QColor(0,0,0,255));

    ana->PrintCoVarMatrix();

    Msg.sprintf("last Lambda : %e", ana->GetLambda());
    Write2Log(Msg);
    Msg.sprintf("DF : %d, SSR: %f, delta: %e", ana->GetNDF(), ana->GetSSR(), ana->GetDelta() );
    Write2Log(Msg);
    double chisq = ana->GetFitVariance()/ana->GetSampleVariance();
    Msg.sprintf("Fit Variance : %f, Sample Variance : %f, Chi-sq/ndf : %f", ana->GetFitVariance(), ana->GetSampleVariance(), chisq);
    Write2Log(Msg);

    if( ana->GetFitFlag() != 0) {
        ui->textEdit->setTextColor(QColor(255,0,0,255));
        Write2Log("!!!!!!!!!!!!!! fit not good.");
        ui->textEdit->setTextColor(QColor(0,0,0,255));
    }

    // update the parameter
    UpdateLineTextParameters(ana->GetParameters(), ana->GetParError());

    PlotFitFunc();

    fitResultPlot->FillData(ana);
    fitResultPlot->PlotData();

}

void MainWindow::on_pushButton_reset_clicked()
{
    if( file == NULL) return;
    statusBar()->showMessage("Reset parameters to default values.");

    ui->lineEdit_Ta->setText("20");
    ui->lineEdit_Tb->setText("80");

    double c = ana->GetSampleMean();
    ui->lineEdit_c->setText(QString::number(c));

    double a = ana->GetDataYMax();
    double b = ana->GetDataYMin();
    a = a-c;
    b = b-c;
    if( fabs(a) < fabs(b)) {
        double tmp = a;
        a = b;
        b = tmp;
    }

    ui->lineEdit_a->setText(QString::number(a));
    ui->lineEdit_b->setText(QString::number(b));

    ui->lineEdit_ea->setText("");
    ui->lineEdit_eTa->setText("");
    ui->lineEdit_eb->setText("");
    ui->lineEdit_eTb->setText("");
    ui->lineEdit_ec->setText("");
    ui->lineEdit_P->setText("");
    ui->lineEdit_eP->setText("");

    PlotFitFunc();
}

void MainWindow::on_pushButton_save_clicked()
{
    if( file == NULL) return;
    statusBar()->showMessage("Save fitted parameters.");
    file->OpenSaveFileforFit();
    file->SaveFitResult(ana);

}

void MainWindow::on_pushButton_FitAll_clicked()
{
    if( file == NULL) return;
    statusBar()->showMessage("Fitting all data.");
    int n = file->GetDataSetSize();
    QProgressDialog progress("Fitting...", "Abort", 0, n-1, this);
    progress.setWindowModality(Qt::WindowModal);
    QString str;
    int count = 0;

    int yStartIndex = 0;
    if( file->HasBackGround()) {
        //on_checkBox_BGsub_clicked(true);
        yStartIndex = 1;
    }

    for( int yIndex = yStartIndex; yIndex < n ; yIndex ++){
    //for( int yIndex = 100; yIndex < 300 ; yIndex ++){
        on_pushButton_reset_clicked();
        on_spinBox_y_valueChanged(yIndex);
        on_pushButton_Fit_clicked();
        PlotFitFunc();
        //Sleep(500);
        str.sprintf("Fitting #%d / %d , saved %d", yIndex + 1, n, count + 1);
        progress.setLabelText(str);
        progress.setValue(yIndex);
        if(progress.wasCanceled()) break;

        //double chisq = ana->GetFitVariance()/ana->GetSampleVariance();

        //bool pcheck = 1;
        //QVector<double> pValue = ana->GetParPValue();
        //for( int p = 0; p < pValue.size(); p++){
        //    pcheck &= std::abs(pValue[p]) < 0.3;
        //}

        file->OpenSaveFileforFit();
        file->SaveFitResult(ana);
        count ++;
        //if( std::abs(chisq-1) < 0.5 && pcheck){
        //}else{
        //    Write2Log("reduced chi-sq > 2 and p-value(s) > 0.3, not save fitting.");
        //}
    }
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
    par.push_back(ui->lineEdit_a->text().toDouble());
    par.push_back(ui->lineEdit_Ta->text().toDouble());
    if( ui->checkBox_b_Tb->isChecked()){
        par.push_back(ui->lineEdit_b->text().toDouble());
        par.push_back(ui->lineEdit_Tb->text().toDouble());
    }
    if( ui->checkBox_c->isChecked()){
        par.push_back(ui->lineEdit_c->text().toDouble());
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

void MainWindow::keyPressEvent(QKeyEvent *key)
{
    if(key->key() == Qt::Key_Shift ){
        //qDebug() << "Shift pressed";
        plot->axisRect()->setRangeDrag(Qt::Vertical);
        plot->axisRect()->setRangeZoom(Qt::Vertical);
    }

    if(key->key() == Qt::Key_Control){
        controlPressed = 1;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *key)
{
    if(key->key() == Qt::Key_Shift ){
        plot->axisRect()->setRangeDrag(Qt::Horizontal);
        plot->axisRect()->setRangeZoom(Qt::Horizontal);
    }
    if(key->key() == Qt::Key_Control){
        //qDebug() << " Ctrl released";
        controlPressed = 0;
    }
}

void MainWindow::PlotContour(double offset)
{
    int yIndexStart = 0;
    if( file->HasBackGround()) yIndexStart = 1;

    int nx = file->GetDataSize();
    int ny = file->GetDataSetSize();

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
            //if( file->IsYRevered()){
            //    colorMap->data()->setCell(xIndex, ny-yIndex-1, z);
            //}else{
            //    colorMap->data()->setCell(xIndex, yIndex, z); // fill data
            //}
        }
    }

    //Clean line, otherwise, the plot range will mess up;
    ctplot->graph(0)->clearData();
    ctplot->graph(1)->clearData();

    ctplot->rescaleAxes();

    ctplot->replot();
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
    ui->pushButton_reset->setEnabled(IO);
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
    ui->actionSave_B_Plot_as_PDF->setEnabled(IO);
    ui->actionSave_Fit_Result_Plot_as_PDF->setEnabled(IO);
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

    RePlotPlots();

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

    RePlotPlots();

}

void MainWindow::on_spinBox_BGIndex_valueChanged()
{
    on_checkBox_BGsub_clicked(1);
}

void MainWindow::on_verticalSlider_z_sliderMoved(int position)
{
    colorMap->setDataRange(QCPRange(-position, position));
    ctplot->replot();
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

    RePlotPlots();
}

void MainWindow::on_verticalSlider_zOffset_sliderMoved(int position)
{
    PlotContour(position);
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

    RePlotPlots();
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
        ctplot->yAxis->setLabel("Ctrl. Vol. [V]");
        ctplot->yAxis->setRange(yMin, yMax);
        unitText = " V";
        ui->lineEdit_y->setText(QString::number(file->GetDataY_CV(yIndex))+ unitText);

    }else if(index == 1){ // Hall Volatage
        ctplot->yAxis->setLabel("Hall Vol. [mV]");
        yMin = file->GetYMin_HV();
        yMax = file->GetYMax_HV();
        ctplot->yAxis->setRange(yMin, yMax);
        unitText = " mV";
        ui->lineEdit_y->setText(QString::number(file->GetDataY_HV(yIndex))+ unitText);

    }else if(index == 2){ // Magnetic field [T]
        ctplot->yAxis->setLabel("B-field [mT]");
        yMin = file->HV2Mag(file->GetYMin_HV());
        yMax = file->HV2Mag(file->GetYMax_HV());
        ctplot->yAxis->setRange(yMin, yMax);
        unitText = " mT";
        ui->lineEdit_y->setText(QString::number(file->HV2Mag(file->GetDataY_HV(yIndex)))+ unitText);

    }

    bPlot->SetPlotUnit(ui->comboBox_yLabelType->currentIndex());
    bPlot->Plot();

    colorMap->data()->setRange(QCPRange(xMin, xMax), QCPRange(yMin, yMax));

    PlotContour(ui->verticalSlider_zOffset->value());
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

    //plot->graph(1)->clearData();
    //plot->graph(2)->clearData();
    //plot->graph(3)->clearData();

    bool ok = plot->savePdf(fileName, pw, ph );

    if( ok ){
        Write2Log("Saved Single-Data Plot as " + fileName);
    }else{
        Write2Log("Save Failed.");
    }
}

void MainWindow::on_actionSave_Contour_Plot_as_PDF_triggered()
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

    int ph = ctplot->geometry().height();
    int pw = ctplot->geometry().width();

    //clear the lines;
    ctplot->graph(0)->clearData();
    ctplot->graph(1)->clearData();

    bool ok = ctplot->savePdf(fileName, pw, ph );

    if( ok ){
        Write2Log("Saved Contour Plot as " + fileName);
    }else{
        Write2Log("Save Failed.");
    }
}

void MainWindow::on_actionSave_B_Plot_as_PDF_triggered()
{
    bPlot->on_pushButton_Print_clicked();
}

void MainWindow::on_actionSave_Fit_Result_Plot_as_PDF_triggered()
{
    fitResultPlot->on_pushButton_SavePlot_clicked();
}

void MainWindow::on_actionSave_data_triggered()
{
    file->SaveTxtData_row();
}

void MainWindow::xAxisChanged(QCPRange range)
{
    if( file == NULL ) return;
    double xMin = file->GetXMin();
    double xMax = file->GetXMax();
    //regulate the xAxis
    if( range.center() + range.size()/2 > xMax){
        plot->xAxis->setRangeUpper(xMax);
        range = plot->xAxis->range();
    }

    if( range.center() - range.size()/2 < xMin){
        plot->xAxis->setRangeLower(xMin);
        range = plot->xAxis->range();
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
}

void MainWindow::on_horizontalScrollBar_sliderMoved(int position)
{
    //qDebug() << "bar: " << position;
    plot->xAxis->setRange(position, plot->xAxis->range().size(), Qt::AlignCenter);
    plot->replot();
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
