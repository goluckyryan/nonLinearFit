#ifndef MATRIX_ZERO_H
#define MATRIX_ZERO_H

/*
A simple matrix class (0-based indexing)
Author: Adapted for 0-based indexing from Ryan Tang's Matrix.h

Features:
  - create a 2D matrix with custom size
  - get/set the cell values (0-based)
  - use operators +, -, *, /
  - use functions Ones(), Zeros(), Diag(), Det(), Inv(), Size()
  - print the content of the matrix
*/

#include <cstdlib>
#include <cstdio>
#include <cmath>

class Exception{
public:
  const char* msg;
  Exception(const char* arg) : msg(arg){}
};

class Matrix{
public:
  Matrix() : rows(0), cols(0), p(nullptr) {}
  Matrix(int row_count, int col_count) : rows(row_count), cols(col_count) {
    if (rows > 0 && cols > 0) {
      p = new double*[rows];
      for (int r = 0; r < rows; r++) {
        p[r] = new double[cols];
        for (int c = 0; c < cols; c++) p[r][c] = 0;
      }
    } else {
      p = nullptr;
    }
  }
  Matrix(const Matrix& a) : rows(a.rows), cols(a.cols) {
    if (rows > 0 && cols > 0) {
      p = new double*[rows];
      for (int r = 0; r < rows; r++) {
        p[r] = new double[cols];
        for (int c = 0; c < cols; c++) p[r][c] = a.p[r][c];
      }
    } else {
      p = nullptr;
    }
  }
  Matrix& operator=(const Matrix& a) {
    if (this == &a) return *this;
    if (p) {
      for (int r = 0; r < rows; r++) delete[] p[r];
      delete[] p;
    }
    rows = a.rows;
    cols = a.cols;
    if (rows > 0 && cols > 0) {
      p = new double*[rows];
      for (int r = 0; r < rows; r++) {
        p[r] = new double[cols];
        for (int c = 0; c < cols; c++) p[r][c] = a.p[r][c];
      }
    } else {
      p = nullptr;
    }
    return *this;
  }
  ~Matrix() {
    if (p) {
      for (int r = 0; r < rows; r++) delete[] p[r];
      delete[] p;
    }
    p = nullptr;
  }

  // 0-based index operator
  double& operator()(int r, int c) {
    if (p && r >= 0 && r < rows && c >= 0 && c < cols) {
      return p[r][c];
    } else {
      throw Exception("Subscript out of range");
    }
  }
  double operator()(int r, int c) const {
    if (p && r >= 0 && r < rows && c >= 0 && c < cols) {
      return p[r][c];
    } else {
      throw Exception("Subscript out of range");
    }
  }

  int GetRows() const { return rows; }
  int GetCols() const { return cols; }

  void Print() const {
    if (p) {
      printf("[");
      for (int r = 0; r < rows; r++) {
        if (r > 0) printf(" ");
        for (int c = 0; c < cols - 1; c++) printf("%+.3e, ", p[r][c]);
        if (r < rows - 1) printf("%+.3e;\n", p[r][cols - 1]);
        else printf("%+.3e]\n", p[r][cols - 1]);
      }
    } else {
      printf("[ ]\n");
    }
  }

  // Element-wise operations
  Matrix& Add(double v) {
    for (int r = 0; r < rows; r++)
      for (int c = 0; c < cols; c++) p[r][c] += v;
    return *this;
  }
  Matrix& Subtract(double v) { return Add(-v); }
  Matrix& Multiply(double v) {
    for (int r = 0; r < rows; r++)
      for (int c = 0; c < cols; c++) p[r][c] *= v;
    return *this;
  }
  Matrix& Divide(double v) {
    if (v == 0) throw Exception("Division by zero");
    return Multiply(1.0 / v);
  }

  // Static creation functions
  static Matrix Ones(int rows, int cols) {
    Matrix res(rows, cols);
    for (int r = 0; r < rows; r++)
      for (int c = 0; c < cols; c++) res(r, c) = 1;
    return res;
  }
  static Matrix Zeros(int rows, int cols) {
    return Matrix(rows, cols);
  }
  static Matrix Diag(int n) {
    Matrix res(n, n);
    for (int i = 0; i < n; i++) res(i, i) = 1;
    return res;
  }
  // Add Minor method for determinant/inverse
  Matrix Minor(int row, int col) const {
    Matrix res(rows - 1, cols - 1);
    for (int r = 0, rr = 0; r < rows; r++) {
      if (r == row) continue;
      for (int c = 0, cc = 0; c < cols; c++) {
        if (c == col) continue;
        res(rr, cc) = p[r][c];
        cc++;
      }
      rr++;
    }
    return res;
  }

private:
  int rows, cols;
  double** p;
};

