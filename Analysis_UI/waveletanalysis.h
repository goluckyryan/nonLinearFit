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
    WaveletAnalysis(QVector<double> x, QVector<double> a, int waveletIndex);
    ~WaveletAnalysis();

    void Decompose();
    void RestoreData();
    void Reconstruct();
    void HardThresholding(double threshold, int sLimit);
    void CleanOutsider(double x1, double x2, int sLimit);

    void PrintV(int s, int flag = 0);
    void PrintV0(int s, int flag = 0);
    void PrintW(int s);
    void PrintW0(int s);

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
    int G0(int k){
        if( waveletIndex == 0){
            if(k == 0 || k == 1 ) return 1;
        }
        return 0;
    }
    int G1(int k){
        //if( waveletIndex == 0){
        //    if(k == 0 ) return 1;
        //    if(k == 1 ) return -1;
        //}

        return G0(k)*qPow(-1,k);
    }

    double H0(int k){
        //if(n == 0 || n==1 ) return 1/2.;
        return 0.5*G0(-k);
    }

    double H1(int k){
        //if(n == 0 ) return 1/2.;
        //if(n == 1 ) return -1/2.;
        return 0.5*qPow(-1,k)*G0(k+1);
    }

    QString msg;

    //unchange copy
    QVector<double> *V0;
    QVector<double> *W0;

    //gated copy
    QVector<double> *V;
    QVector<double> *W;

    QVector<double> *X0;

    int waveletIndex;
    int size;
    int M; // max scale

    double WAbsMax;


};

#endif // WAVELETANALYSIS_H
