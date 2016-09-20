#ifndef MATRIX_H
#define MATRIX_H

#include <QObject>
#include <QDebug>
#include <QVector>

class Exception
{
public:
  const QString msg;
  Exception(const QString arg): msg(arg){}
};

class Matrix : public QObject
{
    Q_OBJECT
public:

    explicit Matrix();
    explicit Matrix(const int row, const int col);
    Matrix(const Matrix& a);
    Matrix(int row, int col,  double v);
    Matrix& operator= (const Matrix& a);
    ~Matrix();

    double & operator () (const int r, const int c);

    // Matrix with Matrix
    Matrix operator + (const Matrix& a);
    Matrix operator - (const Matrix& a);
    Matrix operator * (const Matrix& a);

    // Matrix with double
    Matrix operator + (const double b);
    Matrix operator - (const double b);
    Matrix operator * (const double b);
    Matrix operator / (const double b);

    // double with Matrix
    friend Matrix operator + (const double b, const Matrix& a){
        Matrix res = a;
        return res + b;
    }

    friend Matrix operator - (const double b, const Matrix& a){
        Matrix res = -a;
        return res + b;
    }

    friend Matrix operator * (const double b, const Matrix& a){
        Matrix res = a;
        return res * b;
    }
    // operator unary minus
    friend Matrix operator - (const Matrix& a){
        Matrix res(a.rows, a.cols);

        for (int r = 0; r < a.rows; r++){
            for (int c = 0; c < a.cols; c++){
                res.p[r][c] = -a.p[r][c];
            }
        }

        return res;
    }

    //Matrix Self function
    Matrix Minor(const int row, const int col) const; //for Det()
    Matrix Transpose();
    Matrix Inverse();
    double Det();
    int GetRows(){return rows;}
    int GetCols(){return cols;}
    double Get(int r, int c){return p[r-1][c-1];}

    QVector<double> Matrix2QVec(){
        QVector<double> vec;
        int n = 0;
        if( rows == 1) n = cols;
        if( cols == 1) n = rows;

        for( int i = 1; i <= n; i++){
           if( rows==1) vec.push_back(this->Get(1,i));
           if( cols==1) vec.push_back(this->Get(i,1));
        }
        return vec;
    }

signals:

    void SendMsg(QString msg);

public slots:

    void PrintM();
    void PrintM(QString str);
    void PrintVector(QString str);

private:
    double **p;
    int rows, cols;

    //Axulliary function
    void Swap(double& a, double& b){
        double temp = a;
        a = b;
        b = temp;
    }

};

#endif // MATRIX_H
