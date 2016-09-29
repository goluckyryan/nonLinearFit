
#include "anaLibrary.h"
#include "anaFileIO.h"

using namespace std; 
 
int main(int argc, char *argv[]){ 
	
	if( argc != 2 && argc != 4 && argc != 6 && argc != 7){
		printf("usage: ./analaysis.o yindex (a Ta b Tb) lambda\n");
		printf("         yIndex = -1 ; loop all, minimum massage. \n");
		printf("      a Ta b Tb = are optional initial guesses. \n");
		return 1;
	}

	int yIndex = atoi(argv[1]);
	
	///defual initial guess value, since the algorithm converge quickly, 1 guess is usually OK.
	double  a =  20;
	double Ta =  20;
	double  b = -10;
	double Tb =  80;
	
	if( argc >= 4){
		a  = atof(argv[2]);
		Ta = atof(argv[3]);
	}
	if( argc >= 6){
		b  = atof(argv[4]);
		Tb = atof(argv[5]);
	}
	if( argc >= 7){
		lambda = atof(argv[6]);
	}
	
	///======================= get data
	char filename[100] = "20160725pentacene_pterphenyl.csv";
	getData(filename);
	
	///======================= save in dat file
	//char outfile[100] = "test.dat";
	//output(outfile);

	///====================== fitting 
	char savefile[100] = "FitResult.txt";
	char savefile_single[100] = "FitResult_single.txt";
	
	Matrix * output;
	
	printf ("init guess (a, Ta, b, Tb) = (%3.0f, %3.0f, %3.0f, %3.0f) \n", a, Ta, b, Tb);
	
	Matrix par0 (4,1);
	par0(1,1) = a;
	par0(2,1) = Ta;
	par0(3,1) = b;
	par0(4,1) = Tb;
	
	if( yIndex == -1){
		system("rm FitResult.txt");
		for( int i = 0; i < sizeY; i++){
			NonLinearFit(i, 0, par0);
			SaveFitResult(savefile, i);
		}
	}else{
		NonLinearFit(yIndex, 4, par0);
		SaveFitResult(savefile_single, yIndex);
	}
	
	///====================== gnuplot
	/*
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
	*/
	
	return 0; 
} 

