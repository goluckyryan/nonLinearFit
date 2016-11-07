#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    file(NULL)
{
    ui->setupUi(this);
    fitResultDialog = new Dialog(this);
    connect(fitResultDialog, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));

    savedSimplifiedtxt = 0;

    plot = ui->customPlot;
    plot->xAxis->setLabel("time [us]");
    plot->yAxis->setLabel("Voltage [V]");
    plot->setInteraction(QCP::iRangeDrag,true);
    plot->setInteraction(QCP::iRangeZoom,true);
    plot->plotLayout()->insertRow(0);
    plotTitle = new QCPPlotTitle(plot, "title");
    plotTitle->setFont(QFont("sans", 12, QFont::Bold));
    plot->plotLayout()->addElement(0,0, plotTitle);

    ctplot = ui->customPlot_CT;

    ana = new Analysis();
    connect(ana, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));

    ui->pushButton_Fit->setEnabled(0);
    ui->pushButton_reset->setEnabled(0);
    ui->pushButton_save->setEnabled(0);
    ui->pushButton_FitAll->setEnabled(0);

    statusBar()->showMessage("Please open a file.");

}

MainWindow::~MainWindow(){
    delete ui;
    delete fitResultDialog;

    delete plotTitle;
    delete plot;
    //delete ctplot;
    //delete colorMap;

    if( file != NULL) delete file;
    if( ana != NULL) delete ana;

}

void MainWindow::Plot(int graphID, QVector<double> x, QVector<double> y, double xMin, double xMax, double yMin, double yMax){

    while( plot->graphCount() < graphID+1){
        plot->addGraph();
    }

    switch (graphID) {
    case 0: plot->graph(graphID)->setPen(QPen(Qt::blue)); break;
    case 1: plot->graph(graphID)->setPen(QPen(Qt::red)); break;
    case 2: plot->graph(graphID)->setPen(QPen(Qt::green)); break;
    case 3: plot->graph(graphID)->setPen(QPen(Qt::magenta)); break;
    case 4: plot->graph(graphID)->setPen(QPen(Qt::black)); break;
    }

    plot->graph(graphID)->clearData();

    plot->graph(graphID)->addData(x, y);
    plot->xAxis->setRange(xMin, xMax);
    plot->yAxis->setRange(yMin, yMax);
    plot->replot();

}


void MainWindow::Write2Log(QString str){
    ui->textEdit->append(str);
    qDebug()<< str;
    ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
}

void MainWindow::on_pushButton_clicked(){

    QFileDialog fileDialog(this);
    QStringList filters;
    filters << "Row-wise (*txt *dat *.*)" << "Double-X CSV(*.csv)" << "Col-wise (*.txt *.dat *.csv *.*)" ;
    fileDialog.setNameFilters(filters);
    fileDialog.setReadOnly(1);
    fileDialog.setDirectory(OPENPATH);
    QStringList fileNames;
    QString fileName;
    if( fileDialog.exec()) {
        fileNames = fileDialog.selectedFiles();
        fileName = fileNames[0];
    }

    if(fileName == "") {
        statusBar()->showMessage("No file was selected.");
        return;
    }

    statusBar()->showMessage("Opened an file.");
    ui->lineEdit->setText(fileName);

    file = new FileIO(fileName);
    connect(file, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));
    if( fileDialog.selectedNameFilter() == filters[1]){
        file->OpenCSVData();
    }else if(fileDialog.selectedNameFilter() == filters[2]){
        file->OpenTxtData_col();
    }else if(fileDialog.selectedNameFilter() == filters[0]){
        file->OpenTxtData_row();
    }

    if(file->IsOpen() == 0) {
        Write2Log("Cannot open file.");
        return;
    }
    ui->pushButton_Fit->setEnabled(1);
    ui->pushButton_reset->setEnabled(1);
    ui->pushButton_save->setEnabled(1);
    ui->pushButton_FitAll->setEnabled(1);

    Write2Log("Opened file :");
    Write2Log(fileName);

    ui->spinBox_y->setMinimum(0);
    ui->spinBox_y->setMaximum(file->GetDataSetSize()-1);
    ui->spinBox_x->setMinimum(0);
    ui->spinBox_x->setMaximum(file->GetDataSize()-1);

    on_spinBox_y_valueChanged(0);
    int xIndex = ana->FindstartIndex(TIME1);
    ui->spinBox_x->setValue(xIndex);

    PlotContour();

    //Reset Data in fitResultDialog
    fitResultDialog->ClearData();
    fitResultDialog->SetDataSize(file);
    fitResultDialog->SetFilePath(file->GetFilePath());

    double zMin = file->GetZMin();
    double zMax = file->GetZMax();
    ui->doubleSpinBox_zOffset->setMinimum(-zMax+zMin);
    ui->doubleSpinBox_zOffset->setMaximum(zMax-zMin);
    ui->doubleSpinBox_zOffset->setSingleStep((zMax-zMin)/200.);
    ui->doubleSpinBox_zOffset->setValue(0);
}


