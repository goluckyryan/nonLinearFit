#ifndef WAVELETANALYSIS_H
#define WAVELETANALYSIS_H

//#include <QObject>
#include <QVector>
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
    void Recontruct(int s);

    void PrintV(int s);
    void PrintW(int s);

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
        if(n == 0 ) return -1/2.;
        if(n == 1 ) return  1/2.;
        return 0;
    }

    QVector<double> *V;
    QVector<double> *W;

    int size;
    int M; // max scale


};

#endif // WAVELETANALYSIS_H
