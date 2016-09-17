#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    plot = ui->customPlot;

    /*
    QVector<double> x;
    x.push_back(0);
    x.push_back(1);
    x.push_back(2);
    x.push_back(3);
    x.push_back(4);
    x.push_back(5);
    x.push_back(6);

    Analysis ana(x,x);

    ana.MeanAndvariance(0, x.size()-1);

    ana.Print();
    */
}

MainWindow::~MainWindow()
{
    delete ui;
    delete plot;
}

void MainWindow::Write2Log(QString str){
    ui->plainTextEdit->appendPlainText(str);
}

void MainWindow::on_pushButton_clicked(){
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this,
                                            "Open File",
                                            "/Users/mobileryan/Triplet-ESR");

    //qDebug()<< fileName;

    FileIO file(fileName);
    file.OpenCSVData();

    plot->addGraph();
    plot->graph(0)->addData(file.GetDataSetX(), file.GetDataSetZ(105));
    plot->xAxis->setLabel("time");
    plot->yAxis->setLabel("a.u.");
    plot->xAxis->setRange(file.GetXMin(), file.GetXMax());
    plot->yAxis->setRange(file.GetZMin(), file.GetZMax());
    plot->replot();


    //Analysis ana(file.GetDataSetX(), file.GetDataSetZ(0));
    //QVector<double> par = {20, 20, -20, 100};
    //ana.Regression(1, par);

    //qDebug() << file.GetDataY(0);
    //for( int i = 0; i < 10; i++){
    //    qDebug("%d, %d : %f, %f", i, 0, file.GetDataX(i), file.GetData(i,0));
    //}

}
