#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    file(NULL)
{
    ui->setupUi(this);

    plot = ui->customPlot;

    ana = new Analysis();
    connect(ana, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));

}

MainWindow::~MainWindow(){
    delete ui;
    delete plot;
    if( file != NULL) delete file;
    if( ana != NULL) delete ana;
}

void MainWindow::Plot(int graphID, QVector<double> x, QVector<double> y, double xMin, double xMax, double yMin, double yMax){

    while( plot->graphCount() < graphID+1){
        plot->addGraph();
        if( plot->graphCount() == 1){
            plot->xAxis->setLabel("time [us]");
            plot->yAxis->setLabel("a.u.");
        }
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
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this,
                                            "Open File",
                                            "C:/Users/Triplet-ESR/Desktop/nonLinearFit",
                                            tr("Col-wise (*.csv *.txt *.dat);; Row-wise (*txt *dat)"));

    ui->lineEdit->setText(fileName);

    if(fileName == "") return;

    file = new FileIO(fileName);
    connect(file, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));
    if(fileName.right(3)=="csv"){
        file->OpenCSVData();
    }

    if(file->IsOpen() == 0) return;

    Write2Log("Opened file :");
    Write2Log(fileName);

    qDebug("X: (%f %f)", file->GetXMin(), file->GetXMax());
    qDebug("Y: (%f %f)", file->GetYMin(), file->GetYMax());

    file->OpenSaveFile();

    ui->spinBox_y->setMinimum(0);
    ui->spinBox_y->setMaximum(file->GetDataSetSize()-1);
    ui->spinBox_x->setMinimum(0);
    ui->spinBox_x->setMaximum(file->GetDataSize()-1);

    ui->spinBox_y->setValue(104);
    ui->spinBox_x->setValue(195);

    //ana->SetData(file->GetDataSetX(), file->GetDataSetZ(104));
    //ana->SetY(104, file->GetDataY(104));

}


void MainWindow::on_spinBox_y_valueChanged(int arg1){
    Plot(0, file->GetDataSetX(), file->GetDataSetZ(arg1),
         file->GetXMin(), file->GetXMax(),
         file->GetZMin(), file->GetZMax());

    ui->spinBox_y->setValue(arg1);
    ui->lineEdit_y->setText(QString::number(file->GetDataY(arg1)));
    ana->SetData(file->GetDataSetX(), file->GetDataSetZ(arg1));
    ana->SetY(arg1, file->GetDataY(arg1));
    PlotFitFunc();
}

void MainWindow::on_spinBox_x_valueChanged(int arg1){
    ui->lineEdit_x->setText(QString::number(file->GetDataX(arg1)));
    PlotFitFunc();
}

void MainWindow::PlotFitFunc(){

    if( file == NULL) return;

    QVector<double> par;
    par.push_back(ui->lineEdit_a->text().toDouble());
    par.push_back(ui->lineEdit_Ta->text().toDouble());
    par.push_back(ui->lineEdit_b->text().toDouble());
    par.push_back(ui->lineEdit_Tb->text().toDouble());

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
    Plot(2, xline_x, xline_y,
    file->GetXMin(), file->GetXMax(), yMin, yMax);

    ana->SetStartFitIndex(xIndex);
}

void MainWindow::on_lineEdit_a_returnPressed(){
    PlotFitFunc();
}

void MainWindow::on_lineEdit_Ta_returnPressed(){
    PlotFitFunc();
}

void MainWindow::on_lineEdit_b_returnPressed(){
    PlotFitFunc();
}

void MainWindow::on_lineEdit_Tb_returnPressed(){
    PlotFitFunc();
}



void MainWindow::on_pushButton_Fit_clicked(){

    Msg.sprintf("=================================== %d", ana->GetYIndex());
    Write2Log(Msg);

    QVector<double> par;
    par.push_back(ui->lineEdit_a->text().toDouble());
    par.push_back(ui->lineEdit_Ta->text().toDouble());
    par.push_back(ui->lineEdit_b->text().toDouble());
    par.push_back(ui->lineEdit_Tb->text().toDouble());


    double lambda = ui->lineEdit_lambda->text().toDouble();
    int maxIter = ui->lineEdit_MaxIter->text().toInt();
    //ana->Print();
    ana->Setlambda(lambda);
    ana->SetMaxInteration(maxIter);
    ana->MeanAndvariance(0, ana->GetStartFitIndex()-20);
    ana->NonLinearFit(par);

    //display result
    ana->PrintVector(ana->GetParameters(), "sol");
    ana->PrintVector(ana->GetParError(), "error");
    ana->PrintVector(ana->GetParPValue(), "p-Value");
    if( ana->GetFitFlag() == 2) ui->textEdit->setTextColor(QColor(255,0,0,255));
    ana->PrintVector(ana->GetSSRgrad(), "SSR grad");
    if( ana->GetFitFlag() == 2) ui->textEdit->setTextColor(QColor(0,0,0,255));


    Msg.sprintf("DF : %d, SSR: %f, delta: %e", ana->GetNDF(), ana->GetSSR(), ana->GetDelta() );
    Write2Log(Msg);
    double chisq = ana->GetFitVariance()/ana->GetSampleVariance();
    Msg.sprintf("Fit Variance : %f, Sample Variance : %f, Reduced Chi-squared : %f", ana->GetFitVariance(), ana->GetSampleVariance(), chisq);
    Write2Log(Msg);

    // update the parameter
    QVector<double> sol = ana->GetParameters();
    ui->lineEdit_a->setText(QString::number(sol[0]));
    ui->lineEdit_Ta->setText(QString::number(sol[1]));
    ui->lineEdit_b->setText(QString::number(sol[2]));
    ui->lineEdit_Tb->setText(QString::number(sol[3]));

    PlotFitFunc();

    //ana->Print();

}

void MainWindow::on_pushButton_reset_clicked()
{
    int xIndex = ui->spinBox_x->value();
    double xValue = file->GetDataX(xIndex);

    if(xValue > 0) {
        ui->lineEdit_a->setText("20");
        ui->lineEdit_b->setText("-10");
    }else{
        ui->lineEdit_a->setText("-20");
        ui->lineEdit_b->setText("10");
    }
    ui->lineEdit_Ta->setText("20");
    ui->lineEdit_Tb->setText("80");

    PlotFitFunc();
}

void MainWindow::on_pushButton_save_clicked()
{
    file->SaveFitResult(ana);
}

void MainWindow::on_pushButton_FitAll_clicked()
{
    int n = file->GetDataSetSize();
    QProgressDialog progress("Fitting...", "Abort", 0, n, this);
    progress.setWindowModality(Qt::WindowModal);
    QString str;
    int count = 0;

    for( int yIndex = 0; yIndex < n ; yIndex ++){
    //for( int yIndex = 100; yIndex < 300 ; yIndex ++){
        on_pushButton_reset_clicked();
        on_spinBox_y_valueChanged(yIndex);
        on_pushButton_Fit_clicked();
        PlotFitFunc();
        Sleep(500);
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
            file->SaveFitResult(ana);
            count ++;
        }else{
            Write2Log("reduced chi-sq > 2 and p-value(s) > 0.3, not save fitting.");
        }
    }
    progress.setValue(n);
}
