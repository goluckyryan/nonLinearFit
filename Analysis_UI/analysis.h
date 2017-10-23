#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <QObject>
#include <QVector>
#include <QString>
#include <cmath>
#include <math.h>
#include <QCoreApplication>
#include <QDir>
#include <QScriptValue>
#include <QScriptEngine>
#include <QTimer>
#include "constant.h"
#include "matrix.h"

class Analysis : public QObject
{
    Q_OBJECT
public:

    explicit Analysis();
    explicit Analysis(const QVector<double> x, const QVector<double> y);
    ~Analysis();

    void Initialize();

    void setFunctionType(int fitFuncID);
    void setFunctionExpression(QString str);
    void setFunctionGradExpression(QStringList str_list);
    void setEngineParameter(QVector<double> par);

    void SetY(int yIndex, double Bfield){ this->yIndex = yIndex; this->yValue = Bfield;}
    void SetData(const QVector<double> x, const QVector<double> y);
    void SetStartFitIndex(int index){ this->startIndex = index;}
    void SetEndFitIndex(int index){ this->endIndex = index;}
    void Setlambda(double l){this->lambda = l;}
    void SetTORR(double x){ torr = x;}
    void SetMaxInteration(int i){this->MaxIter = i;}
    QVector<double> MeanAndvariance(int index_1, int index_2);

    int Regression(QVector<double> par0);
    int LMA(QVector<double> par0, double lambda0 = 0.01);
    int GnuFit(QVector<double> par);
    int NonLinearFit(QVector<double> par0, bool gnufit);

    void CalFitData(QVector<double> par);

signals:

    void SendMsg(QString msg);

public slots:
    int GetFitFlag(){return fitFlag;}
    QString GetFitMsg(){return fitMsg;}
    int GetDataSize(){ return n;}
    int GetStartFitIndex(){return startIndex;}
    int GetEndFitIndex(){return endIndex;}
    int GetStart_x(){return xdata[startIndex];}
    int GetParametersSize() {return p;}
    int GetNDF() {return DF;}
    QVector<double> GetData_x() {return xdata;}
    QVector<double> GetData_y() {return zdata;}
    double GetData_x(int i) {return xdata[i-1];}
    double GetData_y(int i) {return zdata[i-1];}

    QVector<double> GetFitData_y() {return fydata;}

    double GetSSR() {return SSR;}
    double GetFitVariance() {return SSR/DF;}

    QVector<double> GetParameters() {return sol;}
    QVector<double> GetParError() {return error;}
    QVector<double> GetParPValue() {return pValue;}
    QVector<double> GetSSRgrad() {return gradSSR;}

    int GetNIteration(){return nIter;}
    double GetParameters(int i) {return sol[i];}
    double GetParError(int i) {return error[i];}
    double GetParPValue(int i) {return pValue[i];}

    double GetSampleVariance(){return var;}
    double GetSampleMean(){return mean;}

    double GetDelta(){return delta;}
    double GetLambda(){return lambda;}
    int GetMaxIteration(){return MaxIter;}
    int GetYIndex()    {return yIndex;}
    double GetDataYValue() {return yValue;}
    double GetDataYMin(){return zMin;}
    double GetDataYMax(){return zMax;}
    double GetDataYMinIndex(){return zMinIndex;}
    double GetDataYMaxIndex(){return zMaxIndex;}

    void Print();
    void PrintVector(QVector<double> vec, QString str);
    void PrintMatrix(Matrix mat, QString str);
    void PrintCoVarMatrix();

    int FindXIndex(double goal);
    double FindXFromYAfterTZero(double y);

private:
    QString Msg;
    QString fitMsg;
    QString function_str; // for custom function
    QStringList function_grad_str;
    QScriptEngine engine;

    QScriptValue cumstomFunction;
    QVector<QScriptValue> cumstomFunGrad;

    QVector<double> xdata, zdata;
    QVector<double> fydata;
    QVector<double> sol;
    QVector<double> dpar;
    QVector<double> error;
    QVector<double> pValue;
    QVector<double> gradSSR;

    int fitFuncID;

    Matrix CoVar;

    double zMin, zMax;
    int zMinIndex, zMaxIndex;

    int MaxIter, nIter;
    double SSR;
    int n, p, DF;
    int startIndex, endIndex;
    int fitFlag;
    double delta; // SSR(p+h) - SSR(p)
    double mean, var; // sample
    double lambda;
    double torr;

