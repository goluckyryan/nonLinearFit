#include "FindFit.h"

#include <random>

int main(){

  // generate data 
  std::vector<double> x, y;
  std::vector<double> par = {10, 50, 10, 8}; // initial parameters for the sigmoid function

  double maxNoise = 0; // maximum noise level

  for( int i = 0; i < 100; i++){
    x.push_back(i);
    double noise = (rand() % 100) / 100.0 * maxNoise; // random noise
    double yVal = par[0] / (1 + exp((i - par[1]) / par[2])) + par[3] + noise; // sigmoid function with noise
    // round up to 3 decimal places
    yVal = round(yVal * 1000.0) / 1000.;

    y.push_back(yVal); 

    // printf("{%.0f, %.3f},\n", x[i], y[i]);

  }


  
  FindFit fit;

  fit.SetData(x, y);
  fit.SetParameters({15, 30, 5, 6}); // initial guess for parameters
  fit.LMA(1e-7, 10000, 10, 10); // run the Levenberg-Marquardt algorithm


}