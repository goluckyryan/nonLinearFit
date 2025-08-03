#include "FindFit.h"

#include <random>

#include "Matrix_zero.h"

int main(){

  // generate data 
  std::vector<double> x, y;
  std::vector<double> par = {10, 50, 10, 8}; // initial parameters for the sigmoid function

  double maxNoise = 4; // maximum noise level

  for( int i = 0; i < 100; i++){
    x.push_back(i);
    double noise = (rand() % 100) / 100.0 * maxNoise; // random noise
    double yVal = par[0] / (1 + exp((i - par[1]) / par[2])) + par[3] + noise; // sigmoid function with noise
    // round up to 3 decimal places
    yVal = round(yVal * 1000.0) / 1000.;

    y.push_back(yVal); 

    printf("{%.0f, %.3f},", x[i], y[i]);

  }

  printf("\n");

  
  FindFit fit;

  fit.SetData(x, y);
  fit.SetParameters({15, 30, 5, 6}); // initial guess for parameters
  fit.LMA(1e-7, 10000, 10, 10); // run the Levenberg-Marquardt algorithm


  // Matrix A(4, 4);
  // A(1, 1) = 1;  A(1, 2) = 2;   A(1, 3) = 15; A(1, 4) = 12;
  // A(2, 1) = 5;  A(2, 2) = 10;  A(2, 3) = 7;  A(2, 4) = 8;
  // A(3, 1) = 9;  A(3, 2) = 6;   A(3, 3) = 11; A(3, 4) = 4;
  // A(4, 1) = 13; A(4, 2) = 14 ; A(4, 3) = 3;  A(4, 4) = 16;
  // A.Print();

  // Matrix B = Transpose(A);
  // B.Print();

  // Matrix C = A * B;
  // C.Print();

  // printf("Det(A) = %f\n", Det(A));
  // Matrix D = Inv(A);
  // D.Print();

  // printf("=====================\n");


  // Matrix0 A0(4, 4);
  // A0(0, 0) = 1;  A0(0, 1) = 2;   A0(0, 2) = 15; A0(0, 3) = 12;
  // A0(1, 0) = 5;  A0(1, 1) = 10;  A0(1, 2) = 7;  A0(1, 3) = 8;
  // A0(2, 0) = 9;  A0(2, 1) = 6;   A0(2, 2) = 11; A0(2, 3) = 4;
  // A0(3, 0) = 13; A0(3, 1) = 14 ; A0(3, 2) = 3;  A0(3, 3) = 16;
  // A0.Print();

  // Matrix0 B0 = Transpose(A0);
  // B0.Print();

  // Matrix0 C0 = A0 * B0;
  // C0.Print();

  // printf("Det(A0) = %f\n", Det(A0));
  // Matrix0 D0 = Inv(A0);
  // D0.Print();


}