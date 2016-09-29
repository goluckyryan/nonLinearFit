#include "analysis.h"

Analysis::Analysis(){
    Initialize();
}

Analysis::Analysis(const QVector<double> x, const QVector<double> y)
{
    Initialize();
    SetData(x,y);
}

Analysis::~Analysis(){

}

void Analysis::Initialize(){
    SSR = 0;
    lastSSR = 0;
    n = 0;
    p = 0;
    DF = 0;
    fitFlag = 0;
    mean = 0;
    var = 0;

    delta = 0;
    lambda = 0;

    yIndex = 0;
    yValue = 0;
}

void Analysis::SetData(const QVector<double> x, const QVector<double> y)
{
    if( x.size() != y.size() ){
        Msg.sprintf("The sizes of input data x and y are not match. Size(x) = %d, Size(y) = %d", x.size(), y.size());
        SendMsg(Msg);
        return;
    }
    this->xdata = x;
    this->ydata = y;

    this->n = this->xdata.size();
}

void Analysis::MeanAndvariance(int index_1, int index_2)
{
    if( this->n == 0 ||
        index_1 < 0 ||
        index_2 < 0 ||
        index_1 > this->n ||
        index_1 >= index_2 ||
        index_2 > this->n){
        Msg.sprintf("index Error. n = %d, range = (%d, %d)", n, index_1, index_2);
        emit SendMsg(Msg);
        return;
    }

    int size = index_2 - index_1 + 1;
    mean = 0;
    for( int i = index_1 ; i <= index_2 ; i++){
        mean += (this->ydata)[i];
    }
    mean = mean / size;

    var = 0;
    for( int i = index_1 ; i <= index_2 ; i++){
        var += pow((this->ydata)[i]-mean,2);
    }
    var = var / (size-1);

    Msg.sprintf("From index %d to %d (%d data)\nMean = %f, Variance = %f, sigma = %f", index_1, index_2, size, mean, var, sqrt(var));
    SendMsg(Msg);

}

int Analysis::Regression(QVector<double> par0)
{
    //Levenberg-Marquardt Algorithm
    fitFlag = 0;
    int xStart = this->startIndex;
    int xEnd = this->n - 1;
    int fitSize = xEnd - xStart + 1;

    this->p = par0.size();
    this->DF = fitSize - this->p;
    Matrix par_old(p,1); for(int i = 0; i < p; i++){ par_old(i+1,1) = par0[i];}

    //============================Start regression
    Matrix Y(fitSize,1);
    Matrix f(fitSize,1);
    Matrix F(fitSize,p); // F = grad(f)
    for(int i = 1; i <= fitSize ; i++) {
        Y(i,1) = ydata[i + xStart - 1];
        double x = xdata[i + xStart - 1];
        f(i,1) = FitFunc(x, par0);
        QVector<double> gradf = GradFitFunc(x, par0);
        for(int j = 1; j <= p; j++){
            F(i,j) = gradf[j-1];
        }
    }

    Matrix Ft = F.Transpose();
    Matrix FtF = Ft*F;
    Matrix D(p,p);
    for(int i = 1; i <= p ; i++) {
        D(i,i) = this->lambda;
        //D(i,i) = this->lambda * FtF(i,i); //alternative
    }

    try{
        this->CoVar = (FtF + D).Inverse();
    }catch( Exception err){
        fitFlag = 1;
        return 0; // return 1 when covariance cannot be compute.
    }

    Matrix dY = f-Y;
    this->SSR = (dY.Transpose() * dY)(1,1);

    Matrix FtdY = Ft*dY; // gradient of SSR
    Matrix dpar = CoVar * FtdY;
    Matrix sol = par_old + dpar;

    //========================================== Check the SSR(p+dpar)
    QVector<double> par_new = sol.Matrix2QVec();
    Matrix fn(fitSize,1);
    for(int i = 1; i <= fitSize ; i++) {
        double x = xdata[i + xStart - 1];
        fn(i,1) = FitFunc(x, par_new);
    }
    Matrix dYn = fn-Y;
    double SSRn = (dYn.Transpose() * dYn)(1,1);

    this->delta = SSRn - this->SSR;

    //========== SSRn > SSR
    if( this->delta >= 0){
        this->lambda = this->lambda / 10;
        sol = par_old;
        this->sol = sol.Matrix2QVec();
        this->dpar = Matrix(p,1).Matrix2QVec(); ///Zero matrix
        this->gradSSR = FtdY.Matrix2QVec();
        return 0;
    }
    //========== SSRn < SSR

    this->lambda = this->lambda * 10;
    this->sol = sol.Matrix2QVec();
    this->dpar = dpar.Matrix2QVec();

    F(fitSize,p); // F = grad(f)
    for(int i = 1; i <= fitSize ; i++) {
        double x = xdata[i - 1 + xStart];
        QVector<double> gradf = GradFitFunc(x, this->sol);
        for(int j = 1; j <= p; j++){
            F(i,j) = gradf[j-1];
        }
    }
    //new gradient of SSR
    FtdY = F.Transpose()*dYn;
    this->gradSSR = FtdY.Matrix2QVec();

    try{
        this->CoVar = (F.Transpose()*F).Inverse();
    }catch( Exception err){
        fitFlag = 1;
        return 0; // return 0 when covariance cannot be compute.
    }

    return 1;
}

