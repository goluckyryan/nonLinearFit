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
    WaveletAnalysis(QVector<double> x, QVector<double> a);
    ~WaveletAnalysis();

    void setWaveletPar(int waveletIndex, int waveletPar);
    void Decompose();
    void RestoreData();
    void Reconstruct();
    void HardThresholding(double threshold, int sLimit);
    void CleanOutsider(double x1, double x2, int sLimit);

    void PrintArray(QVector<double> y, QString str, int s);

    QVector<double>* GetW() { return W; }
    QVector<double>* GetV() { return V; }
    QVector<int>* GetWk() { return Wk; }
    QVector<int>* GetVk() { return Vk; }
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
    double G0(int k){
        if( k >= 0 && k < parSize){
            return Z0[k];
        }
        return 0;
    }
    double G1(int k){
        return G0(1-k)*qPow(-1,k);
    }

    double H0(int k){
        return 0.5*G0(-k);
    }

    double H1(int k){
        return 0.5*qPow(-1,k)*G0(k+1);
    }

    QString msg;

    //unchange copy
    QVector<double> *W0; // the detial
    QVector<double> *V0; // the coarse
    QVector<int> *Wk; // the k-value of w
    QVector<int> *Vk; // the k-value of v
    QVector<double> *X0; // the x-position

    //gated copy
    QVector<double> *V;
    QVector<double> *W;

    QVector<double> Z0;


    int waveletIndex;
    int waveletPar;
    int parSize;
    int size;
    int M; // max scale

    double WAbsMax;


};

#endif // WAVELETANALYSIS_H
