#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    zData = NULL;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete [] zData;
}

void MainWindow::on_pushButton_Open_clicked()
{
    QFileDialog fileDialog(this);
    QStringList filters;
    filters << "Row-wise (*txt *dat)";
    fileDialog.setNameFilters(filters);
    fileDialog.setReadOnly(1);
    //fileDialog.setDirectory(OPENPATH);
    QStringList fileNames;
    QString fileName;
    if( fileDialog.exec()) {
        fileNames = fileDialog.selectedFiles();
        fileName = fileNames[0];
    }

    if(fileName == "") return;

    ui->lineEdit->setText(fileName);

    //Open File
    OpenRow(fileName);

}

void MainWindow::OpenRow(QString fileName){

    QFile *fileIn = new QFile(fileName);
    fileIn->open(QIODevice::ReadOnly);

    fileIn->seek(0);

    QTextStream stream(fileIn);
    QString line;

    // get number of rows and cols;
    ySize = -1;
    while(stream.readLineInto(&line)){
        ySize ++;
        QStringList lineList = line.split(",");

        if( xSize == 0){ // get yDatax
            xSize = lineList.size()-1;
        }
    }

    if( ySize < 1 || xSize < 1) {
        Write2Log("!!!! Invalide file structure.");
        return;
    }

    zData = new QVector<double> [ySize];

    // get Data
    fileIn->seek(0);
    int rows = 0;
    int zMin = 0;
    int zMax = 0;
    while(stream.readLineInto(&line)){
        rows ++;
        QStringList lineList = line.split(",");

        if( rows == 1){ // get xDatax
            for( int i = 1 ; i < lineList.size() ; i++ ){
                xData.push_back((lineList[0]).toDouble() * 1e6) ;
            }
        }else{
            double temp = ExtractYValue(lineList[0]);
            yData.push_back(temp);
            int yIndex = 0;
            for( int i = 1 ; i < lineList.size() ; i++ ){
                temp = (lineList[i]).toDouble() * 1000;
                zData[yIndex].push_back(temp);
                if( temp > zMax ) zMax = temp;
                if( temp < zMin ) zMin = temp;
            }
        }
    }

    qDebug("X: %d , %d", xData.size(), xSize);
    qDebug("Y: %d , %d", yData.size(), ySize);

    int xMin = FindMin(xData);
    int xMax = FindMax(xData);

    int yMin = FindMin(yData);
    int yMax = FindMax(yData);

    QString msg;
    msg.sprintf("X:(%7.3f, %7.3f) sizeX:%d",xMin, xMax, xSize);
    Write2Log(msg);

    msg.sprintf("Y:(%7.3f, %7.3f) sizeY:%d",yMin, yMax, ySize);
    Write2Log(msg);

    msg.sprintf("Z:(%7.3f, %7.3f)",zMin, zMax);
    Write2Log(msg);

}

void MainWindow::Write2Log(QString str){
    ui->textEdit->append(str);
    qDebug()<< str;
    ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
}

void MainWindow::on_pushButton_Convert_clicked()
{
    //Save as CSV
}


double MainWindow::FindMax(QVector<double> vec)
{
    double max = vec[0];
    for( int i = 1; i < vec.size(); i++){
        if( vec[i] > max) max = vec[i];
    }
    return max;
}

double MainWindow::FindMin(QVector<double> vec)
{
    double min = vec[0];
    for( int i = 1; i < vec.size(); i++){
        if( vec[i] < min) min = vec[i];
    }
    return min;
}

double MainWindow::ExtractYValue(QString str){
    int pos = str.lastIndexOf("_") ;
    int pos2 = str.lastIndexOf("FUNCTION");
    QString strY;
    if( pos2 == -1){
        strY = str.mid(pos+1);
    }else{
        strY = str.mid(pos+1, pos2-pos-1);
    }
    //qDebug() << str << ", " << pos << ", " << strY;
    return strY.toDouble();
}
