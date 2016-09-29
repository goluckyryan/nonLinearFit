#ifndef AnaLibrary_H
#define AnaLibrary_H

#include <istream> 
#include <fstream> 
#include <string> 
#include <sstream> 
#include <cstdlib> 
#include <cmath> 
#include <ctime> 
#include <stdio.h> 
#include <math.h> 
#include <cstring> 
#include "Matrix.h"

using namespace std; 

double valX [1024];      // val of X or time
double valY [500];       // val of Y or B-field
double data [500][1024]; // val of data
int sizeX, sizeY;
int errFlag;  //err

Matrix sol, dpar, error, pValue, gradSSR;
int nDF, dataSize, p;
double SSR, lastSSR;
double lambda = 0.01, delta; // delta = SSRn - SSR
double sMean, sVar;

const double TORR = 0.001;
const double MAXITR = 300;

double Cum_tDis30(double x){
	return 1/(1+exp(-x/0.6));
}

void MeanVariance(int yIndex){
	int xStart = 195-20; // start for fitting, end of finding variance 
	
	//find mean
	sMean = 0;
	for( int i = 0; i < xStart ; i++){
		sMean += data[yIndex][i];
	} 
	sMean = sMean/xStart;
	
	//find variance
	sVar = 0;
	for( int i = 0; i < xStart ; i++){
		sVar += pow(data[yIndex][i] - sMean,2);
	} 
	sVar = sVar/(xStart-1);
	
}

double FitFunction(double x, Matrix par){
	double a  = par(1,1);
	double Ta = par(2,1);
	double b  = par(3,1);
	double Tb = par(4,1);
	return a * exp(-x/Ta) + b * exp(-x/Tb); 
}

Matrix GradFitFunction(double x, Matrix par){
	Matrix grad(4,1);
	
	double a  = par(1,1);
	double Ta = par(2,1);
	double b  = par(3,1);
	double Tb = par(4,1);
	
	grad(1,1) = exp(-x/Ta);
	grad(2,1) = a * x * exp(-x/Ta)/Ta/Ta;
	grad(3,1) = exp(-x/Tb);
	grad(4,1) = b * x * exp(-x/Tb)/Tb/Tb;
	
	return grad;
}

int regression(int yIndex, Matrix par, int info){
	
	errFlag = 0;
	
	int xStart = 195;
	int xEnd = sizeX-1;
	
	int p = par.GetRows(); /// number of parameters
	
	int n = xEnd - xStart + 1 ;
	dataSize = n;
	
	Matrix Y(n,1);
	Matrix f(n,1);
	Matrix F(n,p);
	for(int i = 1; i <= n ; i++) {
		Y(i,1) = data[yIndex][i - 1 + xStart];
		double x = valX[i - 1 +xStart];
		f(i,1) = FitFunction(x, par);
		Matrix Frow = GradFitFunction(x, par);
		//printf("Frow :"); Transpose(Frow).Print();
		for( int k = 1; k <= p ; k++){
			F(i,k) = Frow(k,1);
		}
	}
	
	Matrix Ft = Transpose(F); 
	Matrix FtF = Ft*F;        
	
	if( lambda == -1){
		
		lambda = 0;
		for(int i = 1 ; i <= n ; i++){
			for(int j = 1 ; j <= p ; j++){
				lambda += F(i,j)*F(i,j);
			}
		}
		lambda = sqrt(lambda/n/p);
		
		printf("====== cal ini lambda : %f \n", lambda);
	}
	
	Matrix DI(FtF.GetRows(), FtF.GetCols());
	for( int i = 1; i <= FtF.GetRows() ; i++){
		DI(i,i) = lambda ;
	}
	
	Matrix Q = FtF + DI;
	Matrix CoVar;
	try{
		CoVar = Inv(Q); 
	}catch( Exception err){
		errFlag = 1; // 1 if fail to cal inverse.
		return 0;
	}
	
	Matrix dY = Y - f;    
	Matrix FtdY = Ft*dY;  
	Matrix par_old = par;
	
	SSR = (Transpose(dY)*dY)(1,1);
	
	nDF = n - p;	
	Matrix dpar = CoVar * FtdY;
	Matrix par_new = par_old + dpar;
		
	//================================ cal new SSR	
	Matrix fn(n,1);
	Matrix Fn(n,p);
	for(int i = 1; i <= n ; i++) {
		double x = valX[i - 1 +xStart];
		fn(i,1) = FitFunction(x, par_new);
		Matrix Frow = GradFitFunction(x, par_new);
		for( int k = 1; k <= p ; k++){
			Fn(i,k) = Frow(k,1);
		}
	}
	
	Matrix dYn = Y - fn;
	double SSRn = (Transpose(dYn)*dYn)(1,1);
	
	Matrix Fnt = Transpose(Fn);
	Matrix FntdYn = Fnt*dYn;
	
	delta = SSRn - SSR;
	
	//printf("===================================\n");
	//printf(" SSR : %10e, delta : %6e, lambda : %10e \n", SSR, delta, lambda);
	//printf("        p: "); Transpose(par_old).Print();
	//printf("    new_p: "); Transpose(par_new).Print();
	//printf("    dpar : "); Transpose(dpar).Print();
	//printf(" gradSSR : "); Transpose(FtdY).Print();
	
	if( SSRn > SSR ){
		lambda = lambda * 10;
		sol = par_old;
		
		try{
			CoVar = Inv(FtF); 
		}catch( Exception err){
			errFlag = 1;
			return 0;
		}
		
		double var = SSR/nDF;
			
		error = Matrix(p,1);
		for( int i = 1; i <= p ; i++){
			error(i,1) = sqrt(var * CoVar(i,i));
		}
	
		pValue = Matrix(p,1);
		for( int i = 1; i <= p ; i++){
			pValue(i,1) = Cum_tDis30(- std::abs(sol(i,1)/error(i,1)));
		}
		
		gradSSR = FtdY;
		
		//printf(" keep unchange \n");
		
		return 0;
	}
	
	lambda = lambda / 10;
		
	sol = par_old + dpar;
	SSR = SSRn;
	
	try{
		CoVar = Inv(Fnt*Fn); 
	}catch( Exception err){
		errFlag = 1;
		return 0;
	}
	
	double var = SSRn/nDF;
	error = Matrix(p,1);
	for( int i = 1; i <= p ; i++){
		error(i,1) = sqrt(var * CoVar(i,i));
	}
	
	pValue = Matrix(p,1);
	for( int i = 1; i <= p ; i++){
		pValue(i,1) = Cum_tDis30(- std::abs(sol(i,1)/error(i,1)));
	}
	
	gradSSR = FntdYn;
	
}

