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
    void OpenCSVData();
    void OpenTxtData_col();
    void OpenTxtData_row();

signals:

    void SendMsg(QString msg);

public slots:

    QString GetFilePath(){return filePath;}
    bool IsColWise(){return colwise;}
    int GetDataSize(){ return xData.size();}
    int GetDataSetSize() {return yData.size();}
    QVector<double> GetDataSetZ(int yIndex){return zData[yIndex];}
    QVector<double> GetDataSetX(){return xData;}
    double GetDataZ(int xIndex, int yIndex){return zData[yIndex][xIndex];}
    double GetDataX(int xIndex){ return xData[xIndex];}
    double GetDataY(int yIndex){ return yData[yIndex];}

    double GetXMax(){return xMax;}
    double GetXMin(){return xMin;}
    double GetYMax(){return yMax;}
    double GetYMin(){return yMin;}
    double GetZMax(){return zMax;}
    double GetZMin(){return zMin;}

    bool IsOpen(){return openState;}

private:
    QVector<double> xData; // time data
    QVector<double> yData; // B-field
    QVector<double> *zData; //data, [ydata][xdata]

    int xSize, ySize;
    double xMin, xMax;
    double yMin, yMax;
    double zMin, zMax;
    bool colwise; // 0 = data store as row, 1 = data store as col
    bool openState;

    QFile * myfile;
    QString filePath;

    double GetYValue(QString str);

    double FindMax(QVector<double> vec);
    double FindMin(QVector<double> vec);
};

#endif // FILEIO_H
