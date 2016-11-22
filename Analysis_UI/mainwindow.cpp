#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    file(NULL)
{
    ui->setupUi(this);

    fitResultPlot = new FitResult(this);
    connect(fitResultPlot, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));

    bPlot = new BPlot(this);
    connect(bPlot, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));

    fftPlot = new FFTPlot(this);

    savedSimplifiedtxt = 0;

    plot = ui->customPlot;
    plot->xAxis->setLabel("time [us]");
    plot->yAxis->setLabel("Voltage [a.u.]");
    plot->setInteraction(QCP::iRangeDrag,true);
    plot->setInteraction(QCP::iRangeZoom,true);
    plot->axisRect()->setRangeDrag(Qt::Vertical);
    plot->axisRect()->setRangeZoom(Qt::Vertical);

    ctplot = ui->customPlot_CT;
    ctplot->axisRect()->setupFullAxesBox(true);
    ctplot->xAxis->setLabel("time [us]");
    ctplot->yAxis->setLabel("B-field [mV]");

    colorMap = new QCPColorMap(ctplot->xAxis, ctplot->yAxis);
    colorMap->clearData();

    ana = new Analysis();
    connect(ana, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));

    setDisabledPlanel();

    statusBar()->showMessage("Please open a file.");

}

MainWindow::~MainWindow(){
    delete ui;
    delete fitResultPlot;
    delete bPlot;
    delete fftPlot;

    delete colorMap;
    delete plot;
    delete ctplot;

    if( file != NULL) delete file;
    if( ana != NULL) delete ana;

}