int Analysis::LMA( QVector<double> par0, double lambda0){

    this->lambda = lambda0;

    QString tmp;
    PrintVector(par0, "ini. par:");

    int count = 0;
    QVector<double> par = par0;

    bool contFlag;
    Msg.sprintf(" === Start fit using Levenberg-Marquardt Algorithm: ");
    do{
        count += Regression(par);
        par = this->sol;

        if( count >= MaxIter ) {
            fitFlag = 2; // fitFlag = 2 when iteration too many
            break;
        }
        bool converge = 0;
        //since this is 4-parameter fit
        converge = std::abs(this->delta) <  TORR;
        for(int i = 0; i < p; i++){
            converge &= std::abs(this->gradSSR[i]) < TORRGRAD;
        }
        // if lambda to small or too big, reset
        if( this->lambda < 1e-5) this->lambda = 1e+5;
        if( this->lambda > 1e+10) this->lambda = 1e-4;
        contFlag = fitFlag == 0 && ( !converge );

    }while(contFlag);

    tmp.sprintf(" %d", count);
    Msg += tmp;
    if( fitFlag == 0) {
        Msg += "| End Normally.";
    }else if(fitFlag == 1){
        Msg += "| Terminated. Covariance fail to cal.";
    }else if(fitFlag == 2){
        tmp.sprintf("| Terminated. Fail to converge in %d trials.", MaxIter);
        Msg += tmp;
    }
    //qDebug() << Msg;
    SendMsg(Msg);

    //===== cal error
    double fitVar = this->SSR / this->DF;
    Matrix error(p,1);
    for( int i = 1; i <= p ; i++){
        error(i,1)  = sqrt(fitVar * CoVar(i,i));
    }
    Matrix pValue(p,1);
    for( int i = 1; i <= p ; i++){
        double tDis = sol[i-1]/error(i,1);
        pValue(i,1) = cum_tDis30(- std::abs(tDis));
    }

    this->error = error.Matrix2QVec();
    this->pValue = pValue.Matrix2QVec();

    //PrintVector(this->sol, "sol:");
    //PrintVector(this->error, "error:");

    return 0;
}

