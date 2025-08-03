#ifndef LMA_H
#define LMA_H

// this is a stand alone file for Levenberg-Marquardt Algorithm (LMA) implementation

#include <vector>
#include <cstdlib>
#include <cmath>

#include "Matrix.h"

double FitFunction(const double x, const std::vector<double> &par){
	double A = par[1];
	double T = par[2];
	double R = par[3];
	double B = par[4];
	return A /(1 + exp((x-T)/R)) + B; 
}

Matrix GradFitFunction(const double x, const std::vector<double> &par){
	Matrix grad(4,1);
	
	double A = par[1];
	double T = par[2];
	double R = par[3];
	double B = par[4];

  double exp_term = exp((x - T) / R);
	
	grad(1,1) = 1/exp_term; // derivative with respect to A
	grad(2,1) = A *  exp_term / (R * exp_term * exp_term); // derivative with respect to T
	grad(3,1) = A * (x - T) * exp_term / (R * R * exp_term * exp_term); // derivative with respect to R
	grad(4,1) = 1; // derivative with respect to B

  // Note: grad(1,1) is the derivative with respect to A, grad(2,1) with respect to T, etc.
  // Adjust the indices if your parameter vector is structured differently.
	return grad;
}

int NonLinearRegression(const std::vector<double> &x, const std::vector<double> &y, const std::vector<double> &par) {
  // Perform regression to find the best fit parameters
  // This function should compute the sum of squared residuals (SSR)

  // in Taylor series, we have:
  // Y = f + J*(p-p0) + e, J = df/dp = Jacobian, n x p 
  // let J^T*J = H is the Hessian matrix, p x p
  // p = p0 + inverse(H).(J^T*(Y-f)),
  
  
  // in Gradient Descent, 
  // the SSR (sum of squared residuals) is minimized is
  // SSR = (Y - f)^T * (Y - f) 
  // the derivative of SSR with respect to p is
  // dSSR/dp = -2 * J^T * (Y - f)
  // p = p0 + a * J^T * (Y - f)

  // LM method is a combination of Gradient Descent and Newton's method
  // p = p0 + inverse(H + lambda * I) * J^T * (Y - f)
  // Newton's method is fast but unstable, while Gradient Descent is stable but slow

  int n = x.size();
  int p = par.size();

  Matrix J(n, p);  // n x p matrix for function values
  Matrix Y(n, 1);  // n x 1 matrix for observed values
  Matrix f(n ,1);  // n x 1 matrix for fitted values

  for( int i = 1; i <= n; i++) {
    Y(i, 1) = y[i];
    double x_val = x[i];
    f(i, 1) = FitFunction(x_val, par);
    
    Matrix Frow = GradFitFunction(x_val, par); // p x 1 matrix for gradient
    for (int k = 1; k <= p; k++)  J(i, k) = Frow(k, 1); 

  }

  Matrix Jt = Transpose(J);
  Matrix H = Jt * J;

  double lambda = 0;
  for(int i = 1 ; i <= n ; i++){
    for(int j = 1 ; j <= p ; j++){
      lambda += J(i,j)*J(i,j);
    }
  }
  lambda = sqrt(lambda/n/p);
  printf("====== ini lambda : %f \n", lambda);

  Matrix DI(H.GetRows(), H.GetCols()); // identity matrix
	for( int i = 1; i <= H.GetRows() ; i++) DI(i,i) = lambda ;

  Matrix Q = H + DI;
	Matrix CoVar;
	try{
		CoVar = Inv(Q);
	}catch( Exception err){
		return -1; // Error in matrix inversion
	}

  Matrix dY = Y - f;
  Matrix JtY = Jt * dY; // p x 1 matrix for gradient of SSR
  std::vector<double> par_old = par; // store old parameters

  double SSR = (Transpose(dY) * dY)(1, 1); // compute initial SSR

  int nDF = n - p; // degrees of freedom
  Matrix dpar = CoVar * JtY; // p x 1 matrix for

  std::vector<double> par_new = par_old; // new parameters
  for (int i = 1; i <= p; ++i)  par_new[i] += dpar(i, 1); // update parameters

  //====================== Compute new SSR
  Matrix fn(n, 1);
  for(int i = 1; i <= n ; i++) {
    double x_val = x[i - 1];
    fn(i, 1) = FitFunction(x_val, par_new);
  }
  Matrix dYn = Y - fn;
  double new_SSR = (Transpose(dYn) * dYn)(1, 1); // new SSR

  double delta = new_SSR - SSR; // change in SSR
  if (delta > 0) { 
    lambda *= 10; // increase lambda if SSR increased, because grdient descent should weight more 
    par_new = par_old; // revert to old parameters
    return 0;
  }else{
    lambda /= 10; // decrease lambda if SSR decreased
    SSR = new_SSR; // update SSR
    par_old = par_new; // update old parameters
  }

  return 1; // Return 0 for success, non-zero for failure
}


void LMA(std::vector<double> &x, std::vector<double> &y, std::vector<double> &par){

  // Levenberg-Marquardt Algorithm implementation
  // x: independent variable data
  // y: dependent variable data
  // par: parameters to be optimized
  
  int n = x.size();
  int p = par.size();
  
  Matrix F(n, p);
  Matrix Y(n, 1);
  Matrix sol(p, 1);
  Matrix dpar(p, 1);
  Matrix error(p, 1);
  Matrix pValue(p, 1);
  
  double SSR = 0.0;
  double lastSSR = 0.0;
  double lambda = 0.01;
  double delta;
  int errFlag = 0;
  const double TORR = 0.001;
  const int MAXITR = 300;
  
  // Initialize matrices and variables
  for (int i = 0; i < n; ++i) {
    Y(i, 1) = y[i];
  }
  
  // Main loop for LMA
  for (int iter = 0; iter < MAXITR && errFlag == 0; ++iter) {
    // Compute the fit function and its gradient
    for (int i = 0; i < n; ++i) {
      double x_val = x[i];
      F(i, 1) = FitFunction(x_val, par);
      Matrix grad_row = GradFitFunction(x_val, par);
      for (int j = 0; j < p; ++j) {
        F(i, j + 1) = grad_row(j + 1, 1);
      }
    }
    
    Matrix Ft = Transpose(F);
    Matrix FtF = Ft * F;
    
    if (lambda == -1) {
      lambda = 0.01; // Initial lambda value
    }
    
    Matrix FtY = Ft * Y;
    
    try {
      sol = Inv(FtF + lambda * Identity(p)) * FtY;
    } catch (Exception &err) {
      errFlag = 1; // Error in matrix inversion
      break;
    }
    
    dpar = sol - par;
    
    if (std::abs(delta) <= TORR * SSR) {
      errFlag = 2; // Convergence achieved
      break;
    }
    
    lastSSR = SSR;
    
    if (errFlag == 0 && delta > TORR * SSR) {
      lambda /= 10; // Decrease lambda
    } else {
      lambda *= 10; // Increase lambda
      sol = par; // Revert to previous solution
      try {
        sol = Inv(FtF); // Recalculate covariance matrix
      } catch (Exception &err) {    
        errFlag = 1; // Error in matrix inversion
        break;
      }
      double var = SSR / (n - p);
      for (int i = 0; i < p; ++i) {
        error(i, 1) = sqrt(var * sol(i, i));
      } 



}



#endif