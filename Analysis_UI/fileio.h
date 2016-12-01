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

    void OpenHV2MagParametersFile();
    void OpenSaveFileforFit();
    void OpenCSVData();
    void OpenTxtData_col();
    void OpenTxtData_row();
    void SaveFitResult(Analysis *ana);
    void SaveCSV(bool doubleX, bool origin);
    void SaveTxtData_row();

    //Data manipulation // id is bitwise
    void ManipulateData(int id, int bgIndex = -1, int n = -1);

signals:

    void SendMsg(QString msg);

public slots:

    QString GetFilePath(){return filePath;}
    QString GetSimFilePath(){return csvFilePath;}
    bool IsColWise(){return colwise;}
    int GetDataSize(){ return xData.size();}
    int GetDataSetSize() {return yData_CV.size();}
    QVector<double> GetDataSetZ(int yIndex){return zData[yIndex];}
    QVector<double> * GetData(){return zData;}
    QVector<double> * GetFFTDataA(){return fZDataA;}
    QVector<double> * GetFFTDataP(){return fZDataP;}
    QVector<double> GetDataSetY(){return yData_CV;}
    QVector<double> GetDataSetX(){return xData;}
    QVector<double> GetDataSetMeanZ(){return zMean;}
    double GetDataZ(int xIndex, int yIndex){return zData[yIndex][xIndex];}
    double GetDataX(int xIndex){ return xData[xIndex];}
    double GetDataY_CV(int yIndex){ return yData_CV[yIndex];}
    double GetDataY_HV(int yIndex){ return yData_HV[yIndex];}
    QString GetDataYName(int yIndex){return yString[yIndex];}
    double GetDataMeanZ(int yIndex){ return zMean[yIndex];}
    double GetDataMeanZMean() {return zMeanMean;}

    int GetXIndex(double x);
    int GetYIndex_CV(double y);
    int GetYIndex_HV(double y);

    int GetBGIndex(){return bgIndex;}
    double GetXMax(){return xMax;}
    double GetXMin(){return xMin;}
    double GetYMax_CV(){return yMax_CV;}
    double GetYMin_CV(){return yMin_CV;}
    double GetYMax_HV(){return yMax_HV;}
    double GetYMin_HV(){return yMin_HV;}
    double GetZMax(){return zMax;}
    double GetZMin(){return zMin;}
    double GetYStep_CV(){return yStep_CV;}
    double GetYStep_HV(){return yStep_HV;}

    QVector<double> GetfXDataSet(){ return fxData;}
    double GetfXData(int index){return fxData[index];}
    double GetfYData(int index){return fyData[index];}
    double GetfResol(){return fFreqResol;}
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
    void FFTWFilters(int filterID, QVector<double> par, QVector<double> funcX, QVector<double> funcY, bool rev);
    void RemoveYConstant();
    void MovingAvgonFFTW(int n);

    double HV2Mag(double HV){
        double mag = 0;
        for(int i = 0; i < hallPar.size() ; i++){
            mag += hallPar[i] * pow(HV, i);
        }
        return mag;
    }

    double Mag2HV(double mag){
        // usually only 1st order matter;
        return (mag - hallPar[0])/hallPar[1];
    }

private:
    QVector<double> xData; // time data
    QVector<double> yData_CV; // Control Voltage
    QVector<double> yData_HV; // Hall Voltage
    QVector<double> *zData; //data, [ydata][xdata]
    QVector<double> *backUpData; //backup data, [ydata][xdata]
    QVector<double> zMean; // mean
    QVector<QString> yString; // store y-String for saving

    QVector<double> fxData;
    QVector<double> fyData;
    QVector<double> *fZDataA; //amp of fourier zData, [ydata][xdata]
    QVector<double> *fZDataP; //phase fourier zData, [ydata][xdata]

    int xSize, ySize;
    double xMin, xMax, xStep;
    double yMin_CV, yMax_CV, yStep_CV;
    double yMin_HV, yMax_HV, yStep_HV;
    double zMin, zMax;
    double zMeanMean;

    double fxMin, fxMax;
    double fyMin, fyMax;
    double fFreqResol;

    bool colwise; // 0 = data store as row, 1 = data store as col
    bool openState;
    bool isOutFileOpened;
    bool yRevered;
    bool hadBG;

    int multi;
    int bgIndex;

    QFile * myfile;
    QFile * outfile;
    QString filePath, csvFilePath;

    QFile * hallParFile;
    QVector<double> hallPar;


    //Private functions
    double ExtractYValue(QString str, int index = 0);

    double FindMax(QVector<double> vec);
    double FindMin(QVector<double> vec);

    QVector<double> Shift(QVector<double> list, int d);

    void CalMeanVector();
    void RescaleZData();
    int FindIndex(QVector<double> vec, double torr, double goal, bool dir);
};

#endif // FILEIO_H
