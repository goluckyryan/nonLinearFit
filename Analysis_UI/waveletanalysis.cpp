#include "waveletanalysis.h"

//WaveletAnalysis::WaveletAnalysis(QObject *parent) : QObject(parent)
WaveletAnalysis::WaveletAnalysis(QVector<double> x, QVector<double> a)
{
    SetData(x,a);
}

WaveletAnalysis::~WaveletAnalysis(){
    if( V0 != NULL) delete [] V0;
    if( W0 != NULL) delete [] W0;
    if( Vk != NULL) delete [] Vk;
    if( Wk != NULL) delete [] Wk;
    if( X0 != NULL) delete [] X0;

    if( V != NULL) delete [] V;
    if( W != NULL) delete [] W;
}

void WaveletAnalysis::ClearData()
{
    size = 0;
    MaxScale = 0;
    V0->clear();
    W0->clear();
    Vk->clear();
    Wk->clear();
    V->clear();
    W->clear();
    X0->clear();
    msg.clear();

    Z0.clear();
    waveletIndex = 0;
    waveletName = "";
    numberOfKind = 0;
    waveletPar = 0;
    normFactor = 1;

    TotalEnergy = 0;
    TotalEnergy0 = 0;
    energy.clear();
    energy0.clear();
}

void WaveletAnalysis::SetData(QVector<double> x, QVector<double> a)
{
    size = a.size();
    MaxScale = qFloor( qLn(size)/qLn(2.) );

    V0 = new QVector<double> [MaxScale];
    W0 = new QVector<double> [MaxScale];
    Vk = new QVector<int> [MaxScale];
    Wk = new QVector<int> [MaxScale];
    V = new QVector<double> [MaxScale];
    W = new QVector<double> [MaxScale];
    X0 = new QVector<double> [MaxScale];

    TotalEnergy0 = 0;
    TotalEnergy = 0;
    energy.clear();
    energy0.clear();
    for( int i = 0; i < size; i++){
        V0[0].push_back(a[i]);
        W0[0].push_back(0.);
        X0[0].push_back(x[i]);
        Vk[0].push_back(i);
        Wk[0].push_back(i);
        TotalEnergy0 += a[i]*a[i];
    }

    msg.sprintf("Array size = %d; Max scale = %d; Total Energy = %f [mV^2]", size, MaxScale, TotalEnergy0);
    normFactor = 1;
}

