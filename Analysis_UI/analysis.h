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

    void SetData(const QVector<double> x, const QVector<double> y);

    double Mean(int index_1, int index_2);
    double Variance(int index_1, int index_2);
    void Regression(bool fitType, QVector<double> par);
    Matrix *NonLinearFit(int startFitIndex, QVector<double> iniPar);

signals:

    void SendMsg(QString msg);

public slots:
    int GetDataSize(){ return n;}
    int GetParametersSize() {return p;}
    int GetDegreeOfFreedom() {return DF;}
    QVector<double> GetData_x() {return xdata;}
    QVector<double> GetData_y() {return ydata;}
    Matrix GetParameters() {return sol;}
    Matrix GetParError() {return error;}
    Matrix GetParPValue() {return pValue;}
    double GetSSR() {return SSR;}
    double GetFitSigma() {return sigma;}
    void MsgConnector(QString msg){
        emit SendMsg(msg);
    }

private:
    QVector<double> xdata, ydata;
    Matrix sol;
    Matrix dpar;
    Matrix error;
    Matrix tDis;
    Matrix pValue;
    double SSR;
    double sigma; //fit sigma
    int n, p, DF;
    int startIndex;
    bool errFlag;

    double cum_tDis30(double x){
        return 1/(1+exp(-x/0.6));
    }

    double fitFunction(bool fitType, double x, QVector<double> par){
        double fit = 0;
        if(fitType == 1 ) fit = par[0] * exp(-x/par[1]) + par[2] * exp(-x/par[3]);
        if(fitType == 0 ) fit = par[0] * exp(-x/par[1]);
        return fit;
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
