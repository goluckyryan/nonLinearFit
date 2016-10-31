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
    if( fileDialog.exec()) {
        fileNames = fileDialog.selectedFiles();
        fileName  = fileNames[0];
    }

    if(fileName == "") return;

    ui->lineEdit->setText(fileName);

    //Open File
    OpenRow(fileName);

}

void MainWindow::OpenRow(QString fileName){

    xData.clear();
    yData.clear();
    yName.clear();
    delete [] zData;

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
    double zMin;
    double zMax;
    int yIndex = 0;
    while(stream.readLineInto(&line)){
        rows ++;
        QStringList lineList = line.split(",");

        if( rows == 1){ // get xDatax
            for( int i = 1 ; i < lineList.size() ; i++ ){
                xData.push_back((lineList[i]).toDouble()) ;
            }
        }else{
            yName.push_back(lineList[0]);
            double temp = ExtractYValue(lineList[0]);
            yData.push_back(temp);
            for( int i = 1 ; i < lineList.size() ; i++ ){
                temp = (lineList[i]).toDouble();
                zData[yIndex].push_back(temp);
                if( i == 1) {
                    zMax = temp;
                    zMin = temp;
                }
                if( temp > zMax ) zMax = temp;
                if( temp < zMin ) zMin = temp;

            }
            yIndex++;
        }
    }

    qDebug("X: %d , %d", xData.size(), xSize);
    qDebug("Y: %d , %d", yData.size(), ySize);

    double xMin = FindMin(xData);
    double xMax = FindMax(xData);

    double yMin = FindMin(yData);
    double yMax = FindMax(yData);

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


   QString saveFile = fileName;
   saveFile.chop(3);
   saveFile.append("csv");

   QFile fileOut(saveFile);
   fileOut.open(QIODevice::WriteOnly);

   QTextStream stream(&fileOut);
   QString line, tmp;
   line.sprintf("%10s, ", "");
   for( int i = 0; i < ySize-1; i++){
       tmp.sprintf("%s, ", yName[i].toStdString().c_str());
       line.append(tmp);
   }
   tmp.sprintf("%s \n", yName[ySize-1].toStdString().c_str());
   line.append(tmp);
   stream << line;

   for(int j = 0; j < xSize ; j++){
       line.sprintf("%f, ", xData[j]);

       for( int i = 0; i < ySize-1; i++){
           tmp.sprintf("%f,", zData[i][j]);
           line.append(tmp);
       }
       tmp.sprintf("%f \n", zData[ySize-1][j]);
       line.append(tmp);
       stream << line;
   }

    fileOut.close();
    tmp.sprintf("Converted and Save to :%s ", saveFile.toStdString().c_str());
    Write2Log(tmp);
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
