
#include "anaLibrary.h"
#include "anaFileIO.h"

using namespace std; 

void Shift(int n,int * data){
	int m = n/2;
	int temp = data[n-1];
	for(int i = n; i > 0; i--){
		data[i] = data[i-1];
	}
	data[0] = temp;
}

int main(int argc, char *argv[]){ 
		
	if( argc != 3 && argc != 5 && argc != 7 && argc != 8){
		printf("usage: ./analaysis fileName yindex (a Ta b Tb lambda)\n");
		printf("     yIndex = -1 ; loop all, minimum massage. \n");
		return 1;
	}
	char *filename = argv[1]; // = "20160725pentacene_pterphenyl.csv";
	int yIndex = atoi(argv[2]);
	
	///defual initial guess value, since the algorithm converge quickly, 1 guess is usually OK.
	double  a =  20;
	double Ta =  20;
	double  b = -10;
	double Tb =  80;
	
	lambda = -1;

	if( argc >= 5){
		a  = atof(argv[3]);
		Ta = atof(argv[4]);
	}
	if( argc >= 7){
		b  = atof(argv[5]);
		Tb = atof(argv[6]);
	}
	if( argc >= 8){
		lambda = atof(argv[7]);
	}
	
	///======================= get data
	getData(filename);
	
	///======================= save in dat file
	char outfile[100] = "test.dat";
	output(outfile);

	///====================== fitting 
	char savefile[400];
	memcpy (savefile, filename, strlen(filename)-4);
	printf("%s\n", savefile);
	strcat (savefile, "_cmdfit.txt");
	printf("%s\n", savefile);
	
	char savefile_single[100] = "FitResult_single.txt";
	
	Matrix * output;
	
	printf ("init guess (a, Ta, b, Tb) = (%3.0f, %3.0f, %3.0f, %3.0f) \n", a, Ta, b, Tb);
	
	Matrix par0 (4,1);
	par0(1,1) = a;
	par0(2,1) = Ta;
	par0(3,1) = b;
	par0(4,1) = Tb;
	
	if( yIndex == -1){
		printf(" save as : %s \n", savefile);
		for( int i = 0; i < sizeY; i++){
			NonLinearFit(i, 0, par0);
			SaveFitResult(savefile, i);
		}
	}else{
		// for( int i = 0; i < sizeY; i++) printf(" %3d, %7.3f \n", i, valY[i]);
		NonLinearFit(yIndex, 4, par0);
		SaveFitResult(savefile_single, yIndex);
	}
	
	///====================== gnuplot
	char plot_cmd[100];
	if( yIndex == -1 ){
		
		//sprintf(plot_cmd, "gnuplot -e \"plot '%s' u 2:3 w lp \" -p", savefile );
		//system(plot_cmd);
		system("gnuplot \"plot_Loop.gp\" -p");
	}else{
		double a, Ta, b, Tb;
		a  = sol(1,1);
		Ta = sol(2,1);
		b  = 0;
		Tb = 100;
		if( sol.GetRows() == 4){
			b  = sol(3,1);
			Tb = sol(4,1);
		}
		sprintf(plot_cmd, "gnuplot -e \"Col=%d;a=%f;Ta=%f;b=%f;Tb=%f;startX=%d;endX=%d\" plot.gp -p",
                    yIndex, a, Ta, b, Tb, 195, sizeX);
		system(plot_cmd);
	}
	/**/
	
	return 0; 
} 

