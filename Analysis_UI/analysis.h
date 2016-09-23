#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <QObject>
#include <QVector>
#include <QString>
#include <cmath>
#include <math.h>
#include <QCoreApplication>
#include <QDir>
#include "matrix.h"

class Analysis : public QObject
{
    Q_OBJECT
public:
    QString Msg;

    explicit Analysis();
    explicit Analysis(const QVector<double> x, const QVector<double> y);
    ~Analysis();

    void Initialize();

    void SetY(int yIndex, double Bfield){ this->yIndex = yIndex; this->yValue = Bfield;}
    void SetData(const QVector<double> x, const QVector<double> y);
    void SetStartFitIndex(int index){ this->startIndex = index;}
    void Setlambda(double l){this->lambda = l;}
    void SetMaxInteration(int i){this->MaxIter = i;}
    void MeanAndvariance(int index_1, int index_2);

    int Regression(QVector<double> par0);
    int LMA(QVector<double> par0, double lambda0 = 0.01);
    int GnuFit(QVector<double> par);
    int NonLinearFit(QVector<double> par0, bool gnufit);

    void CalFitData(QVector<double> par);


signals:

    void SendMsg(QString msg);

public slots:
    int GetFitFlag(){return fitFlag;}
    int GetDataSize(){ return n;}
    int GetStartFitIndex(){return startIndex;}
    int GetStart_x(){return xdata[startIndex];}
    int GetParametersSize() {return p;}
    int GetNDF() {return DF;}
    QVector<double> GetData_x() {return xdata;}
    QVector<double> GetData_y() {return ydata;}
    double GetData_x(int i) {return xdata[i-1];}
    double GetData_y(int i) {return ydata[i-1];}

    QVector<double> GetFitData_y() {return fydata;}

    double GetSSR() {return SSR;}
    double GetFitVariance() {return SSR/DF;}

    QVector<double> GetParameters() {return sol;}
    QVector<double> GetParError() {return error;}
    QVector<double> GetParPValue() {return pValue;}
    QVector<double> GetSSRgrad() {return gradSSR;}

    double GetParameters(int i) {return sol[i];}
    double GetParError(int i) {return error[i];}
    double GetParPValue(int i) {return pValue[i];}

    double GetSampleVariance(){return var;}
    double GetSampleMean(){return mean;}

    double GetDelta(){return delta;}
    double GetLambda(){return lambda;}
    int GetMaxIteration(){return MaxIter;}

    int GetYIndex() {return yIndex;}
    double GetBValue() {return yValue;}

    void Print();
    void PrintVector(QVector<double> vec, QString str);

    void MsgConnector(QString msg){
        emit SendMsg(msg);
    }

private:
    QVector<double> xdata, ydata;
    QVector<double> fydata;
    QVector<double> sol;
    QVector<double> dpar;
    QVector<double> error;
    QVector<double> pValue;
    QVector<double> gradSSR;

    Matrix CoVar;

    int MaxIter;
    double SSR;
    int n, p, DF;
    int startIndex;
    int fitFlag;
    double delta; // SSR(p+h) - SSR(p)
    double mean, var; // sample
    double lambda;

    int yIndex;
    double yValue;

    double cum_tDis30(double x){
        return 1/(1+exp(-x/0.6));
    }

    double FitFunc(double x, QVector<double> par){
        double fit = 0;
        fit = par[0] * exp(-x/par[1]) + par[2] * exp(-x/par[3]);
        return fit;
    }

    QVector<double> GradFitFunc(double x, QVector<double> par){
        QVector<double> gradFit;

        gradFit.push_back(exp(-x/par[1]));
        gradFit.push_back(par[0] * x* exp(-x/par[1])/par[1]/par[1]);
        gradFit.push_back(exp(-x/par[3]));
        gradFit.push_back(par[2] * x* exp(-x/par[3])/par[3]/par[3]);

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