// Matrix addition
inline Matrix operator+(const Matrix& a, const Matrix& b) {
  if (a.GetRows() != b.GetRows() || a.GetCols() != b.GetCols())
    throw Exception("Dimensions do not match");
  Matrix res(a.GetRows(), a.GetCols());
  for (int r = 0; r < a.GetRows(); r++)
    for (int c = 0; c < a.GetCols(); c++)
      res(r, c) = a(r, c) + b(r, c);
  return res;
}
// Matrix subtraction
inline Matrix operator-(const Matrix& a, const Matrix& b) {
  if (a.GetRows() != b.GetRows() || a.GetCols() != b.GetCols())
    throw Exception("Dimensions do not match");
  Matrix res(a.GetRows(), a.GetCols());
  for (int r = 0; r < a.GetRows(); r++)
    for (int c = 0; c < a.GetCols(); c++)
      res(r, c) = a(r, c) - b(r, c);
  return res;
}
// Matrix multiplication
inline Matrix operator*(const Matrix& a, const Matrix& b) {
  if (a.GetCols() != b.GetRows())
    throw Exception("Dimensions do not match for multiplication");
  Matrix res(a.GetRows(), b.GetCols());
  for (int r = 0; r < a.GetRows(); r++)
    for (int c = 0; c < b.GetCols(); c++) {
      res(r, c) = 0;
      for (int k = 0; k < a.GetCols(); k++)
        res(r, c) += a(r, k) * b(k, c);
    }
  return res;
}
// Scalar multiplication
inline Matrix operator*(const Matrix& a, double b) {
  Matrix res = a;
  res.Multiply(b);
  return res;
}
inline Matrix operator*(double b, const Matrix& a) {
  Matrix res = a;
  res.Multiply(b);
  return res;
}
// Scalar addition/subtraction
inline Matrix operator+(const Matrix& a, double b) {
  Matrix res = a;
  res.Add(b);
  return res;
}
inline Matrix operator-(const Matrix& a, double b) {
  Matrix res = a;
  res.Subtract(b);
  return res;
}
// Scalar division
inline Matrix operator/(const Matrix& a, double b) {
  Matrix res = a;
  res.Divide(b);
  return res;
}

// Determinant (recursive, for small matrices)
inline double Det(const Matrix& a) {
  int n = a.GetRows();
  if (n != a.GetCols()) throw Exception("Matrix must be square");
  if (n == 1) return a(0, 0);
  if (n == 2) return a(0, 0) * a(1, 1) - a(0, 1) * a(1, 0);
  double d = 0;
  for (int c = 0; c < n; c++) {
    double sign = (c % 2 == 0) ? 1 : -1;
    d += sign * a(0, c) * Det(a.Minor(0, c));
  }
  return d;
}

// Transpose
inline Matrix Transpose(const Matrix& a) {
  Matrix res(a.GetCols(), a.GetRows());
  for (int r = 0; r < a.GetRows(); r++)
    for (int c = 0; c < a.GetCols(); c++)
      res(c, r) = a(r, c);
  return res;
}

// Inverse (adjugate method for small matrices)
inline Matrix Inv(const Matrix& a) {
  int n = a.GetRows();
  if (n != a.GetCols()) throw Exception("Matrix must be square");
  double d = Det(a);
  if (d == 0) throw Exception("Matrix is singular");
  if (n == 1) {
    Matrix res(1, 1);
    res(0, 0) = 1.0 / a(0, 0);
    return res;
  }
  if (n == 2) {
    Matrix res(2, 2);
    res(0, 0) = a(1, 1);
    res(0, 1) = -a(0, 1);
    res(1, 0) = -a(1, 0);
    res(1, 1) = a(0, 0);
    return (1.0 / d) * res;
  }
  Matrix res(n, n);
  for (int r = 0; r < n; r++) {
    for (int c = 0; c < n; c++) {
      double sign = ((r + c) % 2 == 0) ? 1 : -1;
      res(c, r) = sign * Det(a.Minor(r, c)); // note transpose
    }
  }
  return (1.0 / d) * res;
}

#endif
