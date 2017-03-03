#ifndef WAVELETANALYSIS_H
#define WAVELETANALYSIS_H

//#include <QObject>
#include <QVector>
#include <QString>
#include <QtMath>
#include <QDebug>

class WaveletAnalysis //: public QObject
{
//    Q_OBJECT
public:
    //explicit WaveletAnalysis(QObject *parent = 0);
    WaveletAnalysis(QVector<double> a);
    ~WaveletAnalysis();

    void Decompose();
    void Recontruct();
    void HardThresholding(double threshold, int sLimit);

    void PrintV(int s);
    void PrintW(int s);

    QVector<double>* GetW() { return W; }
    QVector<double>* GetV() { return V; }
    QVector<double> GetWoct(int s) { return W[s]; }
    QVector<double> GetVoct(int s) { return V[s]; }
    QVector<double> GetVOrigin() { return origin; }
    int GetM() {return M;}
    int GetSize() {return size;}
    QString GetMsg() {return msg;}
    double GetWAbsMax() {return WAbsMax;}

//signals:

//public slots:

private:
    // this is specific for Haar wavelet
    int G0(int n){
        if(n == 0 || n==1 ) return 1;
        return 0;
    }
    int G1(int n){
        if(n == 0 ) return 1;
        if(n == 1 ) return -1;
        return 0;
    }

    double H0(int n){
        if(n == 0 || n==1 ) return 1/2.;
        return 0;
    }

    double H1(int n){
        if(n == 0 ) return 1/2.;
        if(n == 1 ) return -1/2.;
        return 0;
    }

    QString msg;

    QVector<double> origin;

    QVector<double> *V;
    QVector<double> *W;

    int size;
    int M; // max scale

    double WAbsMax;


};

#endif // WAVELETANALYSIS_H
