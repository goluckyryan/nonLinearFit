#ifndef FILEIO_H
#define FILEIO_H

#include <QObject>
#include <QDebug>
#include <QString>
#include <QFile>
#include <qmath.h>
#include "constant.h"
#include "analysis.h"
#include "fftw3.h"
#include <complex.h>

class FileIO : public QObject
{
    Q_OBJECT
public:
    explicit FileIO();
    explicit FileIO(QString filepath);
    ~FileIO();

    void Initialize();

    void OpenSaveFile();
    void OpenCSVData();
    void OpenTxtData_col();
    void OpenTxtData_row();
    void SaveFitResult(Analysis *ana);
    void SaveSingleXCVS();

    //Data manipulation // id is bitwise
    void ManipulateData(int id, int bgIndex = -1, int n = -1);

signals:

    void SendMsg(QString msg);

public slots:

    QString GetFilePath(){return filePath;}
    QString GetSimFilePath(){return cvsFilePath;}
    bool IsColWise(){return colwise;}
    int GetDataSize(){ return xData.size();}
    int GetDataSetSize() {return yData.size();}
    QVector<double> GetDataSetZ(int yIndex){return zData[yIndex];}
    QVector<double> * GetData(){return zData;}
    QVector<double> * GetFFTDataA(){return fZDataA;}
    QVector<double> * GetFFTDataP(){return fZDataP;}
    QVector<double> GetDataSetY(){return yData;}
    QVector<double> GetDataSetX(){return xData;}
    QVector<double> GetDataSetMeanZ(){return zMean;}
    double GetDataZ(int xIndex, int yIndex){return zData[yIndex][xIndex];}
    double GetDataX(int xIndex){ return xData[xIndex];}
    double GetDataY(int yIndex){ return yData[yIndex];}
    double GetDataMeanZ(int yIndex){ return zMean[yIndex];}
    double GetDataMeanZMean() {return zMeanMean;}

    double GetXMax(){return xMax;}
    double GetXMin(){return xMin;}
    double GetYMax(){return yMax;}
    double GetYMin(){return yMin;}
    double GetZMax(){return zMax;}
    double GetZMin(){return zMin;}

    double GetfXMax(){return fxMax;}
    double GetfXMin(){return fxMin;}
    double GetfYMax(){return fyMax;}
    double GetfYMin(){return fyMin;}

    int GetMultiIndex(){return multi;}

    bool IsOpen(){return openState;}
    bool IsYRevered(){return yRevered;}
    bool HasBackGround(){return hadBG;}

    //fourier transform and filters
    void FouierForward();
    void FouierBackward();
    void FouierForwardSingle(int yIndex);
    void FouierBackwardSingle(int yIndex);
    void SwapFFTData(bool dir);
    void FFTWFilters(int filterID, QVector<double> par);
    void RemoveYConstant();
    void MovingAvgonFFTW(int n);

private:
    QVector<double> xData; // time data
    QVector<double> yData; // B-field
    QVector<double> *zData; //data, [ydata][xdata]
    QVector<double> *backUpData; //backup data, [ydata][xdata]
    QVector<double> zMean; // mean

    QVector<double> fxData;
    QVector<double> fyData;
    QVector<double> *fZDataA; //amp of fourier zData, [ydata][xdata]
    QVector<double> *fZDataP; //phase fourier zData, [ydata][xdata]

    int xSize, ySize;
    double xMin, xMax;
    double yMin, yMax;
    double zMin, zMax;
    double zMeanMean;

    double fxMin, fxMax;
    double fyMin, fyMax;
    double ffreqResol;

    bool colwise; // 0 = data store as row, 1 = data store as col
    bool openState;
    bool isOutFileOpened;
    bool yRevered;
    bool hadBG;

    int multi;

    QFile * myfile;
    QFile * outfile;
    QString filePath, cvsFilePath;

    //Private functions
    double ExtractYValue(QString str);

    double FindMax(QVector<double> vec);
    double FindMin(QVector<double> vec);

    QVector<double> Shift(QVector<double> list, int d);

    void CalMeanVector();
    void RescaleZData();

    int FindIndex(QVector<double> vec, double goal, bool dir);
};

#endif // FILEIO_H