void MainWindow::on_spinBox_y_valueChanged(int arg1){

    statusBar()->showMessage("Changed y-Index.");

    ui->spinBox_y->setValue(arg1);
    ui->lineEdit_y->setText(QString::number(file->GetDataY(arg1)));

    QString title;
    title.sprintf("Hall Voltage : %f mV", file->GetDataY(arg1));
    plotTitle->setText(title);

    Plot(0, file->GetDataSetX(), file->GetDataSetZ(arg1),
         file->GetXMin(), file->GetXMax(),
         file->GetZMin(), file->GetZMax());

    ana->SetData(file->GetDataSetX(), file->GetDataSetZ(arg1));
    ana->SetY(arg1, file->GetDataY(arg1));
    if( ui->checkBox_AutoFit->isChecked()) {
        on_pushButton_reset_clicked();
        on_pushButton_Fit_clicked();
    }else{
        PlotFitFunc();
    }
}

void MainWindow::on_spinBox_x_valueChanged(int arg1){
    statusBar()->showMessage("Changed x-Index.");
    ui->lineEdit_x->setText(QString::number(file->GetDataX(arg1)));
    ana->SetStartFitIndex(arg1);
    PlotFitFunc();
}

void MainWindow::PlotFitFunc(){

    if( file == NULL) return;

    QVector<double> par = GetParametersFromLineText();

    ana->CalFitData(par);
    Plot(1, ana->GetData_x(), ana->GetFitData_y(),
         file->GetXMin(), file->GetXMax(),
         file->GetZMin(), file->GetZMax());

    int xIndex = ui->spinBox_x->value();
    double x = file->GetDataX(xIndex);

    QVector<double> xline_y, xline_x;
    double yMin = file->GetZMin();
    double yMax = file->GetZMax();
    int size = ana->GetDataSize();
    for(int i = 0; i < size; i++){
        double y = yMin + (yMax-yMin)*i/size;
        xline_y.push_back(y);
        xline_x.push_back(x);
    }
    Plot(2, xline_x, xline_y, file->GetXMin(), file->GetXMax(), yMin, yMax);

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
    int x1 = ana->FindstartIndex(TIME2);
    ana->MeanAndvariance(0, x1);
    ana->NonLinearFit(par, gnu);

    //display result
    ana->PrintVector(ana->GetParameters(), "sol");
    ana->PrintVector(ana->GetParError(), "error");
    ana->PrintVector(ana->GetParPValue(), "p-Value");
    QVector<double> gradSSR = ana->GetSSRgrad();
    bool redFlag = 0;
    for(int i = 0 ; i < gradSSR.size(); i++){
        redFlag |= std::abs(gradSSR[i]) > 0.2;
    }

    //if( ana->GetFitFlag() == 2) ui->textEdit->setTextColor(QColor(255,0,0,255));
    if( redFlag) ui->textEdit->setTextColor(QColor(255,0,0,255));
    ana->PrintVector(ana->GetSSRgrad(), "SSR grad");
    //if( ana->GetFitFlag() == 2) ui->textEdit->setTextColor(QColor(0,0,0,255));
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

    fitResultDialog->FillData(ana);
    fitResultDialog->PlotData();

}