void WaveletAnalysis::setWaveletPar(int waveletIndex, int waveletPar)
{
    this->waveletIndex = waveletIndex;
    this->waveletPar = waveletPar;

    if( waveletIndex == 0){ // Haar
        Z0 = {1,1};
        numberOfKind = 1;
        waveletName = "Haar";
    }

    if( waveletIndex == 1){ // Daubechies
        numberOfKind = 12;
        waveletName = "Daubechies";
        switch (waveletPar) {
        case 1: Z0 = { 1, 1}; break;
        case 2: Z0 = {-0.183013, 0.316987, 1.18301, 0.683013}; break;
        case 3: Z0 = {0.0498175, -0.120832, -0.190934, 0.650365, 1.14112, 0.470467}; break;
        case 4: Z0 = {-0.014987, 0.0465036, 0.0436163, -0.264507, -0.039575, 0.8922, 1.01095, 0.325803}; break;
        case 5: Z0 = {0.00471743, -0.0177919, -0.0088268, 0.109703, -0.0456011, -0.342657, 0.195767, 1.02433, 0.853944, 0.226419}; break;
        case 6: Z0 = {-0.00152353, 0.00675606, 0.000783251, -0.0446637, 0.0389232, 0.137888, -0.183518, -0.319987, 0.445831, 1.06226, 0.699504, 0.157742}; break;

        case 7: Z0 = {0.000500227, -0.0025479, 0.000607515, 0.0177498, -0.0234399, -0.0537825, 0.114003, 0.100846, -0.316835, \
                      -0.203514, 0.664372, 1.03115, 0.560791, 0.110099}; break;

        case 8: Z0 = {-0.000166137, 0.00095523, -0.000554005, -0.00688772, 0.0123688, 0.0197722, -0.0623502, -0.0245639, 0.182076, \
                      0.000668194, -0.401659, -0.0223857, 0.827817, 0.955486, 0.442467, 0.0769556}; break;

        case 9: Z0 = {0.0000556455, -0.00035633, 0.000325815, 0.00261297, -0.00605496, -0.00667962, 0.0316242, \
                      0.000354893, -0.0956473, 0.0434527, 0.210068, -0.136954, -0.414752, 0.18837, 0.929546, 0.855349, \
                      0.344834, 0.0538503}; break;

        case 10: Z0 = {-0.0000187584, 0.000132354, -0.000164709, -0.000969948, 0.00281769, 0.00197333, -0.015179, 0.00510044, 0.0469698, -0.0416592, \
                       -0.100967, 0.131603, 0.180127, -0.27711, -0.353336, 0.397638, 0.973628, 0.745575, 0.266122, 0.0377172}; break;

        case 11: Z0 = {6.35586e-6, -0.0000489813, 0.0000769885, 0.000352355, -0.00126293, -0.000436416, 0.00696984, -0.00472469, -0.0217291, 0.0294735, \
                       0.0443145, -0.0939586, -0.0657326, 0.211866, 0.0933997, -0.387821, -0.229492, 0.582606, 0.969708, 0.636254, \
                       0.203742, 0.0264377}; break;

        case 12: Z0 = {-2.16243e-6, 0.0000180693, -0.0000342827, -0.000125164, 0.000549638, 9.25621e-6, -0.00308228, 0.00318001, \
                       0.00949149, -0.0181597, -0.0172798, 0.0587553, 0.015343, -0.136376, 0.00757958, 0.258064, -0.0336289, -0.447144, \
                       -0.0633057, 0.729574, 0.929419, 0.533661, 0.15495, 0.0185435}; break;
        }

    }

    if( waveletIndex == 2){ // Symlet
        numberOfKind = 10;
        waveletName = "Symlet";
        switch (waveletPar) {
        case 1: Z0 = { 1, 1}; break;
        case 2: Z0 = {0.683013, 1.18301, 0.316987, -0.183013}; break;
        case 3: Z0 = {0.470467, 1.14112, 0.650365, -0.190934, -0.120832, 0.0498175}; break;
        case 4: Z0 = {-0.107149, -0.041911, 0.703739, 1.13666, 0.421235, -0.140318, -0.0178247, 0.0455703}; break;
        case 5: Z0 = {0.0276322, -0.0298425, -0.247951, 0.0234789, 0.896582, 1.02305, \
                      0.281991, -0.0553442, 0.0417469, 0.0386548}; break;

        case 6: Z0 = {0.0217847, 0.00493661, -0.166863, -0.0683231, 0.694458, 1.11389, \
                     0.477904, -0.102725, -0.0297838, 0.0632506, 0.00249992, -0.0110319}; break;

        case 7: Z0 = {0.0145214, 0.00567134, -0.152464, -0.198057, 0.408184, 1.08578, \
                      0.758163, 0.0246657, -0.0700783, 0.0960148, 0.0431555, -0.0178704, \
                      -0.00148123, 0.00379266}; break;

        case 8: Z0 = {-0.00478346, -0.000766691, 0.0448236, 0.0107586, -0.202649, \
                     -0.0866536, 0.680745, 1.09911, 0.515399, -0.0734625, -0.0384935, \
                     0.0694905, 0.00538639, -0.0211457, -0.000428394, 0.00267279}; break;

        case 9: Z0 = {0.00151249, -0.000669142, -0.0145156, 0.0125289, 0.0877913, \
                      -0.0257864, -0.270894, 0.0498828, 0.873048, 1.01526, 0.337659, \
                      -0.0771722, 0.000825141, 0.0427444, -0.0163034, -0.0187694, \
                      0.000876503, 0.00198119}; break;

        case 10: Z0 = {0.00108917, 0.000135245, -0.0122206, -0.00207236, 0.0649509, \
                       0.0164189, -0.225559, -0.10024, 0.667071, 1.08825, 0.542813, \
                       -0.0502565, -0.0452408, 0.0707036, 0.00815282, -0.0287862, \
                       -0.00113754, 0.00649573, 0.0000806612, -0.00064959}; break;
        }

    }

    if( waveletIndex == 3){ // Coiflet
        numberOfKind = 3;
        waveletName = "Coiflet";
        switch (waveletPar) {
        case 1: Z0 = {-0.102859, 0.477859, 1.20572, 0.544281, -0.102859, -0.0221405}; break;
        case 2: Z0 = {0.0231752, -0.0586403, -0.0952792, 0.546042, 1.14936, 0.589734, \
                      -0.108171, -0.084053, 0.0334888, 0.00793577, -0.00257841, -0.00101901}; break;

        case 3: Z0 = {-0.00536484, 0.0110063, 0.0331671, -0.0930155, -0.0864415, 0.573007, \
                      1.12257, 0.605967, -0.10154, -0.116393, 0.0488682, 0.0224585, \
                      -0.0127392, -0.00364092, 0.00158041, 0.00065933, -0.000100386, \
                      -0.0000489315}; break;
        }

    }

    parSize = Z0.size();

    switch (waveletIndex) {
    case 0: msg.sprintf("Haar wavelet"); break;
    case 1: msg.sprintf("Daubechies wavelet %d", waveletPar); break;
    case 2: msg.sprintf("Symlet wavelet %d", waveletPar); break;
    case 3: msg.sprintf("Coiflet wavelet %d", waveletPar); break;
    }

    //qDebug() << "=============== G0";
    //for( int k = 0; k < parSize ; k++){
    //    qDebug() << k << " , " << G0(k);
    //}
    //qDebug() << "=============== G1";
    //for( int k = 2-parSize; k <= 1 ; k++){
    //    qDebug() << k << " , " << G1(k);
    //}
    //qDebug() << "=============== H0";
    //for( int k = 1-parSize; k <= 0 ; k++){
    //    qDebug() << k << " , " << H0(k);
    //}
    //qDebug() << "=============== H1";
    //for( int k = -1; k <= parSize - 2 ; k++){
    //    qDebug() << k << " , " << H1(k);
    //}

}