void LMA(int yIndex, int info, Matrix par0){
	//Levenberg-Marquardt Algorithm
	
	bool checkdpar = 1;
	Matrix par_temp = par0;
	
	int count = 0;
	lastSSR = 0;

	do{
		regression(yIndex, par_temp, info);
		par_temp = sol;
		lastSSR = SSR;
		count ++ ;
		
		if( info > 0) printf(" %d", count);
		if( count > MAXITR){
			errFlag = 2;
			break;
		}
		
		//printf("sol : ");Transpose(sol).Print();
		//printf("dpar: ");Transpose(dpar).Print();
		//printf("sigma: ");Transpose(error).Print();
		checkdpar = std::abs(delta) > TORR * SSR ;
		
	}while(errFlag == 0  && checkdpar); //iterate when no err and any of the dpar > 0.01;
	
	
	if( info >= 3) if( errFlag == 0 ) printf("| End.\n");
	if( info >= 3) if( errFlag == 1 ) printf("| fail to cal. Covaraince. Terminated. \n");
	if( info >= 3) if( errFlag == 2 ) printf("| fail to converge in 50 trials. Terminated.\n");
	//if( info >= 4) {printf("         sol  : "); Transpose(sol).Print(); }
	//if( info >= 4) {printf("           d  : "); Transpose(dpar).Print(); }
	//if( info >= 4) {printf("        sigma : "); Transpose(error).Print();}
	//if( info >= 4) {printf("Appr. p-Value : "); Transpose(pValue).Print();}
	
}

void NonLinearFit(int yIndex, int info, Matrix par0){
	
	double Yvalue = valY[yIndex];
	if( info >= 0) printf("======== index : %3d, B field : %f ", yIndex, Yvalue);
	if( info >= 1) printf("\n");
	
	LMA(yIndex, info, par0);

	MeanVariance(yIndex);
	double fitVar = SSR/nDF;
	double redchisq = fitVar/sVar;
	
	if( info >= 3) printf("==== SSR : %f , dataSize = %d, ndf = %d\n", SSR, dataSize, nDF);
	if( info >= 3) printf("==== dSSR : %f \n", delta);
	if( info >= 3) printf("####### red-chi^2: %f | FitSigma: %f, SampleSigma(Mean): %f(%f) \n", redchisq, sqrt(fitVar), sqrt(sVar), sMean);
	if( info >= 0) {printf("         sol  : "); Transpose(sol).Print();}
	if( info >= 1) {printf("        sigma : "); Transpose(error).Print();}
	if( info >= 2) {printf("Appr. p-Value : "); Transpose(pValue).Print();}
		
}


#endif
