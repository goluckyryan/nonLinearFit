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

    //Should calculate G0, G1, H1, H2 and R for semi-orthonormal wavelet;

}

WaveletAnalysis::~WaveletAnalysis(){
    if( V0 != NULL) delete [] V0;
    if( W0 != NULL) delete [] W0;
    if( X0 != NULL) delete [] X0;

    if( V != NULL) delete [] V;
    if( W != NULL) delete [] W;
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

        for(int k = 1; k <= sizeV/2.; k++){
            double sum = 0;
            for(int l = 1; l <= sizeV; l++){
                sum += H0(2*k-l)*V0[s][l-1];
            }
            V0[s+1].push_back(sum);

            sum = 0;
            for(int l = 1; l <= sizeV; l++){
                sum += H0(2*k-l)*X0[s][l-1];
            }
            X0[s+1].push_back(sum);

            sum = 0;
            for(int l = 1; l <= sizeV; l++){
                sum += H1(2*k-l)*V0[s][l-1];
            }
            W0[s+1].push_back(sum);

            if( WAbsMax < sum) WAbsMax = sum;
        }

        //PrintV0(s+1,1);

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
    for(int s = 1; s < M ; s++){

        V[s].clear();
        W[s].clear();

        for(int k = 0; k < V0[s].size(); k++){
            V[s].push_back(V0[s][k]);
            W[s].push_back(W0[s][k]);
        }

        //PrintV(s,1);
    }

}

void WaveletAnalysis::Recontruct(){

    for( int s = M-1; s > 0; s--){
        V[s-1].clear();

        for(int l = 1; l <= V0[s].size() * 2; l++){
            double sum = 0;
            for(int k = 1; k <= V[s].size(); k++){
                sum += G0(l+1-2*k)*V[s][k-1];
                sum -= G1(l+1-2*k)*W[s][k-1];
            }
            V[s-1].push_back(sum);
        }

        //PrintV(s-1,1);
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

