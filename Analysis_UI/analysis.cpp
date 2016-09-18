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
    this->SSR = 999;
    this->n = 0;
    this->p = 0;
    this->DF = 0;
    this->fitFlag = 0;
    this->mean = 0;
    this->var = 0;

    /*
    connect(&sol, SIGNAL(SendMsg(QString)), this, SLOT(MsgConnector(QString)));
    connect(&dpar, SIGNAL(SendMsg(QString)), this, SLOT(MsgConnector(QString)));
    connect(&error, SIGNAL(SendMsg(QString)), this, SLOT(MsgConnector(QString)));
    connect(&tDis, SIGNAL(SendMsg(QString)), this, SLOT(MsgConnector(QString)));
    connect(&pValue, SIGNAL(SendMsg(QString)), this, SLOT(MsgConnector(QString)));
    */
}

void Analysis::SetData(const QVector<double> x, const QVector<double> y)
{
    // assume x.size() == y.size()
    if( x.size() != y.size() ){
        Msg.sprintf("The sizes of input data x and y are not match. Size(x) = %d, Size(y) = %d", x.size(), y.size());
        SendMsg(Msg);
        return;
    }
    this->xdata = x;
    this->ydata = y;

    this->n = this->xdata.size();
    Msg.sprintf("Input Data, size = %d", this->n);
    SendMsg(Msg);

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

    Msg.sprintf("Mean from index %d to %d of y-data (%d data) = %f", index_1, index_2, size, mean);
    emit SendMsg(Msg);

    var = 0;
    for( int i = index_1 ; i <= index_2 ; i++){
        var += pow((this->ydata)[i]-mean,2);
    }
    var = var / (size-1);

    Msg.sprintf("Variance from index %d to %d of y-data (%d data) = %f, sigma = %f", index_1, index_2, size, var, sqrt(var));
    emit SendMsg(Msg);

}

int Analysis::Regression(bool fitType, QVector<double> par)
{
    int xStart = this->startIndex;
    int xEnd = this->n - 1;
    int fitSize = xEnd - xStart + 1;

    //Msg.sprintf("%d, %d, %d", xStart, xEnd, fitSize);
    //SendMsg(Msg);

    this->p = 2;//default is 2 parameters fit
    if (fitType) this->p = 4;

    this->DF = fitSize - this->p;

    //============================Start regression
    Matrix Y(fitSize,1);
    for(int i = 1; i <= fitSize ; i++) {
        Y(i,1) = ydata[i + xStart - 1];
    }

    ///qDebug("Y : %d %d", Y.GetRows(), Y.GetCols());
    ///qDebug("Y : %f ... %f" , Y(1,1), Y(fitSize, 1));

    Matrix f(fitSize,1);
    for(int i = 1; i <= fitSize ; i++) {
        double x = xdata[i + xStart - 1];
        f(i,1) = FitFunc(fitType, x, par);
    }

    ///qDebug("f : %d %d", f.GetRows(), f.GetCols());
    ///qDebug("f : %f ... %f" , f(1,1), f(fitSize, 1));


    Matrix F(fitSize,p); // F = grad(f)
    for(int i = 1; i <= fitSize ; i++) {
        double x = xdata[i - 1 + xStart];

        QVector<double> gradf = GradFitFunc(fitType, x, par);

        for(int j = 1; j <= p; j++){
            F(i,j) = gradf[j-1];
        }

        //F(i,1) = exp(-x/par[1]);
        //F(i,2) = par[0] * x * exp(-x/par[1])/par[1]/par[1];
        //if( fitType ) F(i,3) = exp(-x/par[3]);
        //if( fitType ) F(i,4) = par[2] * x * exp(-x/par[3])/par[3]/par[3];
    }

    ///qDebug("F  : %d %d", F.GetRows(), F.GetCols());
    ///qDebug("F1 : %f ... %f" , F(1,1), F(fitSize, 1));
    ///qDebug("F2 : %f ... %f" , F(1,2), F(fitSize, 2));

    Matrix Ft = F.Transpose();

    ///qDebug("Ft : %d %d", Ft.GetRows(), Ft.GetCols());
    ///qDebug("F1 : %f ... %f" , Ft(1,1), Ft(1,fitSize));
    ///qDebug("F2 : %f ... %f" , Ft(2,1), Ft(2,fitSize));

    Matrix FtF = Ft*F;

    ///FtF.PrintM("FtF");

    Matrix CoVar;
    try{
        CoVar = FtF.Inverse();
    }catch( Exception err){
        fitFlag = 1;
        return 1; // return 1 when covariance cannot be compute.
    }

    ///CoVar.PrintM("CoVar");

    Matrix dY = f-Y;
    Matrix FtdY = Ft*dY;

    ///FtdY.PrintVector("FtdY");

    Matrix par_old(p,1);
    par_old(1,1) = par[0];
    par_old(2,1) = par[1];
    if( fitType ) par_old(3,1) = par[2];
    if( fitType ) par_old(4,1) = par[3];

    Matrix dpar = CoVar * FtdY;
    Matrix sol = par_old + dpar;
    this->SSR = (dY.Transpose() * dY)(1,1);
    double fitVar = this->SSR / this->DF;

    Matrix error(p,1);  for( int i = 1; i <= p ; i++){ error(i,1)  = sqrt(fitVar * CoVar(i,i)); }
    Matrix tDis(p,1);   for( int i = 1; i <= p ; i++){ tDis(i,1)   = sol(i,1)/error(i,1); }
    Matrix pValue(p,1); for( int i = 1; i <= p ; i++){ pValue(i,1) = cum_tDis30(- std::abs(tDis(i,1)));}

    this->sol = sol.Matrix2QVec();
    this->dpar = dpar.Matrix2QVec();
    this->error = error.Matrix2QVec();
    this->tDis = tDis.Matrix2QVec();
    this->pValue = pValue.Matrix2QVec();

    ///PrintVector(this->sol, "sol:");
    ///PrintVector(this->dpar, "dpar:");

    //qDebug()<< "======================================";

    return 0;
}

