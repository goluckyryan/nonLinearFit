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

double ERROR = 999.;


using namespace std; 

double valX [1024];      // val of X or time
double valY [500];       // val of Y or B-field
double data [500][1024]; // val of data
int sizeX, sizeY;

int getData(char* filename){
	
  int multi_Z = 3; //multiply the zval by 10^(multi_Z)
	
  string val;
  ifstream file;
  
  //------------ get 1st line, determine how many set of data, get valY
  file.open(filename);
  
  int numCol = 0, numY = -1;
  int strLen = 0;
  while (file.good()){
	getline(file, val, ','); //get it cell by cell
	if( numCol == 0) {
		strLen = val.length();
		//printf(" %d \n", strLen);
		
	}

	// get the Y data
	if( numCol % 2 == 0){
		size_t pos = val.find_last_of("_"); //get the last "_"
		string strY = val.substr(pos+1, 5);
		numY ++;
		valY[numY] = atof(strY.c_str());
		//printf(" %s, %f \n", strY.c_str(), valY[colID]);
	}
	
	numCol ++;
	if ( strLen != val.length()) break; // the last val is data
  }
  file.close();
 
  double stepY = valY[1]-valY[0];
  numY ++;
  
  printf(" Y : (%.3f, %.3f), step : %.3f , numY : %d \n", valY[0], valY[numY-1], stepY , numY); 
  //printf(" total number of col : %d \n", numCol);
  sizeY = numY;
  
  //------------ get number of Row
  file.open(filename);
  int numX = 0;
  while (file.good()){
   numX ++;
   getline (file, val);
  }
  file.close();
  numX = numX - 2;
  //printf(" total number of X : %d \n", numX);
  sizeX = numX;
  
  //------------ get data 
  file.open(filename);
  if( file.good()) getline(file, val); // skip the 1st line by getline
  
  int numRow = 0, count = 0, iY = 0; 
  double maxZ = 0;
  double minZ = 0;
  
  while (file.good()){
   count ++; 

   if( count % numCol !=0 ){
	   getline (file, val, ',');  
   }else{
	   getline (file, val); 
   }

   //get valX
   if( count % numCol == 1){
		valX[numRow] = atof(val.c_str())*1e6; // in us
		//printf("%10d, %3d, %2d|  %s , %f \n", count, numCol, count % numCol , val.c_str(), valX[numRow]); 
   }
   
   // get data
   if( count % 2 == 0){
	   double Zval = atof(val.c_str())*pow(10,multi_Z); // multiplied by 10^6 times
	   data[iY][numRow] = Zval;
	   if( Zval > maxZ) maxZ = Zval;
	   if( Zval < minZ) minZ = Zval;
	   //printf("*%10d, %3d, %3d, %s, %f\n", count, count% numCol, iY, val.c_str(), data[iY][numRow]);
	   iY ++;
   }
   
   if( count % numCol == 0) {
	   numRow ++ ;
	   iY = 0;
   }
   
   if( numRow > numX ) break;
  }
  
  file.close();
  
  double stepX = valX[1]-valX[0];
  
  printf(" X : (%.3f, %.3f), step : %.3f , numX : %d \n", valX[0], valX[numX-1], stepX , numX); 
  printf(" Z : (%.5f, %.5f)\n", minZ, maxZ);
  // check
  /*
  printf("--------------\n");
  int idY = 349;
  printf(" Y = %f \n", valY[idY]);
  for (int idX = 0 ; idX <=3 ; idX ++){
	printf(" row: %d, X : %f, data : %f \n", idX, valX[idX], data[idY][idX]);
  }
  */
  
  printf("-------------------------\n");
  return 0;
}

int output(char* filename){
	FILE * file;
	file = fopen (filename, "w+");
	
	//output of valY
	fprintf(file, "%*s", 10, "");
	for (int i = 0; i < sizeY; i++){
		fprintf(file, "%10.5f", valY[i]);
	} 
	fprintf(file, "\n");
	
	//output of data
	for (int i = 0; i < sizeX; i++){
		fprintf(file, "%10.3f", valX[i]);
		for (int j = 0; j < sizeY; j++){
			fprintf(file, "%10.2f", data[j][i]);
		} 
		fprintf(file, "\n");
	}
	
	fclose(file);
	
	printf(" saved in %s \n", filename);
	
	return 0;
}

double cum_tDis30(double x){
		return 1/(1+exp(-x/0.6));
}

