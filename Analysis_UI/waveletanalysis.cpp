#include "waveletanalysis.h"

//WaveletAnalysis::WaveletAnalysis(QObject *parent) : QObject(parent)
WaveletAnalysis::WaveletAnalysis(QVector<double> a)
{
    size = a.size();
    M = qFloor( qLn(size)/qLn(2.) )+1;

    V = new QVector<double> [M];
    W = new QVector<double> [M];

    qDebug() << size << "--------------" << M;
    for( int i = 0; i < size; i++){
        V[0].push_back(a[i]);
        W[0].push_back(0.);
    }

    //Should calculate G0, G1, H1, H2 and R for semi-orthonormal wavelet;


}

WaveletAnalysis::~WaveletAnalysis(){
    if( V != NULL) delete [] V;
    if( W != NULL) delete [] W;
}

void WaveletAnalysis::Decompose(){
    int s;
    for( s = 0; s < M-1 ; s++){

        int sizeV = V[s].size();
        if( sizeV == 0) return;

        V[s+1].clear();
        W[s+1].clear();

        for(int k = 1; k < sizeV/2.+1; k++){
            double sum = 0;
            for(int l = 0; l < sizeV; l++){
                sum += H0(2*k-l-1)*V[s][l];
            }
            V[s+1].push_back(sum);

            sum = 0;
            for(int l = 0; l < sizeV; l++){
                sum += H1(2*k-l-1)*V[s][l];
            }
            W[s+1].push_back(sum);
        }

    }

    qDebug() << "Decomposed, s =" << s;

    //for( int r = 1; r < M ; r++){
    //    qDebug() << "W(" << r << ") = " << W[r].size();
    //}
}

void WaveletAnalysis::Recontruct(int s){
    if( s < 0 || s  >= M ) return;
    if( V[s+1].size() == 0 || W[s+1].size() == 0) return;

    int sizeV = (V[s+1].size()-1) * 2;

    V[s].clear();

    for(int l = 1; l <= sizeV; l++){
        double sum = 0;
        for(int k = 0; k < sizeV/2.+1; k++){
            sum += G0(l+1-2*k)*V[s+1][k];
            sum += G1(l+1-2*k)*W[s+1][k];
        }
        V[s].push_back(sum);
    }

}

void WaveletAnalysis::PrintV(int s)
{
    qDebug() << "V("<<s << ")" << V[s];
}

void WaveletAnalysis::PrintW(int s)
{
    qDebug() << "W("<<s << ")" << W[s];
}