void WaveletAnalysis::Decompose()
{
    //qDebug() << "=========================== Decompose";
    int s;
    WAbsMax = 0;
    VAbsMax = 0;
    for( s = 0; s < MaxScale-1 ; s++){

        int sizeV = V0[s].size();
        if( sizeV == 0) return;

        //qDebug("clear vector arrays V0, W0, Vk, Wk, X0");
        V0[s+1].clear();
        W0[s+1].clear();
        Vk[s+1].clear();
        Wk[s+1].clear();
        X0[s+1].clear();

        //qDebug("calculate low pass filter bank");
        bool startStore = 0;
        for(int k = -parSize; k <= sizeV/2.; k++){
            double sum = 0;
            for(int l = 0; l < sizeV; l++){
                double h0 = H0(2*k-Vk[s][l]);
                if( h0 == 0) continue;
                sum += h0*V0[s][l];
            }
            if( sum != 0 ) startStore = 1;
            if( startStore){
                sum = sum * normFactor;
                Vk[s+1].push_back(k);
                V0[s+1].push_back(sum);
            }

            if( VAbsMax < qAbs(sum)) VAbsMax = qAbs(sum);
        }

        //qDebug("calculate hight pass filter bank");
        startStore = 0;
        for(int k = -parSize; k <= parSize + 10 + sizeV/2.; k++){
            double sum = 0;
            for(int l = 0; l < sizeV; l++){
                double h1 = H1(2*k-Vk[s][l]);
                if( h1 == 0) continue;
                sum += H1(2*k-Vk[s][l])*V0[s][l];
            }
            if( sum != 0) startStore = 1;
            if( startStore){
                sum = sum * normFactor;
                Wk[s+1].push_back(k);
                W0[s+1].push_back(sum);
            }

            if( WAbsMax < qAbs(sum)) WAbsMax = qAbs(sum);
        }

        //qDebug("calculate the x-position for each octave");
        for(int k = 0; k < sizeV/2.; k++){
            if( 2*k >= X0[s].size()) break;
            X0[s+1].push_back(X0[s][2*k]);
        }


        //int vkSize = Vk[s+1].size();
        //int wkSize = Wk[s+1].size();
        //qDebug("s= %d, Vk(%d, %d)= %d , V0 = %d, Wk(%d, %d) = %d , W0 = %d", \
        //       s+1, Vk[s+1][0], Vk[s+1][vkSize-1] , Vk[s+1].size(), V0[s+1].size(), \
        //        Wk[s+1][0], Wk[s+1][wkSize-1] , Wk[s+1].size(), W0[s+1].size() );

        //PrintArray(Vk[s+1], "Vk", s+1);
        //PrintArray(V0[s+1], "V0", s+1);
        //
        //PrintArray(Wk[s+1], "Wk", s+1);
        //PrintArray(W0[s+1], "W0", s+1);

    }

    //qDebug() << "|W|_max = " << WAbsMax;

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
    for(int s = 1; s < MaxScale ; s++){

        V[s].clear();
        W[s].clear();

        for(int k = 0; k < V0[s].size(); k++){
            V[s].push_back(V0[s][k]);
        }
        for(int k = 0; k < W0[s].size(); k++){
            W[s].push_back(W0[s][k]);
        }

        //PrintArray(Vk[s], "Vk", s);
        //PrintArray(V0[s], "V0", s);
        //
        //PrintArray(Wk[s], "Wk", s);
        //PrintArray(W0[s], "W0", s);

    }

}

