#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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
}

void MainWindow::Write2Log(QString str){
    ui->plainTextEdit->appendPlainText(str);
}

void MainWindow::on_pushButton_clicked(){
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this,
                                            "Open File",
                                            "/Users/mobileryan/Triplet-ESR");

    qDebug()<< fileName;

    FileIO file(fileName);

    file.FileStructure(1);
    file.StoreCSVData();

}
