#ifndef AnaLibrary_H
#define AnaLibrary_H

using namespace std; 

double valX [1024];      // val of X or time
double valY [500];       // val of Y or B-field
double data [500][1024]; // val of data
int sizeX, sizeY;

int getData(char* filename){
	
  int multi = 3; //multiply the zval by 10^(multi)
	
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
	   double Zval = atof(val.c_str())*pow(10,multi); // multiplied by 10^6 times
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


double funcX(double x, double x_off, double a, double Ta, double b, double Tb){
	
	if ( x < 0){
		return 0;
	}else{
		return a * exp(-(x-x_off)/Ta) - b * exp(-(x-x_off)/Tb);
	}
}

double leastsq(const double * x, const double * y, double y_off, double x_off, double a, double Ta, double b, double Tb, int start, int end){
	double chisq = 0;
	
	int DF = end - start + 1 - 2;
	
	for( int i = start ; i < end; i++){
		double val = funcX(x[i], 0, a, Ta, b, Tb);	
		double diff = y[i] - y_off - val;
		chisq += pow(diff,2);
		//printf(" %4d, (%10.3f, %10.6f), %10.3f, %10.6f \n", i,  x[i], y[i], val, diff);
	}
	//printf(" chi-squared : %f \n", chisq);
	
	return pow(chisq/DF,0.5);
}

double* fitX(const double * xVal, const double yVal[], int start, int end){
	
	double * output = new double [5]; //0 = a, 1 = Ta, 2 = b, 3 = Tb, 4 = chi-squared
	
	//initailized
	for (int i = 0; i <= 5 ; i++){
		output[i] = 0.0;
	}
		
	//get the mean, variance x < 0	
	double mean = 0;
	for ( int i = 0; i < start-20; i++){
		mean += yVal[i];
	}
	mean = mean / (start-20);
	
	double sigma =0 ;
	for ( int i = 0; i < start-20; i++){
		sigma += pow(yVal[i]-mean,2);
	}
	sigma = pow(sigma/(start-20),0.5);
	
	printf("noise mean: %f, sigma:%f \n", mean, sigma);
	
	//find fit by seraching
	double chisq=0, a, Ta, b, Tb;
	b = 0;
	Tb = 10;
	
	double minChisq = 1e7;
	double mina, minTa, minb, minTb;
	
	double maxa = 50.;
	double stepa = 4;
	double stepT = 4;
	
	printf(" start: %d (%f), end: %d (%f)\n", start, xVal[start], end, xVal[end]); 
	
	for ( a = 0 ; a < 50 ; a += stepa){
		for (Ta = 50; Ta > 0; Ta -= stepT){
			for ( b = 0 ; b < maxa ; b += stepa){
				for (Tb = 100; Tb > Ta; Tb -= stepT){
					chisq = leastsq(xVal, yVal, mean, 0, a, Ta, b, Tb, start, end);
					if( chisq < minChisq) {
						minChisq = chisq;
						mina  = a;
						minTa = Ta;
						minb  = b;
						minTb = Tb;
					}
					//printf("(a, Ta, b, Tb): %4f, %4f, %6f, %4f | chi^2 : %.7f \n", a, Ta, b, Tb, chisq);
				}
			}
		}
	}
	
	//printf("(a, Ta, b, Tb): %6f, %4f, %6f, %4f | chi^2 : %.7f \n", mina, minTa, minb, minTb, minChisq);

	//TODO add timer

	output[0] = mina;
	output[1] = minTa;
	output[2] = minb;
	output[3] = minTb;
	output[4] = minChisq/sigma;
	
	return output;
	
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


#endif