void WaveletAnalysis::Reconstruct(int octave){

    //qDebug() << "======================== Reconstruct";

    //for( int s = MaxScale-1; s > 0; s--){
    for( int s = octave; s > 0; s--){
        //qDebug() << "---------- " << s;
        V[s-1].clear();

        //PrintArray(V[s], "V", s);
        //PrintArray(W[s], "W", s);


        for(int l = Vk[s-1][0]; l < Vk[s-1][0]+Vk[s-1].size() ; l++){
            double sum = 0;
            for(int k = 0; k < V[s].size(); k++){
                double g0 = G0(l-2*Vk[s][k]);
                if( g0  == 0 ) continue;
                sum += g0*V[s][k];
            }
            for(int k = 0; k < W[s].size(); k++){
                double g1 = G1(l-2*Wk[s][k]);
                if( g1 == 0) continue;
                sum += g1*W[s][k];

            }
            V[s-1].push_back(sum /  normFactor);
        }

        //PrintArray(V[s-1], "V", s-1);

    }

    msg.sprintf("Reconstructed.");
}

void WaveletAnalysis::CalculateEnergy(bool originEnergyFlag)
{
    TotalEnergy = 0;
    energy.clear();
    for(int s = 1; s < MaxScale ; s ++){
        int sizeW = W[s].size();
        double en = 0;
        for( int i = 0; i < sizeW ; i ++ ){
            en += W[s][i]*W[s][i] * qPow(sqrt(2.)/normFactor, 2*s);
        }
        energy.push_back(en);
        TotalEnergy += en;
    }

    int sizeV = V[MaxScale-1].size();
    double en = 0;
    for( int i = 0; i < sizeV ; i ++ ){
        en += V[MaxScale-1][i]*V[MaxScale-1][i] * qPow(sqrt(2.)/normFactor,2*MaxScale-2);
    }
    energy.push_back(en);
    TotalEnergy += en;

    //qDebug() << "TotalEnergy 0 : " << TotalEnergy0;
    //qDebug() << energy0;
    //qDebug() << "TotalEnergy   : " << TotalEnergy;
    //qDebug() << energy;

    if( originEnergyFlag ){
        //TotalEnergy0 = TotalEnergy;
        energy0 = energy;
    }
}

void WaveletAnalysis::HardThresholding(double threshold, int octave, int type)
{
    if( octave == 0 ) return;
    if( threshold <= 0.0) return;
    for( int s = 1 ;  s <= qAbs(octave) ; s++){
        for( int k = 0; k < W[s].size(); k++){
            if( type == 0 && qAbs(W[s][k]) < threshold ) {
                W[s][k] = 0.;
            }
            if( type == 1 && qAbs(W[s][k]) < threshold*(1 - (s-1.0)*1.0/qAbs(octave)) ) {
                W[s][k] = 0.;
            }
        }
    }
    msg.sprintf("Applied Hard Thresholding, level<%2.1f && scale>%d", threshold, octave);
}

