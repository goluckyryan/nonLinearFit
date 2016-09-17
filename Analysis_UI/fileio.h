#ifndef FILEIO_H
#define FILEIO_H

#include <QObject>
#include <QDebug>
#include <QString>
#include <QFile>

class FileIO : public QObject
{
    Q_OBJECT
public:
    explicit FileIO();
    explicit FileIO(QString filepath);
    ~FileIO();

    void FileStructure(bool colwise);
    void StoreCSVData();
    void StoreTxtData();

signals:

    void SendMsg(QString msg);

public slots:

    QString GetFilePath(){return filePath;}
    bool IsColWise(){return colwise;}
    int GetDataSize(){ return xData.size();}
    int GetDataSetSize() {return yData.size();}
    double GetData(int xIndex, int yIndex){return zData[yIndex][xIndex];}

private:
    QVector<double> xData; // time data
    QVector<double> yData; // B-field
    QVector<double> *zData; //data, [ydata][xdata]

    int xSize, ySize;

    bool colwise; // 0 = data store as row, 1 = data store as col

    QFile * myfile;

    QString filePath;

    bool openStatus;


};

#endif // FILEIO_H
