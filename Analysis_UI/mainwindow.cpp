#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    file(NULL),
    ana(NULL)
{
    ui->setupUi(this);

    plot = ui->customPlot;

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
    ui->plainTextEdit->appendPlainText(str);
    qDebug()<< str;
}

void MainWindow::on_pushButton_clicked(){
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this,
                                            "Open File",
                                            "/Users/mobileryan/Triplet-ESR",
                                            tr("Col-wise (*.csv *.txt *.dat);; Row-wise (*txt *dat)"));

    ui->lineEdit->setText(fileName);

    if(fileName == "") return;

    file = new FileIO(fileName);
    connect(file, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));
    if(fileName.right(3)=="csv"){
        file->OpenCSVData();
    }

    if(!file->IsOpen()) return;

    qDebug("X: (%f %f)", file->GetXMin(), file->GetXMax());
    qDebug("Y: (%f %f)", file->GetYMin(), file->GetYMax());

    ui->spinBox_y->setMinimum(0);
    ui->spinBox_y->setMaximum(file->GetDataSetSize()-1);
    ui->spinBox_x->setMinimum(0);
    ui->spinBox_x->setMaximum(file->GetDataSize()-1);

    ui->spinBox_y->setValue(104);
    ui->spinBox_x->setValue(195);

}


void MainWindow::on_spinBox_y_valueChanged(int arg1){
    Plot(0, file->GetDataSetX(), file->GetDataSetZ(arg1),
         file->GetXMin(), file->GetXMax(),
         file->GetZMin(), file->GetZMax());

    ui->lineEdit_y->setText(QString::number(file->GetDataY(arg1)));

    PlotFitFunc();
}

void MainWindow::on_spinBox_x_valueChanged(int arg1){
    ui->lineEdit_x->setText(QString::number(file->GetDataX(arg1)));
    PlotFitFunc();
}

void MainWindow::PlotFitFunc(){ // also initialize ana

    if( file == NULL) return;

    QVector<double> par;
    par.push_back(ui->lineEdit_a->text().toDouble());
    par.push_back(ui->lineEdit_Ta->text().toDouble());
    par.push_back(ui->lineEdit_b->text().toDouble());
    par.push_back(ui->lineEdit_Tb->text().toDouble());

    int yIndex = ui->spinBox_y->value();

    if(ana != NULL) delete ana;
    ana = new Analysis(file->GetDataSetX(), file->GetDataSetZ(yIndex));
    connect(ana, SIGNAL(SendMsg(QString)), this, SLOT(Write2Log(QString)));
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
    QVector<double> par;
    par.push_back(ui->lineEdit_a->text().toDouble());
    par.push_back(ui->lineEdit_Ta->text().toDouble());
    par.push_back(ui->lineEdit_b->text().toDouble());
    par.push_back(ui->lineEdit_Tb->text().toDouble());


    //ana->Print();

    ana->MeanAndvariance(0, ana->GetStartFitIndex()-20);
    ana->NonLinearFit(par);

    ana->PrintVector(ana->GetParameters(), "sol");
    ana->PrintVector(ana->GetParError(), "error");
    ana->PrintVector(ana->GetParPValue(), "p-Value");
    ana->PrintVector(ana->GetSSRgrad(), "SSR grad");

    Msg.sprintf("DF : %d, SSR: %f, delta: %e", ana->GetNDF(), ana->GetSSR(), ana->GetDelta() );
    Write2Log(Msg);
    double chisq = ana->GetFitVariance()/ana->GetSampleVariance();
    Msg.sprintf("Fit Variance : %f, Sample Variance : %f, Reduced Chi-squared : %f", ana->GetFitVariance(), ana->GetSampleVariance(), chisq);
    Write2Log(Msg);

    QVector<double> sol = ana->GetParameters();
    ui->lineEdit_a->setText(QString::number(sol[0]));
    ui->lineEdit_Ta->setText(QString::number(sol[1]));
    ui->lineEdit_b->setText(QString::number(sol[2]));
    ui->lineEdit_Tb->setText(QString::number(sol[3]));

    PlotFitFunc();
}

void MainWindow::on_pushButton_reset_clicked()
{
    ui->lineEdit_a->setText("20");
    ui->lineEdit_Ta->setText("20");
    ui->lineEdit_b->setText("-10");
    ui->lineEdit_Tb->setText("80");

    PlotFitFunc();
}
