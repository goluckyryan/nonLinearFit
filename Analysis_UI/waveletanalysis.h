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
    void RestoreData();
    void Recontruct();
    void HardThresholding(double threshold, int sLimit);

    void PrintV(int s);
    void PrintW(int s);

    QVector<double>* GetW() { return W; }
    QVector<double>* GetV() { return V; }
    QVector<double> GetWoct(int s) { return W[s]; }
    QVector<double> GetVoct(int s) { return V[s]; }

    QVector<double>* GetW0() { return W0; }
    QVector<double>* GetV0() { return V0; }
    QVector<double> GetW0oct(int s) { return W0[s]; }
    QVector<double> GetV0oct(int s) { return V0[s]; }
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

    //unchange copy
    QVector<double> *V0;
    QVector<double> *W0;

    //gated copy
    QVector<double> *V;
    QVector<double> *W;

    int size;
    int M; // max scale

    double WAbsMax;


};

#endif // WAVELETANALYSIS_H
