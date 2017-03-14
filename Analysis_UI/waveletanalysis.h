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
    double G0(int k){
        QVector<double> g0;
        g0.clear();
        if( waveletIndex == 0){ // Haar
            g0 = {1,1};
        }

        if( waveletIndex == 1){ // Daubechies
            switch (waveletPar) {
            case 1: g0 = { 1, 1}; break;
            case 2: g0 = {-0.183013, 0.316987, 1.18301, 0.683013}; break;
            case 3: g0 = {0.0498175, -0.120832, -0.190934, 0.650365, 1.14112, 0.470467}; break;
            case 4: g0 = {-0.014987, 0.0465036, 0.0436163, -0.264507, -0.039575, 0.8922, 1.01095, 0.325803}; break;
            case 5: g0 = {0.00471743, -0.0177919, -0.0088268, 0.109703, -0.0456011, -0.342657, 0.195767, 1.02433, 0.853944, 0.226419}; break;
            case 6: g0 = {-0.00152353, 0.00675606, 0.000783251, -0.0446637, 0.0389232, 0.137888, -0.183518, -0.319987, 0.445831, 1.06226, 0.699504, 0.157742}; break;

            case 7: g0 = {0.000500227, -0.0025479, 0.000607515, 0.0177498, -0.0234399, -0.0537825, 0.114003, 0.100846, -0.316835, \
                          -0.203514, 0.664372, 1.03115, 0.560791, 0.110099}; break;

            case 8: g0 = {-0.000166137, 0.00095523, -0.000554005, -0.00688772, 0.0123688, 0.0197722, -0.0623502, -0.0245639, 0.182076, \
                          0.000668194, -0.401659, -0.0223857, 0.827817, 0.955486, 0.442467, 0.0769556}; break;

            case 9: g0 = {0.0000556455, -0.00035633, 0.000325815, 0.00261297, -0.00605496, -0.00667962, 0.0316242, \
                          0.000354893, -0.0956473, 0.0434527, 0.210068, -0.136954, -0.414752, 0.18837, 0.929546, 0.855349, \
                          0.344834, 0.0538503}; break;

            case 10: g0 = {-0.0000187584, 0.000132354, -0.000164709, -0.000969948, 0.00281769, 0.00197333, -0.015179, 0.00510044, 0.0469698, -0.0416592, \
                           -0.100967, 0.131603, 0.180127, -0.27711, -0.353336, 0.397638, 0.973628, 0.745575, 0.266122, 0.0377172}; break;

            case 11: g0 = {6.35586e-6, -0.0000489813, 0.0000769885, 0.000352355, -0.00126293, -0.000436416, 0.00696984, -0.00472469, -0.0217291, 0.0294735, \
                           0.0443145, -0.0939586, -0.0657326, 0.211866, 0.0933997, -0.387821, -0.229492, 0.582606, 0.969708, 0.636254, \
                           0.203742, 0.0264377}; break;

            case 12: g0 = {-2.16243e-6, 0.0000180693, -0.0000342827, -0.000125164, 0.000549638, 9.25621e-6, -0.00308228, 0.00318001, \
                           0.00949149, -0.0181597, -0.0172798, 0.0587553, 0.015343, -0.136376, 0.00757958, 0.258064, -0.0336289, -0.447144, \
                           -0.0633057, 0.729574, 0.929419, 0.533661, 0.15495, 0.0185435}; break;
            }

        }

        // output
        parSize = g0.size();
        if( k >= 0 && k < parSize){
            return g0[k];
        }
        return 0;
    }
    double G1(int k){
        return G0(1-k)*qPow(-1,k);
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
    int waveletPar;
    int parSize;
    int size;
    int M; // max scale

    double WAbsMax;


};

#endif // WAVELETANALYSIS_H