void MainWindow::Plot(int graphID, QVector<double> x, QVector<double> y){

    while( plot->graphCount() < graphID+1){
        plot->addGraph();
    }

    switch (graphID) {
    case 0: plot->graph(graphID)->setPen(QPen(Qt::blue)); break;
    case 1: plot->graph(graphID)->setPen(QPen(Qt::red)); break;
    case 2: plot->graph(graphID)->setPen(QPen(Qt::darkGreen)); break;
    case 3: plot->graph(graphID)->setPen(QPen(Qt::magenta)); break;
    case 4: plot->graph(graphID)->setPen(QPen(Qt::black)); break;
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

void MainWindow::on_pushButton_OpenFile_clicked(){

    QFileDialog fileDialog(this);
    QStringList filters;
    //======== set allowed data structure.
    filters << "Row-wise (*txt *dat *.*)" << "Double-X CSV(*.csv)" << "Col-wise (*.txt *.dat *.csv *.*)" ;
    fileDialog.setNameFilters(filters);
    fileDialog.setReadOnly(1);
    fileDialog.setDirectory(OPENPATH);
    QString fileName;
    //======== Open read the first file
    if( fileDialog.exec()) {
        QStringList fileNames = fileDialog.selectedFiles();
        fileName = fileNames[0];
    }

    //======== If no file selected.
    if(fileName == "") {
        statusBar()->showMessage("No file was selected.");
        return;
    }

    //======== show file name
    statusBar()->showMessage("Selected a file.");
    ui->lineEdit->setText(fileName);

    //======== if open another filed, i.e. exist file, delete it.
    if( file != NULL ) delete file;
    //======== new FileIO
    file = new FileIO(fileName);
    connect(file, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));

    //======== read data structure according to the seleteced data format.
    if( fileDialog.selectedNameFilter() == filters[1]){
        file->OpenCSVData();
    }else if(fileDialog.selectedNameFilter() == filters[2]){
        file->OpenTxtData_col();
    }else if(fileDialog.selectedNameFilter() == filters[0]){
        file->OpenTxtData_row();
    }
    //======== if some shit happen, abort.
    if(file->IsOpen() == 0) {
        Write2Log("Cannot open file.");
        return;
    }
    QString msg = "Opened file : ";
    msg.append(fileName);
    Write2Log(msg);

    //========= once the file is opened, eanble planel, set checkBoxes to uncheck, etc...
    setEnabledPlanel();
    ui->checkBox_BGsub->setChecked(0);
    ui->checkBox_MeanCorr->setChecked(0);
    ui->verticalSlider_zOffset->setValue(0);
    ui->spinBox_y->setMinimum(0);
    ui->spinBox_y->setMaximum(file->GetDataSetSize()-1);
    ui->spinBox_x->setMinimum(0);
    ui->spinBox_x->setMaximum(file->GetDataSize()-1);
    ui->spinBox_BGIndex->setMinimum(0);
    ui->spinBox_BGIndex->setMaximum(file->GetDataSetSize()-1);

    //========= rename y-title
    QString yLabel;
    yLabel.sprintf("Voltage [ 10^%d V]", -1 * file->GetMultiIndex());
    plot->yAxis->setLabel(yLabel);

    //========= Plot single data
    double xMin = file->GetXMin();
    double xMax = file->GetXMax();
    double yMin = file->GetYMin();
    double yMax = file->GetYMax();
    plot->xAxis->setRange(xMin, xMax);
    double yRange = qMax(fabs(yMax), fabs(yMin));
    plot->yAxis->setRange(-yRange, yRange);

    //========= if file has background data
    //========= the plotting function is called using spinBox_y
    if( file->HasBackGround()){
        on_spinBox_y_valueChanged(1);
        //on_checkBox_BGsub_clicked(1);
    }else{
        on_spinBox_y_valueChanged(0);
    }

    //========= find x-index of TIME1, in constant.h
    int xIndex = ana->FindstartIndex(TIME1);
    ui->spinBox_x->setValue(xIndex);

    //========= Plot contour
    // sey Plot Contour z-range
    double zMin = file->GetZMin();
    double zMax = file->GetZMax();
    ui->verticalSlider_zOffset->setMinimum(qFloor(zMin));
    ui->verticalSlider_zOffset->setMaximum(qCeil(zMax));
    ui->verticalSlider_zOffset->setValue(0);

    int nx = file->GetDataSize();
    int ny = file->GetDataSetSize();
    if( file->HasBackGround() ) ny = ny -1;
    colorMap->data()->setSize(nx, ny);

    ctplot->xAxis->setRange(xMin, xMax);
    ctplot->yAxis->setRange(yMin, yMax);

    colorMap->data()->setRange(QCPRange(xMin, xMax), QCPRange(yMin, yMax));

    PlotContour(0);

    QCPColorScale *colorScale = new QCPColorScale(ctplot);
    //ctplot->plotLayout()->removeAt(1); // remove any element if there is any
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
    int zRange = qCeil(qMax(fabs(file->GetZMin()), fabs(file->GetZMax())));
    colorMap->setDataRange(QCPRange(-zRange, zRange));
    ui->verticalSlider_z->setMinimum(1);
    ui->verticalSlider_z->setMaximum(zRange);
    ui->verticalSlider_z->setSingleStep(qCeil(zRange/100.));
    ui->verticalSlider_z->setValue(zRange);

    QCPMarginGroup *marginGroup = new QCPMarginGroup(ctplot);
    ctplot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    ctplot->rescaleAxes();
    ctplot->replot();

    //========= Reset Data in fitResultDialog
    fitResultPlot->ClearData();
    fitResultPlot->SetDataSize(file);
    fitResultPlot->SetFilePath(file->GetFilePath());

    //======== Plot B-plot
    bPlot->SetData(file);

}


void MainWindow::on_spinBox_y_valueChanged(int arg1){

    statusBar()->showMessage("Changed y-Index.");

    ui->spinBox_y->setValue(arg1);
    ui->lineEdit_y->setText(QString::number(file->GetDataY(arg1))+" mV");

    QString title;
    title.sprintf("Hall Voltage : %f mV", file->GetDataY(arg1));
    //plotTitle->setText(title);

    Plot(0, file->GetDataSetX(), file->GetDataSetZ(arg1));

    ana->SetData(file->GetDataSetX(), file->GetDataSetZ(arg1));
    ana->SetY(arg1, file->GetDataY(arg1));
    if( ui->checkBox_AutoFit->isChecked()) {
        on_pushButton_reset_clicked();
        on_pushButton_Fit_clicked();
    }else{
        PlotFitFunc();
    }

    int x1 = ana->FindstartIndex(TIME2);
    ana->MeanAndvariance(0, x1);
}

void MainWindow::on_spinBox_x_valueChanged(int arg1){
    statusBar()->showMessage("Changed x-Index.");
    ui->lineEdit_x->setText(QString::number(file->GetDataX(arg1))+ " us");
    ana->SetStartFitIndex(arg1);
    PlotFitFunc();
}

void MainWindow::PlotFitFunc(){

    if( file == NULL) return;

    QVector<double> par = GetParametersFromLineText();

    ana->CalFitData(par);
    Plot(1, ana->GetData_x(), ana->GetFitData_y());

    int xIndex = ui->spinBox_x->value();
    double x = file->GetDataX(xIndex);

    //Draw X-line
    QVector<double> xline_y, xline_x;
    double yMin = file->GetZMin();
    double yMax = file->GetZMax();

    double yRange = qMax(fabs(yMax), fabs(yMin));

    //double yMean = (yMax + yMin)/2;
    //double yWidth = (yMax - yMin)/2;
    //
    //yMin = yMean - yWidth*1.2;
    //yMax = yMean + yWidth*1.2;

    int size = ana->GetDataSize();
    for(int i = 0; i < size; i++){
        //double y = yMin + (yMax-yMin)*i/size;
        double y = -yRange + 2*yRange*i/size;
        xline_y.push_back(y);
        xline_x.push_back(x);
    }
    Plot(2, xline_x, xline_y);

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

    bool gnu = ui->checkBox->isChecked();

    if( savedSimplifiedtxt == 0 && gnu){
        file->SaveSimplifiedTxt();
        savedSimplifiedtxt = 1;
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
    bool redFlag = 0;
    for(int i = 0 ; i < gradSSR.size(); i++){
        redFlag |= std::abs(gradSSR[i]) > 0.02;
    }
    if( redFlag) ui->textEdit->setTextColor(QColor(255,0,0,255));
    ana->PrintVector(ana->GetSSRgrad(), "SSR grad");
    if( redFlag) ui->textEdit->setTextColor(QColor(0,0,0,255));


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
    file->OpenSaveFile();
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

        double chisq = ana->GetFitVariance()/ana->GetSampleVariance();

        bool pcheck = 1;
        //QVector<double> pValue = ana->GetParPValue();
        //for( int p = 0; p < pValue.size(); p++){
        //    pcheck &= std::abs(pValue[p]) < 0.3;
        //}

        if( std::abs(chisq-1) < 0.5 && pcheck){
            file->OpenSaveFile();
            file->SaveFitResult(ana);
            count ++;
        }else{
            Write2Log("reduced chi-sq > 2 and p-value(s) > 0.3, not save fitting.");
        }
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

void MainWindow::PlotContour(double offset)
{
    int yIndexStart = 0;
    if( file->HasBackGround()) yIndexStart = 1;

    int nx = file->GetDataSize();
    int ny = file->GetDataSetSize();

    for(int xIndex = 0; xIndex < nx; xIndex++){
        for(int yIndex = yIndexStart; yIndex < ny; yIndex++){
            double z = file->GetDataZ(xIndex, yIndex) + offset;
            if( file->IsYRevered()){
                colorMap->data()->setCell(xIndex, ny-yIndex-1, z);
            }else{
                colorMap->data()->setCell(xIndex, yIndex, z); // fill data
            }
        }
    }

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
        bPlot->Plot();
    }
}

void MainWindow::setDisabledPlanel()
{
    ui->actionB_Plot->setEnabled(0);
    ui->actionFFTW_Plot->setEnabled(0);
    ui->pushButton_Fit->setEnabled(0);
    ui->pushButton_FitAll->setEnabled(0);
    ui->pushButton_reset->setEnabled(0);
    ui->pushButton_save->setEnabled(0);
    ui->pushButton_RestoreData->setEnabled(0);
    ui->spinBox_y->setEnabled(0);
    ui->spinBox_x->setEnabled(0);
    ui->checkBox_MeanCorr->setEnabled(0);
    ui->verticalSlider_zOffset->setEnabled(0);
    ui->verticalSlider_z->setEnabled(0);
    ui->checkBox_BGsub->setEnabled(0);
    ui->spinBox_BGIndex->setEnabled(0);
    ui->spinBox_MovingAvg->setEnabled(0);
}

void MainWindow::setEnabledPlanel()
{
    ui->actionB_Plot->setEnabled(1);
    ui->actionFFTW_Plot->setEnabled(1);
    ui->pushButton_Fit->setEnabled(1);
    ui->pushButton_FitAll->setEnabled(1);
    ui->pushButton_reset->setEnabled(1);
    ui->pushButton_save->setEnabled(1);
    ui->pushButton_RestoreData->setEnabled(1);
    ui->spinBox_y->setEnabled(1);
    ui->spinBox_x->setEnabled(1);
    ui->checkBox_MeanCorr->setEnabled(1);
    ui->verticalSlider_zOffset->setEnabled(1);
    ui->verticalSlider_z->setEnabled(1);
    ui->checkBox_BGsub->setEnabled(1);
    ui->spinBox_MovingAvg->setEnabled(1);
}

void MainWindow::on_checkBox_MeanCorr_clicked(bool checked)
{
    if( checked){
        file->MeanCorrection();
        on_spinBox_y_valueChanged(ui->spinBox_y->value());
        PlotContour(ui->verticalSlider_zOffset->value());
        bPlot->Plot();
    }else{
        on_pushButton_RestoreData_clicked();
    }

}

void MainWindow::on_checkBox_BGsub_clicked(bool checked)
{
    ui->spinBox_BGIndex->setEnabled(checked);
    int bgYIndex = ui->spinBox_BGIndex->value();

    if( checked ){
        file->SubstractData(bgYIndex);
        on_spinBox_y_valueChanged(ui->spinBox_y->value());
        PlotContour(ui->verticalSlider_zOffset->value());
        bPlot->Plot();
    }else{
        on_pushButton_RestoreData_clicked();
    }

}

void MainWindow::on_spinBox_BGIndex_valueChanged(int arg1)
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
        file->FouierForward();
        file->FouierBackward();
        //fftPlot->ContourPlot(file->GetDataSize(), file->GetDataSetSize(), file->GetFFTDataA(), file->GetFFTDataP());
        fftPlot->ContourPlot(file->GetDataSize(), file->GetDataSetSize(), file->GetFFTDataA(), file->GetData());
    }
}

void MainWindow::on_pushButton_RestoreData_clicked()
{
    file->RestoreData();
    ui->checkBox_BGsub->setChecked(0);
    ui->checkBox_MeanCorr->setChecked(0);
    ui->verticalSlider_zOffset->setValue(0);
    ui->spinBox_MovingAvg->setValue(-1);

    on_spinBox_y_valueChanged(ui->spinBox_y->value());
    PlotContour(ui->verticalSlider_zOffset->value());
    bPlot->Plot();
}

void MainWindow::on_verticalSlider_zOffset_sliderMoved(int position)
{
    PlotContour(position);
}

void MainWindow::on_spinBox_MovingAvg_valueChanged(int arg1)
{
    if( arg1 == -1) {
        on_pushButton_RestoreData_clicked();
        return;
    }


    if( ui->checkBox_MeanCorr->isChecked()== 0){
        ui->checkBox_MeanCorr->setChecked(1);
        file->MeanCorrection();
    }

    file->MovingAvg(arg1);

    PlotContour(ui->verticalSlider_zOffset->value());
    on_spinBox_y_valueChanged(ui->spinBox_y->value());
    bPlot->Plot();
}
