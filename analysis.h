#ifndef Analysis_H
#define Analysis_H

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
#include <vector>
#include "Matrix.h"

class Analysis{
	
private:

double *x, *y;
int n, p, DF;
int xStart;
double SSR, mean, var;
std::vector<double> sol, dpar, error, tDis, pValue;

std::vector<double> Matrix2Vector(const Matrix mat);
void MeanAndVariance(int xEnd);
double cum_tDis30(double x){ return 1/(1+exp(-x/0.6));}
	
	
public:
	
Analysis();
Analysis(std::vector<double> x, std::vector<double> y);
///Analysis& operator= (const Analysis& a);
~Analysis();

void Initialize();
int Regression(std::vector<double> par);


///interface
double GetMean() { return mean;}
double GetVariance() { return var;}
double GetFitVariance() {return SSR/DF;}
double GetFitSigma() {return sqrt(SSR/DF);}
double GetxStart() {return x[xStart]}; 
double GetxStartIndex() {return xStart};
double Getx(int i) {return x[i];} 
double Gety(int i) {return y[i];} 

std::vector<double> GetFittedPar() {return sol;}
std::vector<double> GetParError()  {return error;}
std::vector<double> GetdPar()      {return dpar;}
std::vector<double> GettDis()      {return tDis;}
std::vector<double> GetpValue()    {return pValue;}
double GetFittedPar(int i) {return sol[i];}
double GetParError(int i)  {return error[i];}
double GetdPar(int i)      {return dpar[i];}
double GettDis(int i)      {return tDis[i];}
double GetpValue(int i)    {return pValue[i];}


};

Analysis::Analysis(){
	Initialize();
}

Analysis::Analysis(std::vector<double> x, std::vector<double> y){
	Initialize();
	
	int nx = x.size();
	int ny = y.size();
	printf(" size of x (%d),  size of y (%d)\n", nx, ny);
	if( nx != ny){
		printf(" size of x (%d) != size of y (%d)\n", nx, ny);
		return;
	}
	
	this->n = nx;
	
	const int N = nx;
	this->x = new double[N];
	this->y = new double[N];
	
	for( int i = 0; i < this->n ; i++){
		(this->x)[i] = x[i];
		(this->y)[i] = y[i];
		//printf("%d/%d,  %f \n", i, this->n, (this->x)[i]);
	}
	
	MeanAndVariance(this->n);
	
}

Analysis::~Analysis(){
	///printf("destructor\n");
	///delete this->x;
	///delete this->y;
}

void Analysis::Initialize(){
	n = 0;
	p = 0;
	DF = 0;
	xStart = 0;
	SSR = 9999;
	mean = 0;
	var = 0;
	
	x = NULL;
	y = NULL;
}

std::vector<double> Analysis::Matrix2Vector(const Matrix mat){
	if(mat.GetRows() != 1 && mat.GetCols() != 1){
		printf(" abort.\n");
	}
	
	int size = 0;
	if(mat.GetRows() == 1) size = mat.GetCols();
	if(mat.GetCols() == 1) size = mat.GetRows();
	
	std::vector<double> outvec;
	
	for( int i = 0; i < size ; i++){
		if(mat.GetRows() == 1) outvec.push_back(mat.Get(1,i+1));
		if(mat.GetCols() == 1) outvec.push_back(mat.Get(i+1,1));
	}	
	return outvec;
}

void Analysis::MeanAndVariance(int xEnd){
	
	this->mean = 0;
	this->var  = 0;

	//find mean
	for( int i = 0; i < xEnd ; i ++ ){
		this->mean += (this->y)[i];
	}
	this->mean = this->mean / xEnd;
	
	for( int i = 0; i < xEnd ; i ++ ){
		this->var += pow((this->y)[i] - this->mean,2);
	}
	this->var = this->var/(xEnd-1);
	
	///printf("%f, %f \n", this->mean, this->var);

}

