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
  

  char filename[100] = "20160725pentacene_pterphenyl.csv";
  printf("------------------------------\n");
  printf(" %s \n", filename);
  
  getData(filename);
  
  printf("------------------------------------------------------\n"); 
  Matrix temp = regression(60, 35, -30, 60);
  temp = regression(temp(1,1), temp(2,1), temp(3,1), temp(4,1));
  temp = regression(temp(1,1), temp(2,1), temp(3,1), temp(4,1));
  temp = regression(temp(1,1), temp(2,1), temp(3,1), temp(4,1));
  temp = regression(temp(1,1), temp(2,1), temp(3,1), temp(4,1));
  temp = regression(temp(1,1), temp(2,1), temp(3,1), temp(4,1));
  temp = regression(temp(1,1), temp(2,1), temp(3,1), temp(4,1));
  temp = regression(temp(1,1), temp(2,1), temp(3,1), temp(4,1));
   
  return 0; 
 
} 

/*========================================*/
