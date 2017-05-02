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

    void ClearData();
    void SetData(QVector<double> x, QVector<double> a);
    void setWaveletPar(int waveletIndex, int waveletPar);
    void setNormFactor(double f){ normFactor = f;}
    void Decompose();
    void RestoreData();
    void Reconstruct(int octave);
    void CalculateEnergy(bool originEnergyFlag = true);
    void HardThresholding(double threshold, int octave, int type = 0);
    void SoftThresholding(double threshold, int octave);
    void CleanOutsider(double x1, double x2, int octave);

    void PrintArray(QVector<double> y, QString str, int octave);

    QVector<double>* GetW() { return W; }
    QVector<double>* GetV() { return V; }
    QVector<int>* GetWk() { return Wk; }
    QVector<int>* GetVk() { return Vk; }
    QVector<double> GetVoctave(int s) { return V[s]; }
    QVector<double> GetWoctave(int s) { return W[s]; }
    QVector<int> GetVkoctave(int s) { return Vk[s]; }
    QVector<int> GetWkoctave(int s) { return Wk[s]; }

    QVector<double>* GetW0() { return W0; }
    QVector<double>* GetV0() { return V0; }
    QVector<double> GetW0octave(int s) { return W0[s]; }
    QVector<double> GetV0octave(int s) { return V0[s]; }
    QVector<double> GetEnergy0() { return energy0; }
    QVector<double> GetEnergy() { return energy; }
    double GetTotalEnergy0() { return TotalEnergy0; }
    double GetTotalEnergy() { return TotalEnergy; }
    int GetMaxScale() {return MaxScale;}
    int GetSize() {return size;}
    QString GetMsg() {return msg;}
    double GetWAbsMax() {return WAbsMax;}
    double GetVAbsMax() {return VAbsMax;}
    int GetWaveletNumberOfKind() {return numberOfKind;}
    double GetNormFactor() {return normFactor;}

//signals:

//public slots:

private:
    double G0(int k){
        if( k >= 0 && k < parSize){
            return Z0[k];
        }
        return 0;
    }
    //The following formula are for Orthonormal wavelet.
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

    QVector<double> energy0;
    QVector<double> energy;
    double TotalEnergy0;
    double TotalEnergy;

    //gated copy
    QVector<double> *V;
    QVector<double> *W;

    QVector<double> Z0;

    double normFactor;

    int waveletIndex;
    int numberOfKind;
    int waveletPar;
    int parSize;
    int size;
    int MaxScale; // max scale

    double WAbsMax, VAbsMax;


};

#endif // WAVELETANALYSIS_H
