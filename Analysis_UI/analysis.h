#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <QObject>
#include <QVector>
#include <QString>
#include <cmath>
#include <math.h>
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

    void SetData(const QVector<double> x, const QVector<double> y);
    void SetStartFitIndex(int index){ this->startIndex = index;}
    void MeanAndvariance(int index_1, int index_2);

    int Regression(bool fitType, QVector<double> par);
    Matrix *NonLinearFit(int startFitIndex, QVector<double> iniPar);

    void CalFitData(QVector<double> par);

signals:

    void SendMsg(QString msg);

public slots:
    bool IsWellFit(){return fitFlag;}
    int GetDataSize(){ return n;}
    int GetStartFitIndex(){return startIndex;}
    int GetStart_x(){return xdata[startIndex];}
    int GetParametersSize() {return p;}
    int GetDegreeOfFreedom() {return DF;}
    QVector<double> GetData_x() {return xdata;}
    QVector<double> GetData_y() {return ydata;}
    double GetData_x(int i) {return xdata[i-1];}
    double GetData_y(int i) {return ydata[i-1];}

    QVector<double> GetFitData_y() {return fydata;}

    double GetSSR() {return SSR;}
    double GetFitVariance() {return SSR/DF;}

    Matrix GetParameters() {return sol;}
    Matrix GetParError() {return error;}
    Matrix GetParPValue() {return pValue;}

    double GetParameters(int i) {return sol.Get(i,1);}
    double GetParError(int i) {return error.Get(i,1);}
    double GetParPValue(int i) {return pValue.Get(i,1);}

    void Print();

    void MsgConnector(QString msg){
        emit SendMsg(msg);
    }

private:
    QVector<double> xdata, ydata;
    QVector<double> fydata;
    Matrix sol;
    Matrix dpar;
    Matrix error;
    Matrix tDis;
    Matrix pValue;
    double SSR;
    int n, p, DF;
    int startIndex;
    bool fitFlag;
    double mean, var; // sample

    double cum_tDis30(double x){
        return 1/(1+exp(-x/0.6));
    }

    double FitFunc(bool fitType, double x, QVector<double> par){
        double fit = 0;
        if(fitType == 1 ) fit = par[0] * exp(-x/par[1]) + par[2] * exp(-x/par[3]);
        if(fitType == 0 ) fit = par[0] * exp(-x/par[1]);
        return fit;
    }

    QVector<double> GradFitFunc(bool fitType, double x, QVector<double> par){
        QVector<double> gradFit;

        gradFit.push_back(exp(-x/par[1]));
        gradFit.push_back(par[0] * x* exp(-x/par[1])/par[1]/par[1]);
        if(fitType == 1){
            gradFit.push_back(exp(-x/par[3]));
            gradFit.push_back(par[2] * x* exp(-x/par[3])/par[3]/par[3]);
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
