//#include "anaLibrary.h"
#include "analysis.h"
#include <vector>
 
using namespace std; 
 
int main(int argc, char *argv[]){ 
	
	Analysis ana;
	vector<double> x (5);
	x[0] = 0;
	x[1] = 1;
	x[2] = 2;
	x[3] = 3;
	x[4] = 4;

	printf("%ld，%f\n", x.size(), x[0]);
	
	ana = Analysis(x,x);
		
	printf("%f \n", ana.GetMean());
	
	
	
	/*********
	if( argc != 2){
		printf("usage: ./test.o yindex\n");
		return 1;
	}

	int yIndex = atoi(argv[1]);


	char filename[100] = "20160725pentacene_pterphenyl.csv";
	printf("------------------------------\n");
	printf(" %s \n", filename);

	getData(filename);

	if( yIndex == -1){
		for( int i = 120; i <= 150; i++){
			Fitting(i);
		}
	}else{
		Fitting(yIndex);
	}
	//for( int i = 20; i <= 30; i++){
	//	Fitting(i);
	//}
	
	**********/ 
	
	return 0; 
} 