int Analysis::GnuFit(QVector<double> par)
{   // using gnuplot to fit and read the gnufit.log
    // need the "text.dat", which is condensed from *.cvs

    PrintVector(par, "ini. par:");
    Msg.sprintf(" === Start fit using gnuplot, also Levenberg-Marquardt Algorithm. ");
    SendMsg(Msg);

    QString cmd;
    cmd.sprintf("gnuplot -e \"yIndex=%d;a=%f;Ta=%f;b=%f;Tb=%f;startX=%d\" gnuFit.gp", yIndex, par[0], par[1], par[2], par[3], startIndex);
    qDebug() << cmd.toStdString().c_str();
    QDir::setCurrent(OPENPATH); // OPENPATH in constant.h
    qDebug() << QDir::currentPath();
    system(cmd.toStdString().c_str());

    this->p = 4;
    this->sol.clear();
    this->error.clear();
    this->pValue.clear();
    this->gradSSR.clear();

    QString gnuFitLogPath = OPENPATH;
    gnuFitLogPath += "/gnufit.log";
    QFile fitlog(gnuFitLogPath);
    fitlog.open( QIODevice::ReadOnly);
    QTextStream stream(&fitlog);
    QString line, findstr;
    int pos;
    //Get fit parameter
    while(stream.readLineInto(&line)){
        //get SSR;
        findstr = "residuals :";
        pos = line.indexOf(findstr) +  findstr.length();
        if( pos > findstr.length()) {
            SSR = line.mid(pos,10).toDouble();
        }

        //get NDF;
        findstr = "(FIT_NDF)";
        pos = line.indexOf(findstr) + findstr.length();
        if( pos > findstr.length()) {
            pos = line.indexOf(":");
            DF = line.mid(pos+1, 10).toDouble();
        }

        //get delta;
        findstr = "iteration";
        pos = line.indexOf(findstr) + findstr.length();
        if( pos > findstr.length()) {
            pos = line.indexOf(":");
            delta = line.mid(pos+1, 20).toDouble();
        }

        //get a and error a;
        double a, ea;
        findstr = "=======================";
        pos = line.indexOf(findstr);
        if( pos > -1) {

            for( int j = 0; j < this->p; j++){
                stream.readLineInto(&line);

                pos = line.indexOf("=");
                a = line.mid(pos+1, 12).toDouble();

                pos = line.indexOf("+/-");
                ea = line.mid(pos+3, 8).toDouble();
                this->sol.push_back(a);
                this->error.push_back(ea);
                this->dpar.push_back(0);
                this->pValue.push_back(cum_tDis30(- std::abs(a/ea)));
            }

        }

    }

    if( this->sol.isEmpty() ) return 1;
    int xStart = this->startIndex;
    int xEnd = this->n - 1;
    int fitSize = xEnd - xStart + 1;
    Matrix Y(fitSize,1);
    for(int i = 1; i <= fitSize ; i++) {
        Y(i,1) = ydata[i + xStart - 1];
    }

    Matrix fn(fitSize,1);
    for(int i = 1; i <= fitSize ; i++) {
        double x = xdata[i + xStart - 1];
        fn(i,1) = FitFunc(x, this->sol);
    }
    Matrix dYn = fn-Y;

    Matrix F(fitSize,p); // F = grad(f)
    for(int i = 1; i <= fitSize ; i++) {
        double x = xdata[i - 1 + xStart];
        QVector<double> gradf = GradFitFunc(x, this->sol);
        for(int j = 1; j <= p; j++){
            F(i,j) = gradf[j-1];
        }
    }
    //new gradient of SSR
    Matrix FtdY = F.Transpose()*dYn;
    this->gradSSR = FtdY.Matrix2QVec();

    return 0;
}

int Analysis::NonLinearFit(QVector<double> par0, bool gnufit)
{
    if( !gnufit){
        LMA(par0, this->lambda);
    }else{
        GnuFit(par0);
    }
    //Print();

    return 0;
}

void Analysis::CalFitData(QVector<double> par){
    fydata.clear();
    for ( int i = 0; i < this->n; i++){
        double x = xdata[i];
        fydata.push_back( FitFunc(x, par) );
    }
}

void Analysis::Print()
{
    qDebug() << "======= Ana ==========";
    qDebug("yIndex : %d, yValue : %f", yIndex, yValue);
    qDebug("Data size : %d", n);
    qDebug("par size : %d", p);
    qDebug("Start Fit Index : %d", startIndex);
    qDebug("DF : %d", DF);
    qDebug("Sample mean : %f", mean);
    qDebug("Sample variance : %f", var);
    qDebug("SSR : %f", SSR);
    qDebug("fitFlag : %d", fitFlag);
    qDebug("lambda : %f, delta : %f", lambda, delta);

    PrintVector(sol, "sol:");
    PrintVector(dpar, "dpar:");
    PrintVector(error, "error:");
    PrintVector(pValue, "p-Value:");
    PrintVector(gradSSR, "gradSSR:");

    qDebug()<< "======== End of Ana =========";
}

void Analysis::PrintVector(QVector<double> vec, QString str)
{
    QString tmp;
    tmp.sprintf("%*s(%d) : [ ", 15, str.toStdString().c_str(), vec.size());
    Msg = tmp;
    for(int i = 0; i < vec.size() - 1; i++){
        tmp.sprintf(" %7.3f,", vec[i]);
        Msg += tmp;
    }
    tmp.sprintf(" %7.3f]", vec[vec.size()-1]);
    Msg += tmp;

    SendMsg(Msg);

}