/********
double valX [1024];      // val of X or time
double valY [500];       // val of Y or B-field
double data [500][1024]; // val of data
int sizeX, sizeY;
int errFlag;  //err

Matrix sol, dpar, error, tDis, pValue;
int DF;
double SSR, var;

const double torr = 0.01;
const double CL   = 0.05;




double mean(int yIndex){
	int xStart = 195-20; // start for fitting, end of finding variance 
	
	//find mean
	double mean = 0;
	for( int i = 0; i < xStart ; i++){
		mean += data[yIndex][i];
	} 
	mean = mean/xStart;
	
	return mean;
}

double variance(int yIndex){ // finding variance T < ~3 ns
	int xStart = 195-20; // start for fitting, end of finding variance 
	
	const double m = mean(yIndex);
	
	//find variance;
	double var = 0;
	for( int i = 0; i < xStart ; i++){
		var += pow(data[yIndex][i] - m,2);
	} 
	var = var/(xStart-1);
	
	return var;
}

void regression(bool fit4par, int yIndex,double a, double Ta, double b, double Tb, int info){
	
	errFlag = 0;
	
	int xStart = 195;
	int xEnd = 1000;
	
	int p = 2; // number of parameters
	if( fit4par ) p = 4;
	
	int n = xEnd - xStart ;
	
	Matrix Y(n,1);
	for(int i = 1; i <= n ; i++) {
		Y(i,1) = data[yIndex][i - 1 + xStart];
	}
	
	Matrix f(n,1);
	for(int i = 1; i <= n ; i++) {
		//f(i,1) = funcX(valX[i - 1 +xStart], a, Ta, b, Tb);
		double x = valX[i - 1 +xStart];
		
		f(i,1) = a * exp(-x/Ta); 
		if(  fit4par ) f(i,1) += b * exp(-x/Tb);  
	}
	
	Matrix F(n,p);
	for(int i = 1; i <= n ; i++) {
		double x = valX[i - 1 +xStart];
		F(i,1) = exp(-x/Ta);
		F(i,2) = a * x * exp(-x/Ta)/Ta/Ta;
		if( fit4par ) F(i,3) = exp(-x/Tb);
		if( fit4par ) F(i,4) = b * x * exp(-x/Tb)/Tb/Tb;
	}
	
	Matrix Ft = Transpose(F); //printf("    Ft(%d,%d)\n", Ft.GetRows(), Ft.GetCols());
	Matrix FtF = Ft*F;        //printf("   FtF(%d,%d)\n", FtF.GetRows(), FtF.GetCols());
	
	Matrix CoVar;
	try{
		CoVar = Inv(FtF);  //printf(" CoVar(%d,%d)\n", CoVar.GetRows(), CoVar.GetCols());
	}catch( Exception err){
		//if( info >= 1) printf("%s. #par=%d | Terminated.\n", err.msg, p);
		//FtF.Print();
		errFlag = 1;
		return;
	}
	
	//CoVar.Print();
	
	if( isnan(CoVar(1,1)) ) {
		errFlag = 1;
		return;
	}
	
	Matrix dY = Y - f;    //printf("    dY(%d,%d)\n", dY.GetRows(), dY.GetCols());
	Matrix FtdY = Ft*dY;  //printf("  FtdY(%d,%d)\n", FtdY.GetRows(), FtdY.GetCols());
	
	Matrix par_old(p,1);
	par_old(1,1) = a;
	par_old(2,1) = Ta;
	if( fit4par ) par_old(3,1) = b;
	if( fit4par ) par_old(4,1) = Tb;

	DF = n - p;	
	dpar = CoVar * FtdY;  //printf("  dpar(%d,%d)\n", dpar.GetRows(), dpar.GetCols());
	sol = par_old + dpar;  //printf("  sol(%d,%d)\n", sol.GetRows(), sol.GetCols());
	SSR = (Transpose(dY)*dY)(1,1); //printf("SSR = %f\n", SSR);
	var = SSR / DF;
	
	error = Matrix(p,1);
	for( int i = 1; i <= p ; i++){
		error(i,1) = sqrt(var * CoVar(i,i));
	}

	tDis = Matrix(p,1);
	for( int i = 1; i <= p ; i++){
		tDis(i,1) = sol(i,1)/error(i,1);
	}
	
	pValue = Matrix(p,1);
	for( int i = 1; i <= p ; i++){
		pValue(i,1) = cum_tDis30(- std::abs(tDis(i,1)));
	}
	
}

void GaussNewton(int yIndex, bool fit4par, int info, double a, double Ta, double b, double Tb){
	//info < 0; mo msg;
	//info = 0; only B-field and sol;
	//info = 1; + sigma 
	//info = 2; + p-value;
	//info = 3; + Regression;
	//info = 4; + Reg4
	
	if( fit4par && ( a == 0 || b == 0) ){
		 errFlag = 3;
		 printf("a or b = 0 result ill Covariance. abort 4-parameters fit.\n");
		 return;
	}

	if( info >= 3) if( fit4par == 1 )printf(" Regression of 4-parameters: ");
	if( info >= 3) if( fit4par == 0 )printf(" Regression of 2-parameters: ");

	
	int count = 0;
	
	bool checkdpar = 1;
	
	double  a1 =  a;
	double Ta1 = Ta;
	double  b1 =  b;
	double Tb1 = Tb;
	if(fit4par == 0){
		b1 = 0;
		Tb = 0;
	}
	
	do{
		count ++ ;
		if( count > 50){
			errFlag = 2;
			break;
		}
		regression(fit4par, yIndex, a1, Ta1, b1, Tb1, info);
		a1  = sol(1,1);
		Ta1 = sol(2,1);
		b1  = 0;
		Tb1 = 0;
		if( fit4par){
			b1  = sol(3,1);
			Tb1 = sol(4,1);
		}

		//printf("sol : ");Transpose(sol).Print();
		//printf("dpar: ");Transpose(dpar).Print();
		//printf("sigma: ");Transpose(error).Print();
		printf("%d ", count);
		if( fit4par == 1) checkdpar = (std::abs(dpar(1,1)) > torr || 
										std::abs(dpar(2,1)) > torr ||
										std::abs(dpar(3,1)) > torr || 
										std::abs(dpar(4,1)) > torr ) ;
		
		if( fit4par == 0) checkdpar = (std::abs(dpar(1,1)) > torr || 
									   std::abs(dpar(2,1)) > torr ) ;
		
	}while(errFlag == 0  && checkdpar); //iterate when no err and any of the dpar > 0.01;
	
	
	if( info >= 3) if( errFlag == 0 ) printf("| End.\n");
	if( info >= 3) if( errFlag == 1 ) printf("| fail to cal. Covaraince. Terminated. \n");
	if( info >= 3) if( errFlag == 2 ) printf("| fail to converge in 50 trials. Terminated.\n");
	if( info >= 4) {printf("         sol  : "); Transpose(sol).Print(); }
	//if( info >= 4) {printf("           d  : "); Transpose(dpar).Print(); }
	if( info >= 4) {printf("        sigma : "); Transpose(error).Print();}
	//if( info >= 4) {printf("        t-dis : "); Transpose(tDis).Print();}
	if( info >= 4) {printf("Appr. p-Value : "); Transpose(pValue).Print();}
	
}


void Fitting(int yIndex, int info, double a, double Ta, double b, double Tb){
	
	double Yvalue = valY[yIndex];
	if( info >= 0) printf("========================= index : %3d, B field : %f ", yIndex, Yvalue);
	if( info >= 1) printf("\n");
	

	GaussNewton(yIndex, 1, info, a, Ta, b, Tb);

	/// Check the fit
	int failFlag = 0;
	if( errFlag == 0 && 
	   (pValue(1,1) > CL ||
		pValue(2,1) > CL ||
		pValue(3,1) > CL ||
		pValue(4,1) > CL )
	   ){ //95% confident level

		if( info >= 3) {printf(" *********** Result rejected, Appr. p-Value : "); Transpose(pValue).Print();}
		
		failFlag = 1;
	}

	if( failFlag || errFlag){
		GaussNewton(yIndex, 0, info, a, Ta, b, Tb);
	}

	double SampleMean = mean(yIndex);
	double SampleVar = variance(yIndex);
	double fitVar = var;
	double redchisq = fitVar/SampleVar;
	
	if( info >= 3) printf("####### red-chi^2: %f | FitSigma: %f, SampleSigma(Mean): %f(%f) \n", redchisq, sqrt(fitVar), sqrt(SampleVar), SampleMean);
	if( info >= 0) {printf("         sol  : "); Transpose(sol).Print();}
	if( info >= 1) {printf("        sigma : "); Transpose(error).Print();}
	//if( info >= 0) {printf("        t-dis : "); Transpose(tDis).Print();}
	if( info >= 2) {printf("Appr. p-Value : "); Transpose(pValue).Print();}
	

}

void SaveFitResult(char* filename, int yIndex){

	double fitvar = var;
	
	double SampleVar = variance(yIndex);
	double redchisq = fitvar/SampleVar;
	
	int p = sol.GetRows();
	
	double a[4], s[4], pV[4];

	a[0] = sol(1,1); s[0] = error(1,1); pV[0] = pValue(1,1);
	a[1] = sol(2,1); s[1] = error(2,1); pV[1] = pValue(2,1);
	a[2] = 0       ; s[2] = 0         ; pV[2] = 0;
	a[3] = 0       ; s[3] = 0         ; pV[3] = 0;
	
	if( p == 4){
		a[2] = sol(3,1); s[2] = error(3,1); pV[2] = pValue(3,1);
		a[3] = sol(4,1); s[3] = error(4,1); pV[3] = pValue(4,1);
	}
	
	FILE * file;

	if( std::abs(a[0]) < 1e+3 && a[1]>0 && a[1] < 1000){ 
		file = fopen (filename, "a+");	
		//output of B-field
		fprintf(file, "%3d, %7.3f, ", yIndex, valY[yIndex]);
		//output of data
		for ( int i = 0; i < 4 ; i++) fprintf(file, "%8.3f, ", a[i]);
		for ( int i = 0; i < 4 ; i++) fprintf(file, "%8.3f, ", s[i]);
		for ( int i = 0; i < 4 ; i++) fprintf(file, "%8.3f, ", pV[i]);
		fprintf(file, "%8.4f\n", redchisq);
	
		fclose(file);	
	}
	
	//delete file;
		
}

***********/


#endif