void WaveletAnalysis::SoftThresholding(double threshold, int octave)
{
    if( octave == 0 ) return;
    if( threshold <= 0.0) return;

    for( int s = 1 ;  s <= qAbs(octave) ; s++){
        for( int k = 0; k < W[s].size(); k++){
            if( qAbs(W[s][k]) < threshold ) {
                W[s][k] = 0.;
            }else{
                double temp = W[s][k];
                if( temp > 0){
                    W[s][k] =  WAbsMax * (qAbs(temp)-threshold) / (WAbsMax - threshold);
                }else{
                    W[s][k] =  -WAbsMax * (qAbs(temp)-threshold) / (WAbsMax - threshold);
                }
            }
        }
    }
    msg.sprintf("Applied Soft Thresholding, level<%2.1f && scale>%d", threshold, octave);

}

void WaveletAnalysis::CleanOutsider(double x1, double x2, int octave)
{
    // is incorrect
    //return;
    if( octave == 0 ) return;
    for( int s = 1 ;  s <= qAbs(octave) ; s++){
        for( int k = 0; k <= Wk[s].size(); k++){
            int j = Wk[s][k];
            if( j < 0 || j > X0[s].size()) continue;
            if( X0[s][j] < x1 || X0[s][j] > x2 ) {
                W[s][k] = 0.;
            }
        }
    }
}

void WaveletAnalysis::PrintArray(QVector<double> y, QString str, int octave)
{
    qDebug() << str << "(" << octave << "," << y.size() <<")" << y;
}

void WaveletAnalysis::SaveCoefficients(QString fileName, int yIndex)
{
    QFile savefile(fileName);

    savefile.open(QIODevice::WriteOnly);

    QTextStream stream(&savefile);

    QString lineout, tmp;

    lineout.sprintf("yIndex = %4d, Total Number of Data : %5d; %12s-%02d\n", yIndex, this->size, this->waveletName.toStdString().c_str(), this->waveletPar);
    stream << lineout;

    for( int s = 1; s < MaxScale ; s++){
        int size = Wk[s].size();
        lineout.sprintf("   Wk[%2d], %4d, ", s, size);
        for(int k = 0; k < size ; k++){
            tmp.sprintf("%10d, ", Wk[s][k]);
            lineout += tmp;
        }
        stream << lineout;
        stream << "\n";

        size = W0[s].size();
        lineout.sprintf("    W[%2d], %4d, ", s, size);
        for(int k = 0; k < size ; k++){
            tmp.sprintf("%10.5f, ", W0[s][k]);
            lineout += tmp;
        }
        stream << lineout;
        stream << "\n";

    }

    int size = Vk[MaxScale-1].size();
    lineout.sprintf("   Vk[%2d], %4d, ", MaxScale-1, size);
    for(int k = 0; k < size ; k++){
        tmp.sprintf("%10d, ", Vk[MaxScale-1][k]);
        lineout += tmp;
    }
    stream << lineout;
    stream << "\n";

    size = V0[MaxScale-1].size();
    lineout.sprintf("    V[%2d], %4d, ", MaxScale-1, size);
    for(int k = 0; k < size ; k++){
        tmp.sprintf("%10.5f, ", V0[MaxScale-1][k]);
        lineout += tmp;
    }
    stream << lineout;
    stream << "\n";

    stream << "==================== Origin Data \n";

    size = Vk[0].size();
    lineout.sprintf("   Vk[%2d], %4d, ", 0, size);
    for(int k = 0; k < size ; k++){
        tmp.sprintf("%10d, ", Vk[0][k]);
        lineout += tmp;
    }
    stream << lineout;
    stream << "\n";

    size = X0[0].size();
    lineout.sprintf("   X0[%2d], %4d, ", 0, size);
    for(int k = 0; k < size ; k++){
        tmp.sprintf("%10.4f, ", X0[0][k]);
        lineout += tmp;
    }
    stream << lineout;
    stream << "\n";

    size = V0[0].size();
    lineout.sprintf("    V[%2d], %4d, ", 0, size);
    for(int k = 0; k < size ; k++){
        tmp.sprintf("%10.5f, ", V0[0][k]);
        lineout += tmp;
    }
    stream << lineout;
    stream << "\n";

    stream << "=============== End of File.\n";

    savefile.close();
}
