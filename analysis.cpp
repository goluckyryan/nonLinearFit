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
//#include "gnuplot_i.hpp"
 
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
	int col = 100;
	printf(" Y : %f , ", valY[col]);
	fitX( valX, data[col]);
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
  sprintf(plot_cmd, "gnuplot -e \"Col=%d\" plot.gp", col);
  system(plot_cmd);
  //}
  
  printf("------------------------------------------------------\n"); 
   
  return 0; 
 
} 

/*========================================*/
