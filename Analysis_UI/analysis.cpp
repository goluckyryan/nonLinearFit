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

    connect(&sol, SIGNAL(SendMsg(QString)), this, SLOT(MsgConnector(QString)));
    connect(&dpar, SIGNAL(SendMsg(QString)), this, SLOT(MsgConnector(QString)));
    connect(&error, SIGNAL(SendMsg(QString)), this, SLOT(MsgConnector(QString)));
    connect(&tDis, SIGNAL(SendMsg(QString)), this, SLOT(MsgConnector(QString)));
    connect(&pValue, SIGNAL(SendMsg(QString)), this, SLOT(MsgConnector(QString)));
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

    Msg.sprintf("%d, %d, %d", xStart, xEnd, fitSize);
    SendMsg(Msg);

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
        CoVar = FtF.Inverse();  //printf(" CoVar(%d,%d)\n", CoVar.GetRows(), CoVar.GetCols());
    }catch( Exception err){
        fitFlag = 1; // set the p-Value to be 9999;
        return 1;
    }

    CoVar.PrintM("CoVar");


    Matrix dY = Y - f;    //printf("    dY(%d,%d)\n", dY.GetRows(), dY.GetCols());
    Matrix FtdY = Ft*dY;  //printf("  FtdY(%d,%d)\n", FtdY.GetRows(), FtdY.GetCols());

    Matrix par_old(1,p);
    par_old(1,1) = par[0];
    par_old(1,2) = par[1];
    if( fitType ) par_old(1,3) = par[2];
    if( fitType ) par_old(1,4) = par[3];

    this->dpar = (CoVar * FtdY).Transpose();  //printf("  dpar(%d,%d)\n", dpar.GetRows(), dpar.GetCols());
    this->sol = par_old + this->dpar;
    this->SSR = (dY.Transpose() * dY)(1,1);
    double fitVar = this->SSR / this->DF;

    this->error  = Matrix(1,p); for( int i = 1; i <= p ; i++){ error(1,i)  = sqrt(fitVar * CoVar(i,i)); }
    this->tDis   = Matrix(1,p); for( int i = 1; i <= p ; i++){ tDis(1,i)   = this->sol(1,i)/this->error(1,i); }
    this->pValue = Matrix(1,p); for( int i = 1; i <= p ; i++){ pValue(1,i) = cum_tDis30(- std::abs(this->tDis(1,i)));}

    par_old.PrintVector("in(par)");
    this->dpar.PrintVector("dpar");
    this->sol.PrintVector("sol");

    qDebug()<< "======================================";
}

Matrix *Analysis::NonLinearFit(int startFitIndex, QVector<double> iniPar)
{

}

void Analysis::CalFitData(QVector<double> par){
    for ( int i = 0; i < this->n; i++){
        double x = xdata[i];
        fydata.push_back( FitFunc(1,x, par) );
    }
}

void Analysis::Print()
{
    qDebug("Data size : %d", n);
    qDebug("par size : %d", p);
    qDebug("DF : %d", DF);
    qDebug("Start Fit Index : %d", startIndex);
    qDebug("mean : %f", mean);
    qDebug("variance : %f", var);
    qDebug("SSR : %f", SSR);
    qDebug("Is fit ? %d", fitFlag);

    sol.PrintVector("sol:");
    dpar.PrintVector("dpar:");
    error.PrintVector("error:");
    tDis.PrintVector("tDis:");
    pValue.PrintVector("p-Value:");
}
