
#include "anaLibrary.h"
 
using namespace std; 
 
int main(int argc, char *argv[]){ 
	
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
	
	return 0; 
} 

