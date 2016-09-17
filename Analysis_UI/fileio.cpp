#include "fileio.h"

FileIO::FileIO()
{
    myfile = NULL;
    zData = NULL;
    colwise = 0;
}

FileIO::FileIO(QString filePath){
    this->filePath = filePath;
    myfile = new QFile(filePath);

    openStatus = myfile->open(QIODevice::ReadOnly);

}

FileIO::~FileIO(){
    delete myfile;
    //delete zData;
}

void FileIO::FileStructure(bool colwise){

    this->colwise = colwise;

    myfile->seek(0);
    QTextStream stream(myfile);
    QString line;

    int cols = 0;
    int rows = 0;
    while(stream.readLineInto(&line)){
        rows++;
        if( rows == 1){
            QStringList lineList = line.split(",");
            cols = lineList.size()-1;
        }
    }
    rows --;

    //qDebug() << rows << "," << cols;

    if( colwise){
        ySize = cols;
        xSize = rows;
    }else{
        xSize = cols;
        ySize = rows;
    }


    this->zData = new QVector<double> [ySize];

    qDebug("X %d, Y %d", xSize, ySize);

    if( this->zData == NULL){
        qDebug() << "jjjjj";
    }

}

void FileIO::StoreCSVData(){

    this->colwise = 1;

    myfile->seek(0);

    QTextStream stream(myfile);
    QString line;

    int rows = 0;
    while(stream.readLineInto(&line)){
        rows ++;
        QStringList lineList = line.split(",");

        if( rows == 1){ // get yData
            for( int i = 1 ; i < lineList.size() ; i++ ){
                yData.push_back(0) ; // get data from string.
            }
        }else{
            xData.push_back( (lineList[0]).toDouble() );
            for( int i = 1 ; i < lineList.size() ; i++ ){
                zData[rows-1].push_back( (lineList[i]).toDouble() ); ; // get data from string.
            }
        }

    }

    //transpose the zData
    for( int i = 0; i < xSize ; i ++){
        for(int j = 0; j < ySize ; j++){
            double temp = zData[i][j];
            zData[i][j] = zData[j][i];
            zData[j][i] = temp;
        }
    }

    //check
    qDebug("X: %d , %d, %d", xSize, xData.size(), zData[0].size());
    qDebug("Y: %d , %d, %d", ySize, yData.size(), zData->size());

}

void FileIO::StoreTxtData(){
    this->colwise = 0;
    myfile->seek(0);

    QTextStream stream(myfile);
    QString line;

    int rows = 0;
    while(stream.readLineInto(&line)){
        QStringList lineList = line.split(",");
        rows ++;
        if( rows == 1){ // get xData
            for( int i = 1 ; i < lineList.size() ; i++ ){
                xData.push_back((lineList[i]).toDouble()) ;
            }
        }else{
            yData.push_back(0);
            for( int i = 1 ; i < lineList.size() ; i++ ){
                zData[rows-1].push_back((lineList[i]).toDouble() ); ; // get data from string.
            }
        }

    }

    //check
    qDebug("X: %d , %d, %d", xSize, xData.size(), zData[0].size());
    qDebug("Y: %d , %d, %d", ySize, yData.size(), zData->size());

}
