#ifndef FILEIO_H
#define FILEIO_H

#include <QObject>
#include <QDebug>
#include <QString>
#include <QFile>
#include "constant.h"
#include "analysis.h"

class FileIO : public QObject
{
    Q_OBJECT
public:
    explicit FileIO();
    explicit FileIO(QString filepath);
    ~FileIO();

    void Initialize();
    void OpenSaveFile();
    void FileStructure(bool colwise);
    void OpenCSVData();
    void OpenTxtData_col();
    void OpenTxtData_row();
    void SaveFitResult(Analysis *ana);
    void SaveSimplifiedTxt();

signals:

    void SendMsg(QString msg);

public slots:

    QString GetFilePath(){return filePath;}
    QString GetSimFilePath(){return simFilePath;}
    bool IsColWise(){return colwise;}
    int GetDataSize(){ return xData.size();}
    int GetDataSetSize() {return yData.size();}
    QVector<double> GetDataSetZ(int yIndex){return zData[yIndex];}
    QVector<double> GetDataSetY(){return yData;}
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
    bool isOutFileOpened;

    QFile * myfile;
    QFile * outfile;
    QString filePath, simFilePath;

    double ExtractYValue(QString str);

    double FindMax(QVector<double> vec);
    double FindMin(QVector<double> vec);
};

#endif // FILEIO_H
