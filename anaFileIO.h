#ifndef AnaFileIO_H
#define AnaFileIO_H

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

using namespace std; 

int getData(char* filename){
	
  int multi_Z = 3; //multiply the zval by 10^(multi_Z)
	
  string val;
  ifstream file;
  
  printf("------------------------------\n");
  printf(" %s \n", filename);
  //------------ get 1st line, determine how many set of data, get valY
  file.open(filename);
  
  int numCol = 0, yIndex = -1;

  size_t pos;
  while (file.good()){
	getline(file, val, ','); //get it cell by cell
	pos = -1;
	pos = val.find_last_of("_"); //get the last "_"
	//printf("%lu,  %s\n", pos, val.c_str());
	if( pos == -1) {
		break;
	}
			
	// get the Y data
	if( numCol % 2 == 0){
		string strY = val.substr(pos+1, 5);
		yIndex ++;
		valY[yIndex] = atof(strY.c_str());
		//printf(" %s, %f \n", strY.c_str(), valY[yIndex]);
	}

	numCol ++;
  }
 
  double stepY = valY[1]-valY[0];
  sizeY = yIndex + 1 ;
	
	printf(" %d, %f \n", sizeY, stepY);

  //------------ get number of Row
  file.seekg(0);
  sizeX = 0;
  while (file.good()){
   sizeX ++;
   getline (file, val);
  }
  sizeX = sizeX - 2 ;
  file.close();
  
  printf(" %d\n", sizeX);
  
  
  //------------ get data  
  file.open(filename);
  if( file.good()) getline(file, val); // skip the 1st line by getline
  
  int xIndex = 0, cols = 0;
  yIndex = 0; 
  double maxZ = 0;
  double minZ = 0;
  
  printf(" numCol : %d, IsEnd : %d, IsGood : %d\n", numCol, file.eof(), file.good());
  
  while (file.good()){
   cols ++; 

   if( cols % numCol !=0 ){
	   getline (file, val, ',');  
   }else{
	   getline (file, val); 
   }

   //get valX
   if( cols % numCol == 1){
		valX[xIndex] = atof(val.c_str())*1e6; // in us
		///printf("%10d, %3d, %2d|  %s , %f \n", cols, numCol, cols % numCol , val.c_str(), valX[xIndex]); 
   }
   
   // get data
   if( cols % 2 == 0){
	   double Zval = atof(val.c_str())*pow(10,multi_Z); // multiplied by 10^6 times
	   data[yIndex][xIndex] = Zval;
	   if( Zval > maxZ) maxZ = Zval;
	   if( Zval < minZ) minZ = Zval;
	   ///printf("*%10d, %3d, %3d, %s, %f\n", cols, cols% numCol, yIndex, val.c_str(), data[yIndex][xIndex]);
	   yIndex ++;
   }
   
   if( cols % numCol == 0) {
	   xIndex ++ ;
	   yIndex = 0;
	   cols = 0;
   }
   
   if( xIndex > sizeX ) break;
  }
  
  file.close();
  
  double stepX = valX[1]-valX[0];
  
  printf(" Y : (%.3f, %.3f), step : %.3f , sizeY : %d \n", valY[0], valY[sizeY-1], stepY , sizeY); 
  printf(" X : (%.3f, %.3f), step : %.3f , sizeX : %d \n", valX[0], valX[sizeX-1], stepX , sizeX); 
  printf(" Z : (%.5f, %.5f)\n", minZ, maxZ);
  printf("------------------------------\n");
  return 0;
}

int output(char* filename){
	FILE * file;
	file = fopen (filename, "w+");
	
	//output of valY
	fprintf(file, "%*s,", 10, "");
	for (int i = 0; i < sizeY; i++){
		fprintf(file, "%10.5f,", valY[i]);
	} 
	fprintf(file, "\n");
	
	//output of data
	for (int i = 0; i < sizeX; i++){
		fprintf(file, "%10.3f,", valX[i]);
		for (int j = 0; j < sizeY; j++){
			fprintf(file, "%10.2f,", data[j][i]);
		} 
		fprintf(file, "\n");
	}
	
	fclose(file);
	
	printf(" saved in %s \n", filename);
	
	return 0;
}

void SaveFitResult(char* filename, int yIndex){

	double fitvar = SSR/nDF;
	
	double SampleVar = sVar;
	double redchisq = fitvar/sVar;
	
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
		//for ( int i = 0; i < 4 ; i++) fprintf(file, "%8.3f, ", pV[i]);
		fprintf(file, "%8.4f\n", redchisq);
	
		fclose(file);	
	}
	
	//delete file;
		
}


#endif
