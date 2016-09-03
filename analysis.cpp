
#include "anaLibrary.h"
 
using namespace std; 
 
int main(int argc, char *argv[]){ 
	
	if( argc != 2 && argc != 4 && argc != 6){
		printf("usage: ./analaysis.o yindex (a Ta b Tb)\n");
		printf("         yIndex = -1 ; loop from 120 to 150 \n");
		printf("      a Ta b Tb = are optional initial guess \n");
		return 1;
	}

	int yIndex = atoi(argv[1]);
	
	///defual initial guess value, since the algorithm converge quickly, 1 guess is usually OK.
	double  a =  40;
	double Ta =  35;
	double  b = -30;
	double Tb =  60;
	
	if( argc >= 4){
		a  = atof(argv[2]);
		Ta = atof(argv[3]);
	}
	if( argc >= 6){
		b  = atof(argv[4]);
		Tb = atof(argv[5]);
	}

	///======================= get data
	char filename[100] = "20160725pentacene_pterphenyl.csv";
	printf("------------------------------\n");
	printf(" %s \n", filename);
	getData(filename);

	///====================== fitting 
	printf ("init guess (a, Ta, b, Tb) = (%3.0f, %3.0f, %3.0f, %3.0f) \n", a, Ta, b, Tb);
	if( yIndex == -1){
		for( int i = 120; i <= 150; i++){
			Fitting(i, 0, a, Ta, b, Tb);
		}
	}else{
		Fitting(yIndex,4, a, Ta, b, Tb);
	}
	
	///====================== save fit result
	
	
	///====================== gnuplot
	
	
	return 0; 
} 