    int yIndex;
    double yValue;

    double cum_tDis30(double x){
        return 1/(1+exp(-x/0.6));
    }

    double FitFunc(double x, QVector<double> par, int fitFuncID){
        double fit = 0;
        if(fitFuncID == 0){
            fit = par[0] * exp(-x/par[1]);          // when a*Exp[t/Ta]
            if( par.size() == 3) fit += par[2];     // when a*Exp[t/Ta] + c
            if( par.size() == 4) fit += par[2] * exp(-x/par[3]); // when a*Exp[t/Ta] + b*Exp[t/Tb]
            if( par.size() == 5) fit += par[2] * exp(-x/par[3]) + par[4];     // when a*Exp[t/Ta] + b*Exp[t/Ta] + c
        }

        if(fitFuncID == 1){
//            fit = par[0] * exp(-x/par[1]);          // when a*Exp[t/Ta]
//            if( par.size() == 3) fit += par[2];     // when a*Exp[t/Ta] + c
//            if( par.size() == 4) fit = par[0] * exp(-x/par[1]) * sin(2*PI*x/par[3]) + par[3]; // when a*Exp[t/Ta]*sin[2 pi t/Tb] + c
            fit = par[0] * exp(-x/par[1]) * sin(par[2] + 2* PI *x/par[3] ) + par[4];     // when a*Exp[t/Ta] + b*Exp[t/Ta] + c
        }

        if(fitFuncID == 2){

            engine.globalObject().setProperty("x", x);
            fit = engine.evaluate(function_str).toNumber();

            //qDebug() << x  << ", " << fit;
        }

        return fit;
    }

    QVector<double> GradFitFunc(double x, QVector<double> par, int fitFuncID){
        QVector<double> gradFit;

        if( fitFuncID == 0){
            gradFit.push_back(exp(-x/par[1]));
            gradFit.push_back(par[0] * x* exp(-x/par[1])/par[1]/par[1]);
            if(par.size() == 3) gradFit.push_back(1); // in case of c

            if( par.size() == 4){
                gradFit.push_back(exp(-x/par[3]));
                gradFit.push_back(par[2] * x* exp(-x/par[3])/par[3]/par[3]);
            }
            if( par.size() == 5) {
                gradFit.push_back(exp(-x/par[3]));
                gradFit.push_back(par[2] * x* exp(-x/par[3])/par[3]/par[3]);
                gradFit.push_back(1); // in case of c
            }
        }

        if( fitFuncID == 1){
            gradFit.push_back(exp(-x/par[1])*sin(par[2] + 2 * PI * x/par[3] ) );
            gradFit.push_back(par[0] * x * exp(-x/par[1]) * sin(2*PI*x/par[3] + par[2]) / par[1] /par[1]);
            gradFit.push_back(par[0] * exp(-x/par[1]) * cos(2*PI*x/par[3] + par[2]));
            gradFit.push_back((-2) * PI * x * par[0] * exp(-x/par[1]) * cos(2*PI*x/par[3] + par[2])/par[3]/par[3]);
            gradFit.push_back(1);
        }

        if( fitFuncID == 2){

            engine.globalObject().setProperty("x", x);

            for( int p = 0; p < par.size() ; p++ ){
                QString gradFunction = function_grad_str.at(p);
                double fit = engine.evaluate(gradFunction).toNumber();
                gradFit.push_back(fit);

                //qDebug() << x << "," << gradFunction << ", " << fit;
            }

            //QScriptValueList args;
            //args << x;
            //for( int i = 0; i < par.size() ; i++ ){
            //    args << i;
            //}
            //
            //for( int p = 0; p < par.size() ; p++ ){
            //    QString gradFunction = function_grad_str.at(p);
            //    QScriptEngine expression;
            //    QScriptValue fun = expression.evaluate(gradFunction);
            //    QScriptValue ans = fun.call(QScriptValue(), args);
            //    gradFit.push_back(ans.toNumber());
            //
            //    //Why the ans becomes nan for finite par?
            //    qDebug() << x << "," << gradFunction << ", " << ans.toNumber();
            //}

            //qDebug() << x << gradFit;

        }
        return gradFit;
    }

    QVector<double> RowVector2QVector(Matrix m){
        QVector<double> out;
        for(int i = 1; i <= m.GetRows(); i++){
            out.push_back(m(i,1));
        }
        return out;
    }

};

#endif // ANALYSIS_H
