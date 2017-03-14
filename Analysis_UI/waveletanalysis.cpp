#include "waveletanalysis.h"

//WaveletAnalysis::WaveletAnalysis(QObject *parent) : QObject(parent)
WaveletAnalysis::WaveletAnalysis(QVector<double> x, QVector<double> a)
{
    size = a.size();
    M = qFloor( qLn(size)/qLn(2.) )+1;

    V0 = new QVector<double> [M];
    W0 = new QVector<double> [M];
    V = new QVector<double> [M];
    W = new QVector<double> [M];
    X0 = new QVector<double> [M];

    for( int i = 0; i < size; i++){
        V0[0].push_back(a[i]);
        W0[0].push_back(0.);
        X0[0].push_back(x[i]);
    }

    msg.sprintf("Array size = %d; Max scale = %d", size, M);

}

WaveletAnalysis::~WaveletAnalysis(){
    if( V0 != NULL) delete [] V0;
    if( W0 != NULL) delete [] W0;
    if( X0 != NULL) delete [] X0;

    if( V != NULL) delete [] V;
    if( W != NULL) delete [] W;
}

void WaveletAnalysis::setWaveletPar(int waveletIndex, int waveletPar)
{
    this->waveletIndex = waveletIndex;
    this->waveletPar = waveletPar;

    G0(0); // cal parSize;

    switch (waveletIndex) {
    case 0: msg.sprintf("Haar wavelet"); break;
    case 1: msg.sprintf("Daubechies wavelet %d", waveletPar); break;
    }

    qDebug() << "=============== G0";
    for( int k = 0; k < parSize ; k++){
        qDebug() << k << " , " << G0(k);
    }
/*
    qDebug() << "=============== G1";
    for( int k = -3; k < 3 ; k++){
        qDebug() << k << " , " << G1(k);
    }
    qDebug() << "=============== H0";
    for( int k = -3; k < 3 ; k++){
        qDebug() << k << " , " << H0(k);
    }
    qDebug() << "=============== H1";
    for( int k = -3; k < 3 ; k++){
        qDebug() << k << " , " << H1(k);
    }
*/

}

void WaveletAnalysis::Decompose(){
    int s;
    WAbsMax = 0;
    for( s = 0; s < M-1 ; s++){

        int sizeV = V0[s].size();
        if( sizeV == 0) return;

        V0[s+1].clear();
        W0[s+1].clear();
        X0[s+1].clear();

        for(int k = 1; k <= sizeV/2.+1; k++){
            double sum = 0;
            for(int l = 0; l < sizeV; l++){
                sum += H0(2*k-l-2)*V0[s][l];
            }
            V0[s+1].push_back(sum);

            sum = 0;
            for(int l = 0; l < sizeV; l++){
                sum += H0(2*k-l-2)*X0[s][l];
            }
            X0[s+1].push_back(sum);

            sum = 0;
            for(int l = 0; l < sizeV; l++){
                sum += H1(2*k-l-2)*V0[s][l];
            }
            W0[s+1].push_back(sum);

            if( WAbsMax < sum) WAbsMax = sum;
        }

        PrintV0(s+1,1);

    }

    RestoreData();

    msg.clear();
    msg.sprintf("Decomposed scale = %d", s );

    //for(int s = 1; s < M; s++){
    //    PrintV(s);
    //    PrintW(s);
    //}

}

void WaveletAnalysis::RestoreData()
{
    //qDebug() << "=========================== RestoreData";
    for(int s = 1; s < M ; s++){

        V[s].clear();
        W[s].clear();

        for(int k = 0; k < V0[s].size(); k++){
            V[s].push_back(V0[s][k]);
            W[s].push_back(W0[s][k]);
        }

        //PrintV(s);
        //PrintW(s);
    }

}

void WaveletAnalysis::Reconstruct(){

    //qDebug() << "======================== Reconstruc";

    for( int s = M-1; s > 0; s--){
        //qDebug() << "---------- " << s;
        V[s-1].clear();

        //PrintV(s);
        //PrintW(s);

        for(int l = 1; l <= V0[s-1].size() ; l++){
            double sum = 0;
            for(int k = 0; k < V0[s].size(); k++){
                if( l-1-2*k < 0 || l-1-2*k > parSize) continue;
                sum += G0(l-1-2*k)*V[s][k];
                sum += G1(l-1-2*k)*W[s][k];
            }
            V[s-1].push_back(sum);
        }

        //PrintV(s-1,0);

    }

    msg.sprintf("Reconstructed.");
}

void WaveletAnalysis::HardThresholding(double threshold, int sLimit)
{
    if( sLimit == 0 ) return;
    if( threshold <= 0.0) return;
    for( int s = 1 ;  s <= qAbs(sLimit) ; s++){
        for( int k = 0; k <= W[s].size(); k++){
            if( qAbs(W[s][k]) < threshold ) {
                //qDebug() << s << "," << k << "," << W[s][k];
                W[s][k] = 0.;
            }
        }
    }
    msg.sprintf("Applied Hard Thresholding, level<%2.1f && scale>%d", threshold, sLimit);
}

void WaveletAnalysis::CleanOutsider(double x1, double x2, int sLimit)
{
    if( sLimit == 0 ) return;
    for( int s = 1 ;  s <= qAbs(sLimit) ; s++){
        for( int k = 0; k <= W[s].size(); k++){
            if( X0[s][k] < x1 || X0[s][k] > x2 ) {
                W[s][k] = 0.;
            }
        }
    }
}

void WaveletAnalysis::PrintV(int s, int flag)
{
    if( flag == 0){
        qDebug() << "V("<<s << "," << V[s].size() << ")" << V[s];
    }else{
        qDebug() << "V("<<s << "," << V[s].size() << ")" ;
    }
}

void WaveletAnalysis::PrintV0(int s, int flag)
{
    if( flag == 0){
        qDebug() << "V0("<<s << "," << V0[s].size() << ")" << V0[s];
    }else{
        qDebug() << "V0("<<s << "," << V0[s].size() << ")" ;
    }
}

void WaveletAnalysis::PrintW(int s)
{
    qDebug() << "W("<<s << ")" << W[s];
}

void WaveletAnalysis::PrintW0(int s)
{
    qDebug() << "W0("<<s << ")" << W0[s];
}