Matrix* regression(bool tag, int yIndex,double a, double Ta, double b, double Tb, int info){
	
	int xStart = 195;
	int xEnd = 1000;
	
	int p = 2; // number of parameters
	if( tag ) p = 4;
	
	Matrix * output = new Matrix[5]; // 0 = par; 1 = dpar; 2 = sigma ; 3 = t-dis, 4 = p-value;
	output[0] = Zeros(p, 1);
	output[1] = Zeros(p, 1);
	output[2] = Zeros(p, 1);
	output[3] = Zeros(p, 1);
	output[4] = Zeros(p, 1);
	
	int n = xEnd - xStart ;
	
	//printf(" --- forming Y. dim(Y) = %d \n", n);
	Matrix Y(n,1);
	for(int i = 1; i <= n ; i++) {
		Y(i,1) = data[yIndex][i - 1 + xStart];
	}
	//if( tag ) printf(" --- forming f(i) = a * exp(-t/Ta) - b * exp(-t/Tb)\n");
	//if (!tag) printf(" --- forming f(i) = a * exp(-t/Ta)\n");
	Matrix f(n,1);
	for(int i = 1; i <= n ; i++) {
		//f(i,1) = funcX(valX[i - 1 +xStart], a, Ta, b, Tb);
		double x = valX[i - 1 +xStart];
		
		f(i,1) = a * exp(-x/Ta); 
		if(  tag ) f(i,1) += b * exp(-x/Tb);  
	}
	
	//printf(" --- forming F(i) = grad(f(i)) \n");
	Matrix F(n,p);
	for(int i = 1; i <= n ; i++) {
		double x = valX[i - 1 +xStart];
		F(i,1) = exp(-x/Ta);
		F(i,2) = a * x * exp(-x/Ta)/Ta/Ta;
		if( tag ) F(i,3) = exp(-x/Tb);
		if( tag ) F(i,4) = b * x * exp(-x/Tb)/Tb/Tb;
	}
	
	//printf(" --- cal. new parameters\n");
	Matrix Ft = Transpose(F); //printf("    Ft(%d,%d)\n", Ft.GetRows(), Ft.GetCols());
	Matrix FtF = Ft*F;        //printf("   FtF(%d,%d)\n", FtF.GetRows(), FtF.GetCols());
	
	Matrix CoVar;
	try{
		CoVar = Inv(FtF);  //printf(" CoVar(%d,%d)\n", CoVar.GetRows(), CoVar.GetCols());
	}catch( Exception err){
		if( info >= 1) printf("%s. #par=%d | Terminated.\n", err.msg, p);
		//FtF.Print();
		(output[4])(1,1) = ERROR;
		return output;
	}
	
	//CoVar.Print();
	
	Matrix dY = Y - f;    //printf("    dY(%d,%d)\n", dY.GetRows(), dY.GetCols());
	Matrix FtdY = Ft*dY;  //printf("  FtdY(%d,%d)\n", FtdY.GetRows(), FtdY.GetCols());
	
	Matrix par_old(p,1);
	par_old(1,1) = a;
	par_old(2,1) = Ta;
	if( tag ) par_old(3,1) = b;
	if( tag ) par_old(4,1) = Tb;
	
	Matrix dpar = CoVar * FtdY;  //printf("  dpar(%d,%d)\n", dpar.GetRows(), dpar.GetCols());
	
	Matrix par = par_old + dpar;
	
	//printf(" old par : "); Transpose(par_old).Print();
	//printf("  dpar   : "); Transpose(dpar).Print();
	//printf(" new par : "); Transpose(par).Print();
	
	Matrix SSR = Transpose(dY)*dY;
	
	int DF = n - p;
	
	//if( !tag) printf(" sqrt(SSR/DF) = %f \n", sqrt(SSR(1,1)/DF));
	//if(  tag) printf(" sqrt(SSR/DF) = %f \n", sqrt(SSR(1,1)/DF));
	
	
	double var = SSR(1,1) / DF;
	
	Matrix sigma(p,1);
	for( int i = 1; i <= p ; i++){
		sigma(i,1) = sqrt(var * CoVar(i,i));
	}
	
	//printf("   sigma : "); Transpose(sigma).Print();
	
	Matrix tDis(p,1);
	for( int i = 1; i <= p ; i++){
		tDis(i,1) = par(i,1)/sigma(i,1);
	}
	
	//printf("   t-dis : "); Transpose(tDis).Print();
	
	Matrix ApValue(p,1); // approximated p-value
	for( int i = 1; i <= p ; i++){
		ApValue(i,1) = cum_tDis30(- std::abs(tDis(i,1)));
	}
	
	//printf("A.p-Value : "); Transpose(ApValue).Print();
	
	//printf("-------------------------------------\n");
	
	output[0] = par;
	output[1] = dpar;
	output[2] = sigma;
	output[3] = tDis;
	output[4] = ApValue;
	
	return output;
	
}