int Analysis::NonLinearFit(QVector<double> iniPar)
{
    const int nIter = 40;
    const double torr = 0.01;
    const double CL = 0.05;
    QString tmp;

    // do 4-parameter fit first.
    int count = 0;
    QVector<double> par = iniPar;
    bool contFlag;
    Msg.sprintf(" === Start 4-parameters fit : ");
    do{
        Regression(1, par);
        par = this->sol;
        count ++;
        tmp.sprintf(" %d", count);
        Msg += tmp;
        if( count > nIter ) {
            fitFlag = 2; // fitFlag = 2 when iteration too many
            break;
        }
        bool converge = 0;
        //since this is 4-parameter fit
        converge = std::abs(dpar[0]) < torr &&
                   std::abs(dpar[1]) < torr &&
                   std::abs(dpar[2]) < torr &&
                   std::abs(dpar[3]) < torr;
        contFlag = fitFlag == 0 && ( !converge );
    }while(contFlag);

    //Check pValue
    bool rej = 0;
    ///rej = pValue[0] > CL || pValue[1] > CL || pValue[2] > CL || pValue[3] > CL;
    rej = pValue[0] > CL || pValue[2] > CL;
    if( rej) fitFlag == 3;

    //
    if( fitFlag == 0) {
        Msg += "| End Normally.";
    }else if(fitFlag == 1){
        Msg += "| Terminated. Covariance fail to cal.";
    }else if(fitFlag == 2){
        tmp.sprintf("| Terminated. Fail to converge in %d trials.", nIter);
        Msg += tmp;
    }else if(fitFlag == 3){
        tmp.sprintf("| End Normally. Result rejected. p(a) = %f, p(b) = %f", pValue[0], pValue[1] );
        Msg += tmp;
    }
    qDebug() << Msg;

    if( fitFlag){
        PrintVector(sol, "sol:");
        PrintVector(dpar, "dpar");
        PrintVector(error, "error");
        PrintVector(pValue, "pValue");
    }

    if( fitFlag ) qDebug() << " ========== User should try other inipar before trusting the 2-parameter fit !!!!!!!!";

    if( fitFlag == 0) return 0;

    count = 0;
    par.clear();
    par.push_back(iniPar[0]);
    par.push_back(iniPar[1]);
    Msg.sprintf(" === Start 2-parameters fit : ");
    do{
        Regression(0, par);
        par = this->sol;
        count ++;
        tmp.sprintf(" %d", count);
        Msg += tmp;
        if( count > nIter ) {
            fitFlag = 2; // fitFlag = 2 when iteration too many
            break;
        }
        bool converge = 0;
        //since this is 4-parameter fit
        converge = std::abs(dpar[0]) < torr &&
                   std::abs(dpar[1]) < torr;
        contFlag = fitFlag == 0 && ( !converge );
    }while(contFlag);

    //Check pValue
    rej = 0;
    ///rej = pValue[0] > CL || pValue[1] > CL ;
    rej = pValue[0] > CL;
    if( rej) fitFlag == 3;

    //
    if( fitFlag == 0) {
        Msg += "| End Normally.";
    }else if(fitFlag == 1){
        Msg += "| Terminated. Covariance fail to cal.";
    }else if(fitFlag == 2){
        tmp.sprintf("| Terminated. Fail to converge in %d trials.", nIter);
        Msg += tmp;
    }else if(fitFlag == 3){
        tmp.sprintf("| End Normally. Result rejected. p(a) = %f", pValue[0]);
        Msg += tmp;
    }
    qDebug() << Msg;


    return 0;
}

void Analysis::CalFitData(QVector<double> par){
    for ( int i = 0; i < this->n; i++){
        double x = xdata[i];
        fydata.push_back( FitFunc(1,x, par) );
    }
}

void Analysis::Print()
{
    qDebug() << "======= Ana ==========";
    qDebug("Data size : %d", n);
    qDebug("par size : %d", p);
    qDebug("DF : %d", DF);
    qDebug("Start Fit Index : %d", startIndex);
    qDebug("mean : %f", mean);
    qDebug("variance : %f", var);
    qDebug("SSR : %f", SSR);
    qDebug("Is fit ? %d", fitFlag);

    PrintVector(sol, "sol:");
    PrintVector(dpar, "dpar:");
    PrintVector(error, "error:");
    PrintVector(tDis, "tDis:");
    PrintVector(pValue, "p-Value:");

    qDebug()<< "======== End of Ana =========";
}

void Analysis::PrintVector(QVector<double> vec, QString str)
{
    QString tmp;
    Msg.sprintf("%*s(%d) : [ ", 15, str.toStdString().c_str(), vec.size());
    for(int i = 0; i < vec.size() - 1; i++){
        tmp.sprintf(" %7.3f,", vec[i]);
        Msg += tmp;
    }
    tmp.sprintf(" %7.3f]", vec[vec.size()-1]);
    Msg += tmp;
    qDebug() << Msg;
}
