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
#include "anaLibrary.h"
 
using namespace std; 
 
int main(int argc, char *argv[]){ 
  time_t Tstart=time(0); 
 
  //if(argc != 9) { 
  //  exit(0); 
  //} 
  
  char filename[100] = "20160725pentacene_pterphenyl.csv";
  printf("------------------------------\n");
  printf(" %s \n", filename);

  //####################### read input
  getData(filename);

 
  //####################### fitting
  //for ( int i = 0; i < sizeY; i++){
	int col = 140, startX = 195, endX = 1000;
	printf("------ Y : %f  \n", valY[col]);
	double *par = fitX( valX, data[col], startX, endX);
	
	printf("(a, Ta, b, Tb): %6f, %4f, %6f, %4f | chi : %.7f\n", 
		par[0], par[1], par[2], par[3], par[4]);
	
  //}
  
  //TODO linearize the non-linear fit, get the fitting ANOVA
  //TODO include signal noise level to check the fitting.
  
  //####################### output file 
  //TODO save fit result
  
  //char outfile[100] = "test.dat";
  //output(outfile); //save data


  //####################### display result 
  time_t Tend=time(0); 
  //TODO call external program to plot result.

  char plot_cmd[100];
  
  //for( int i = 1 ; i <= sizeY; i++){
  sprintf(plot_cmd, "gnuplot -e \"Col=%d;a=%f;Ta=%f;b=%f;Tb=%f;startX=%d;endX=%d\" plot.gp",
                    col, par[0],par[1],par[2],par[3], startX, endX);
  system(plot_cmd);
  //}
  
  printf("------------------------------------------------------\n"); 
   
  return 0; 
 
} 

/*========================================*/