Matrix* Fitting(int yIndex, int info, double a, double Ta, double b, double Tb){
	
	//info < 0; mo msg;
	//info = 0; only B-field and sol;
	//info = 1; + sigma 
	//info = 2; + p-value;
	//info = 3; + Regression;
	//info = 4; + Reg4

	double Yvalue = valY[yIndex];
	if( info >= 0) printf("=============== index : %3d, B field : %f ", yIndex, Yvalue);
	if( info >= 1) printf("\n");

	if( info >= 3) printf(" Regression of 4 parameters %d  ", 1 );
	Matrix* output = regression(1, yIndex, a, Ta, b, Tb, info); 
	Matrix sol = output[0];
	Matrix dpar = output[1];
	sol = output[0]; 
	
	double errFlag = (output[4])(1,1);

	int count = 2;
		
	while( errFlag != ERROR &&
		   std::abs(dpar(1,1)) > 0.01 && 
		   std::abs(dpar(2,1)) > 0.01 &&
		   std::abs(dpar(3,1)) > 0.01 && 
		   std::abs(dpar(4,1)) > 0.01 ){

		if( info >= 3) printf(" %d  ", count ++ );
		output = regression(1, yIndex, sol(1,1), sol(2,1), sol(3,1), sol(4,1), info); 
		sol = output[0]; 
		dpar = output[1];
		errFlag = (output[4])(1,1);
		if( errFlag == ERROR ) break;
	}

	
	if( info >= 3) if( errFlag != ERROR ) printf("| End.\n");
	if( info >= 4) {printf("         sol  : "); Transpose(sol).Print(); }
	if( info >= 4) {printf("        sigma : "); Transpose(output[2]).Print();}
	if( info >= 4) {printf("        t-dis : "); Transpose(output[3]).Print();}
	if( info >= 4) {printf("Appr. p-Value : "); Transpose(output[4]).Print();}

	Matrix ApValue = output[4];

	if( ApValue(1,1) > 0.05 ||
		ApValue(2,1) > 0.05 ||
		ApValue(3,1) > 0.05 ||
		ApValue(4,1) > 0.05 ){ //95% confident level

		if( info >= 3) {printf(" ******************** Result rejected, Appr. p-Value : "); Transpose(output[4]).Print();}
		count = 1;
		if( info >= 3) printf(" Regression of 2 parameters %d  ", count);
		output = regression(0, yIndex, a, Ta, 0, 100, info); 
		sol = output[0]; 
		dpar = output[1];

		errFlag = (output[4])(1,1);
		
		while( std::abs(dpar(1,1)) > 0.01 && 
			   std::abs(dpar(2,1)) > 0.01 &&
			   errFlag != ERROR ){
			count ++ ;
			if( info >= 3) printf(" %d  ", count);
			output = regression(0, yIndex, sol(1,1), sol(2,1), 0, 100, info); 
			sol = output[0]; 
			dpar = output[1];
			errFlag = (output[4])(1,1);
			if( errFlag == ERROR ) break;
		}
		if( info >= 3) printf("| End.\n");
	}


	if( info >= 3) printf("############# \n");
	if( info >= 0) {printf("         sol  : "); Transpose(sol).Print();}
	if( info >= 1) {printf("        sigma : "); Transpose(output[2]).Print();}
	//if( info >= 0) {printf("        t-dis : "); Transpose(output[3]).Print();}
	if( info >= 2) {printf("Appr. p-Value : "); Transpose(output[4]).Print();}
	
	return output;

}

void SaveFitResult(char* filename, int yIndex, Matrix * output){

	//output of data
	Matrix par = output[0];
	Matrix sigma = output[2];
	Matrix pValue = output[4];
	
	int p = par.GetRows();
	
	double a[4], s[4], pV[4];

	a[0] = par(1,1); s[0] = sigma(1,1); pV[0] = pValue(1,1);
	a[1] = par(2,1); s[1] = sigma(2,1); pV[1] = pValue(2,1);
	a[2] = 0       ; s[2] = 0         ; pV[2] = 0;
	a[3] = 0       ; s[3] = 0         ; pV[3] = 0;
	
	if( p == 4){
		a[2] = par(3,1); s[2] = sigma(3,1); pV[2] = pValue(3,1);
		a[3] = par(4,1); s[3] = sigma(4,1); pV[3] = pValue(4,1);
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
		fprintf(file, "\n");
	
		fclose(file);	
	}
	
	//delete file;
		
}


#endif