void MainWindow::on_pushButton_reset_clicked()
{
    if( file == NULL) return;
    statusBar()->showMessage("Reset parameters to default values.");
    int xIndex = ui->spinBox_x->value();
    int yIndex = ui->spinBox_y->value();
    double zValue = file->GetDataZ(xIndex, yIndex);

    ui->lineEdit_a->setText(QString::number(zValue));
    ui->lineEdit_Ta->setText("20");
    if(zValue > 0) {
        ui->lineEdit_b->setText("-10");
    }else{
        ui->lineEdit_b->setText("10");
    }
    ui->lineEdit_Tb->setText("80");
    ui->lineEdit_c->setText("0");

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

    for( int yIndex = 0; yIndex < n ; yIndex ++){
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

    //fitResultDialog->ClearData();
    //fitResultDialog->SetDataSize(file->GetDataSetSize());

    if( checked ){
        statusBar()->showMessage("Enable b and Tb.");
        if(ui->checkBox_c->isChecked() ){
            fitResultDialog->SetAvalibleData(5);
        }else{
            fitResultDialog->SetAvalibleData(4);
        }
    }else{
        statusBar()->showMessage("Disable b and Tb.");
        if(ui->checkBox_c->isChecked() ){
            fitResultDialog->SetAvalibleData(3);
        }else{
            fitResultDialog->SetAvalibleData(2);
        }
    }

}

void MainWindow::on_checkBox_c_clicked(bool checked)
{
    ui->lineEdit_c->setEnabled(checked);

    //fitResultDialog->ClearData();
    //fitResultDialog->SetDataSize(file->GetDataSetSize());

    if( checked ){
        statusBar()->showMessage("Enable c.");
        if(ui->checkBox_b_Tb->isChecked() ){
            fitResultDialog->SetAvalibleData(5);
        }else{
            fitResultDialog->SetAvalibleData(3);
        }
    }else{
        statusBar()->showMessage("Disable c.");
        if(ui->checkBox_b_Tb->isChecked() ){
            fitResultDialog->SetAvalibleData(4);
        }else{
            fitResultDialog->SetAvalibleData(2);
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
    double P, eP;
    ui->lineEdit_a ->setText(QString::number(par[0]));
    ui->lineEdit_Ta->setText(QString::number(par[1]));
    if( par.size() == 3){
        ui->lineEdit_c->setText(QString::number(par[2]));
        ui->lineEdit_b ->setText("");
        ui->lineEdit_Tb->setText("");
        ui->lineEdit_P->setText("");
    }
    if( par.size() == 4 ){
        P = (par[0]-par[2])/(par[0]+par[2]);
        ui->lineEdit_P->setText(QString::number(P));
        ui->lineEdit_b ->setText(QString::number(par[2]));
        ui->lineEdit_Tb->setText(QString::number(par[3]));
        ui->lineEdit_c->setText("");
    }
    if( par.size()== 5){
        P = (par[0]-par[2])/(par[0]+par[2]);
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
        eP = 1/pow(par[0]+par[2],2)*sqrt(pow(epar[0],2)+ pow(epar[2],2)) * sqrt(2*(pow(par[0],2)+pow(par[2],2)));
        ui->lineEdit_eP->setText(QString::number(eP));
        ui->lineEdit_eb ->setText(QString::number(epar[2]));
        ui->lineEdit_eTb->setText(QString::number(epar[3]));
        ui->lineEdit_ec->setText("");
    }
    if( par.size()== 5){
        eP = 1/pow(par[0]+par[2],2)*sqrt(pow(epar[0],2)+ pow(epar[2],2)) * sqrt(2*(pow(par[0],2)+pow(par[2],2)));
        ui->lineEdit_eP->setText(QString::number(eP));
        ui->lineEdit_eb ->setText(QString::number(epar[2]));
        ui->lineEdit_eTb->setText(QString::number(epar[3]));
        ui->lineEdit_ec->setText(QString::number(epar[4]));
    }

}

void MainWindow::PlotContour()
{
    ctplot->axisRect()->setupFullAxesBox(true);
    ctplot->xAxis->setLabel("time [us]");
    ctplot->yAxis->setLabel("y-Value");

    colorMap = new QCPColorMap(ctplot->xAxis, ctplot->yAxis);
    int nx = file->GetDataSize();
    int ny = file->GetDataSetSize();
    colorMap->data()->setSize(nx, ny);

    double xMin = file->GetXMin();
    double xMax = file->GetXMax();
    double yMin = file->GetYMin();
    double yMax = file->GetYMax();
    ctplot->xAxis->setRange(xMin, xMax);
    ctplot->yAxis->setRange(yMin, yMax);
    colorMap->data()->setRange(QCPRange(xMin, xMax), QCPRange(yMin, yMax));

    for(int xIndex = 0; xIndex < nx; xIndex++){
        for(int yIndex = 0; yIndex < ny; yIndex++){
            double z = file->GetDataZ(xIndex, yIndex);
            colorMap->data()->setCell(xIndex, yIndex, z); // fill data
        }
    }

    QCPColorScale *colorScale = new QCPColorScale(ctplot);
    ctplot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap->setColorScale(colorScale); // associate the color map with the color scale

    colorMap->setGradient(QCPColorGradient::gpJet ); //color scheme

    colorMap->rescaleDataRange();

    QCPMarginGroup *marginGroup = new QCPMarginGroup(ctplot);
    ctplot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    ctplot->rescaleAxes();

    ctplot->replot();
}

void MainWindow::on_actionFit_Result_triggered()
{
    if(fitResultDialog->isHidden()){
        fitResultDialog->show();
        fitResultDialog->PlotData();
    }
}

void MainWindow::on_doubleSpinBox_zOffset_valueChanged(double arg1)
{
    int nx = file->GetDataSize();
    int ny = file->GetDataSetSize();

    //double zMax, zMin;
    for(int xIndex = 0; xIndex < nx; xIndex++){
        for(int yIndex = 0; yIndex < ny; yIndex++){
            double z = file->GetDataZ(xIndex, yIndex)+ arg1;
            colorMap->data()->setCell(xIndex, yIndex, z); // fill data
            //if(xIndex == 0 && yIndex == 0){
            //    zMax = z;
            //    zMin = z;
            //}
            //if( zMax < z) zMax = z;
            //if( zMin > z) zMin = z;
        }
    }

    //colorMap->setDataRange(QCPRange(zMin, zMax));

    ctplot->replot();
}
