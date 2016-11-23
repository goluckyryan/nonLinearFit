#include "fileio.h"

FileIO::FileIO()
{
    Initialize();
}

FileIO::FileIO(QString filePath){
    Initialize();

    this->filePath = filePath;
    myfile = new QFile(filePath);
    myfile->open(QIODevice::ReadOnly);

}

void FileIO::Initialize(){
    myfile = NULL;
    outfile = NULL;
    zData = NULL;
    fZDataA = NULL;
    fZDataP = NULL;
    backUpData = NULL;

    colwise = 0;
    xMax = 0;
    xMin = 0;
    yMax = 0;
    yMin = 0;
    zMax = 0;
    zMin = 0;
    openState = 0;
    isOutFileOpened = 0;
    yRevered = 0;
    hadBG = 0;
    multi = 0;

    xSize = 0;
    ySize = 0;
}

FileIO::~FileIO(){
    delete myfile;
    if( outfile != NULL ) delete outfile;
    if( zData != NULL ) delete [] zData;
    if( backUpData != NULL ) delete [] backUpData;
    if( fZDataA != NULL ) delete [] fZDataA;
    if( fZDataP != NULL ) delete [] fZDataP;
}

void FileIO::OpenSaveFile(){
    if( openState == 0 ) return;
    if( isOutFileOpened == 1 ) return;
    int lenght = filePath.length();
    QString outfilePath = filePath.left(lenght -4);
    outfilePath += "_fit.txt";
    outfile = new QFile(outfilePath);
    outfile->open(QIODevice::Append );
    SendMsg("Fit result will save to :");
    SendMsg(outfilePath);
    isOutFileOpened = 1;
}

void FileIO::OpenCSVData(){

    this->colwise = 1;

    myfile->seek(0);

    QTextStream stream(myfile);
    QString line;

    // get number of rows and cols;
    xSize = -1;
    while(stream.readLineInto(&line)){
        xSize ++;
        QStringList lineList = line.split(",");

        if( xSize == 0){ // get yDatax
            ySize = lineList.size()/2;
        }
    }

    if( ySize < 1 || xSize < 1) {
        SendMsg("!!!! Invalide file structure.");
        return;
    }

    zData = new QVector<double> [ySize];
    backUpData = new QVector<double> [ySize];
    fZDataA = new QVector<double> [ySize];
    fZDataP = new QVector<double> [ySize];
    double ** z ;
    z = new double *[ySize];
    for(int i = 0; i < ySize; i++){
        z[i] = new double [xSize];
    }

    //get data
    multi = 3;
    yData.clear();
    xData.clear();
    myfile->seek(0);
    int rows = 0;
    while(stream.readLineInto(&line) && rows <= xSize+1){
        rows ++;
        QStringList lineList = line.split(",");
        if( rows == 1){ // get yDatax
            for( int i = 1 ; i < lineList.size() ; i++ ){
                if( i % 2 == 1) {
                    double temp = ExtractYValue(lineList[i]);
                    yData.push_back(temp) ; // get data from string.
                }
            }
        }else{
            xData.push_back( (lineList[0]).toDouble() * 1e6);
            int yIndex = 0;
            for( int i = 1 ; i < lineList.size() ; i++ ){
                if( i % 2 == 1){
                    z[yIndex][rows-2] = (lineList[i]).toDouble() * pow(10,3);
                    yIndex ++;
                }
            }
        }
    }

    //transpose the zData to row-wise
    zMin = z[0][0];
    zMax = z[0][0];
    for( int j = 0; j < ySize ; j ++){
        zData[j].clear();
        backUpData[j].clear();
        for(int i = 0; i < xSize ; i++){
            zData[j].push_back(z[j][i]);
            backUpData[j].push_back(z[j][i]);
            if( z[j][i] > zMax) zMax = z[j][i];
            if( z[j][i] < zMin) zMin = z[j][i];
        }
    }

    delete z;

    //check
    qDebug("X: %d , %d", xData.size(), xSize);
    qDebug("Y: %d , %d", yData.size(), ySize);

    xMin = FindMin(xData);
    xMax = FindMax(xData);

    yMin = FindMin(yData);
    yMax = FindMax(yData);

    if(yData[0] > yData[1]) yRevered = 1;

    openState = 1;

    QString msg;
    msg.sprintf("X:(%7.3f, %7.3f) sizeX:%d",xMin, xMax, xSize);
    SendMsg(msg);

    msg.sprintf("Y:(%7.3f, %7.3f) sizeY:%d",yMin, yMax, ySize);
    SendMsg(msg);

    msg.sprintf("Z:(%7.3f, %7.3f)",zMin, zMax);
    SendMsg(msg);

    myfile->close();

    CalMeanVector();

}

void FileIO::OpenTxtData_col() // great problem in this function. not updated for long time.
{
    this->colwise = 0;

    myfile->seek(0);

    QTextStream stream(myfile);
    QString line;

    // get number of rows and cols;
    xSize = -1;
    while(stream.readLineInto(&line)){
        xSize ++;
        QStringList lineList = line.split(",");

        if( xSize == 0){ // get yDatax
            ySize = lineList.size()-1;
        }
    }

    if( ySize < 1 || xSize < 1) {
        SendMsg("!!!! Invalide file structure.");
        return;
    }

    // get Data
    zData = new QVector<double> [ySize];
    backUpData = new QVector<double> [ySize];
    double ** z ;
    z = new double *[ySize];
    for(int i = 0; i < ySize; i++){
        z[i] = new double [xSize];
    }

    myfile->seek(0);
    int rows = 0;
    while(stream.readLineInto(&line)){
        rows ++;
        QStringList lineList = line.split(",");
        if( rows == 1){ // get xDatax
            for( int i = 1 ; i < lineList.size() ; i++ ){
                double temp = ExtractYValue(lineList[i]);
                yData.push_back(temp);
            }
        }else{
            xData.push_back((lineList[0]).toDouble()) ;
            int yIndex = 0;
            for( int i = 1 ; i < lineList.size() ; i++ ){
                z[yIndex][rows-2] = (lineList[i]).toDouble();
                yIndex++;
            }
        }
    }

    //transpose the zData to row-wise
    zMin = z[0][0];
    zMax = z[0][0];
    for( int j = 0; j < ySize ; j ++){
        zData[j].clear();
        backUpData[j].clear();
        for(int i = 0; i < xSize ; i++){
            zData[j].push_back(z[j][i]);
            backUpData[j].push_back(z[j][i]);
            if( z[j][i] > zMax) zMax = z[j][i];
            if( z[j][i] < zMin) zMin = z[j][i];
        }
    }

    delete z;

    qDebug("X: %d , %d", xData.size(), xSize);
    qDebug("Y: %d , %d", yData.size(), ySize);

    xMin = FindMin(xData);
    xMax = FindMax(xData);

    yMin = FindMin(yData);
    yMax = FindMax(yData);

    if(yData[0] > yData[1]) yRevered = 1;

    openState = 1;

    QString msg;
    msg.sprintf("X:(%7.3f, %7.3f) sizeX:%d",xMin, xMax, xSize);
    SendMsg(msg);

    msg.sprintf("Y:(%7.3f, %7.3f) sizeY:%d",yMin, yMax, ySize);
    SendMsg(msg);

    msg.sprintf("Z:(%7.3f, %7.3f)",zMin, zMax);
    SendMsg(msg);

    myfile->close();

    CalMeanVector();
}

void FileIO::OpenTxtData_row(){
    this->colwise = 0;

    myfile->seek(0);

    QTextStream stream(myfile);
    QString line;

    //============= get number of rows and cols;
    ySize = -1;
    while(stream.readLineInto(&line)){
        ySize ++;
        QStringList lineList = line.split(",");

        if( ySize == 0){ // get yDatax
            xSize = lineList.size()-1;
        }
    }

    if( ySize < 1 || xSize < 1) {
        SendMsg("!!!! Invalide file structure.");
        return;
    }

    //============ get Data
    zData = new QVector<double> [ySize];
    backUpData = new QVector<double> [ySize];
    fZDataA = new QVector<double> [ySize];
    fZDataP = new QVector<double> [ySize];

    myfile->seek(0);
    int rows = 0;
    int yIndex = 0;
    while(stream.readLineInto(&line)){
        rows ++;
        QStringList lineList = line.split(",");

        if( rows == 1){ // get xDatax
            for( int i = 1 ; i < lineList.size() ; i++ ){
                xData.push_back((lineList[i]).toDouble()) ;
            }
        }else{
            double temp = ExtractYValue(lineList[0]);
            yData.push_back(temp);
            zData[yIndex].clear();
            backUpData[yIndex].clear();
            for( int i = 1 ; i < lineList.size() ; i++ ){
                temp = (lineList[i]).toDouble();
                zData[yIndex].push_back(temp);
                backUpData[yIndex].push_back(temp);
                if( i == 1 && rows == 2) {
                    zMax = temp;
                    zMin = temp;
                }
                if( temp > zMax ) zMax = temp;
                if( temp < zMin ) zMin = temp;
            }
            yIndex++;
        }
    }

    //qDebug("X: %d , %d", xData.size(), xSize);
    //qDebug("Y: %d , %d", yData.size(), ySize);
    //qDebug("zMax: %f, zMin: %f", zMax, zMin);

    //=========== cal the rescale factor
    RescaleData();

    //============== check is BG data exist by checking Hall probe volatge,
    QVector<double> newYData;
    for( int i = 1; i < ySize; i++){
        newYData.push_back(yData[i]);
    }

    //if Hall probe volatge smaller than 3 mV
    int misIndex = -1;
    if( fabs(yData[0]) < 3 ) misIndex = 1;

    if( misIndex == 1) {
        hadBG = 1;
        for( int i = 1; i < ySize; i++){
            newYData.push_back(yData[i]);
        }
    }

    yMin = FindMin(newYData);
    yMax = FindMax(newYData);

    //=========== check is data revered.
    if(newYData[0] > newYData[1]) yRevered = 1;

    //=========== the open file complete, show message
    openState = 1;

    QString msg;
    msg.sprintf("X:(%7.3f, %7.3f) sizeX:%d",xMin, xMax, xSize);
    SendMsg(msg);
    msg.sprintf("Y:(%7.3f, %7.3f) sizeY:%d",yMin, yMax, ySize);
    SendMsg(msg);
    if( hadBG ){
        msg.sprintf("BG data exist. At yIndex = %d", misIndex-1);
        SendMsg(msg);
    }
    msg.sprintf("Z:(%7.3f, %7.3f), multiplied : 10^%d",zMin, zMax, multi);
    SendMsg(msg);
    myfile->close();

    //=========== calculate mean vector for mean correction, and estimated c-parameter
    CalMeanVector();
}

void FileIO::SaveFitResult(Analysis *ana)
{
    QTextStream stream(outfile);
    QString text, tmp;
    int p = ana->GetParametersSize();

    QVector<QString> header, eheader;

    header.push_back("a"); eheader.push_back("e(a)");
    header.push_back("Ta"); eheader.push_back("e(Ta)");
    if( p == 3){
        header.push_back("c"); eheader.push_back("e(c)");
    }else if( p == 4){
        header.push_back("b"); eheader.push_back("e(b)");
        header.push_back("Tb"); eheader.push_back("e(Tb)");
    }else if( p == 5){
        header.push_back("b"); eheader.push_back("e(b)");
        header.push_back("Tb"); eheader.push_back("e(Tb)");
        header.push_back("c"); eheader.push_back("e(c)");
    }

    //set header
    if( outfile->pos() == 0){

        text.sprintf("%5s, %8s,","yIndex", "yValue");
        for( int i = 0; i <p ; i++){
            tmp.sprintf(" %8s,", header[i].toStdString().c_str());
            text.append(tmp);
        }
        for( int i = 0; i <p ; i++){
            tmp.sprintf(" %8s,", eheader[i].toStdString().c_str());
            text.append(tmp);
        }
        tmp.sprintf(" %8s, %8s, %8s\n","SSR", "DF", "chi-sq/ndf");
        text.append(tmp);
        stream << text;
    }

    tmp.sprintf("%5d, %8.4f, ", ana->GetYIndex(), ana->GetDataYValue());
    text = tmp;

    QVector<double> sol = ana->GetParameters();

    for( int i = 0; i < p ; i++){
        tmp.sprintf("%8.4f, ", sol[i]);
        text += tmp;
    }

    QVector<double> error = ana->GetParError();
    for( int i = 0; i < p ; i++){
        tmp.sprintf("%8.4f, ", error[i]);
        text += tmp;
    }

    double chisq = ana->GetFitVariance()/ana->GetSampleVariance();
    tmp.sprintf("%8.4e, %5d, %8.5f \n", ana->GetSSR(), ana->GetNDF(), chisq);
    text += tmp;

    SendMsg("fit result saved.");
    SendMsg(text);
    stream << text;
}

void FileIO::SaveSingleXCVS()
{
    // the simplified txt can be used for Analysis::Gnufit
    this->cvsFilePath = this->filePath;
    cvsFilePath.chop(3);
    cvsFilePath.append("cvs");
    //this->simFilePath = OPENPATH;
    //this->simFilePath.append("test.dat");

    QFile out(cvsFilePath);
    out.open( QIODevice::WriteOnly );
    QTextStream stream(&out);
    QString str, tmp;

    //output y-value (B-value)
    tmp.sprintf("%*s,", 8, ""); str = tmp;
    for(int i = 0; i < ySize; i++){
        tmp.sprintf("%10.4f,", yData[i]); str += tmp;
    }
    str += "\n";
    stream << str;

    //output x and z
    for(int i = 0; i < xSize; i++){
        tmp.sprintf("%8.3f,", xData[i]); str = tmp;
        for(int j = 0; j < ySize-1; j++){
            tmp.sprintf("%10.4f,", zData[j][i]); str += tmp;
        }
        tmp.sprintf("%10.4f\n", zData[ySize-1][i]); str += tmp;
        stream << str;
    }

    out.close();

    QString msg;
    msg.sprintf("Saved a CVS file as : %s ", cvsFilePath.toStdString().c_str());
    SendMsg(msg);

}

void FileIO::RestoreData()
{
    SendMsg("restoring Data.");
    for( int yIndex = 0; yIndex < ySize; yIndex++){
        zData[yIndex].clear();
        for(int xIndex = 0; xIndex < xSize; xIndex++){
            zData[yIndex].push_back(backUpData[yIndex][xIndex]);
        }
    }

    CalMeanVector();
}

void FileIO::SubstractData(int yIndex)
{
    QString msg;
    msg.sprintf("Using the %d(th)-data as background.", yIndex);
    SendMsg(msg);
    QVector<double> ref = zData[yIndex];

    for( int y = 0; y < ySize; y++){
        QVector<double> temp = zData[y];
        zData[y].clear();
        for(int x = 0; x < xSize; x++){
            zData[y].push_back(temp[x] - ref[x]);
        }
    }

    CalMeanVector();
}

void FileIO::MeanCorrection()
{
    SendMsg("Mean correction.");
    for(int xIndex = 0; xIndex < xSize; xIndex++){
        for(int yIndex = 0; yIndex < ySize; yIndex++){
            double mean = zMean[yIndex];
            zData[yIndex][xIndex] = zData[yIndex][xIndex] - mean;
        }
    }

    CalMeanVector();
}

void FileIO::MovingAvg(int n)
{
    if( n == -1 ) {
        return;
    }
    if( n % 2 == 0) return;

    QString msg;
    msg.sprintf("Moving Average n = %d.", n);
    SendMsg(msg);

    int m = (n-1)/2;
    for(int yIndex = 0; yIndex < ySize; yIndex++){
        QVector<double> temp = zData[yIndex];

        for(int xIndex = 0; xIndex < xSize; xIndex++){
            double mean = 0;
            for( int k = xIndex-m; k <= xIndex+m; k++ ){
                if( k < 0 || k >= xSize) continue;
                mean += temp[k]/n;
            }
            zData[yIndex][xIndex] = mean;
        }
    }

    CalMeanVector();
}

void FileIO::FouierForward()
{
    SendMsg("Fouier Transform - Forward.");

    fftw_complex *in, *out;
    in = (fftw_complex* ) fftw_alloc_complex(ySize*xSize*sizeof(fftw_complex));
    out = (fftw_complex* ) fftw_alloc_complex(ySize*xSize*sizeof(fftw_complex));

    fftw_plan plan;
    plan = fftw_plan_dft_2d(ySize, xSize, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    for( int i = 0; i < ySize ; i++){
        for( int j = 0; j < xSize ; j++){
            in[i*xSize + j][0] = zData[i][j];
            in[i*xSize + j][1] = 0.;
        }
    }

    fftw_execute(plan); //TODO: somehow it crashed when data is too big.

    for( int i = 0; i < ySize; i++){
            fZDataA[i].clear();
            fZDataP[i].clear();
        for(int j = 0; j < xSize; j++){
            fZDataA[i].push_back(out[i*xSize + j][0] / xSize / ySize);
            fZDataP[i].push_back(out[i*xSize + j][1] / xSize / ySize);
        }
    }

    fftw_destroy_plan(plan);
    fftw_free(out);
    fftw_free(in);

    SendMsg("Fouier Transform - Forward. Done.");

    SwapFFTData(1);

}

void FileIO::FouierForwardSingle(int yIndex)
{
    SendMsg("Single Fouier Transform - Forward.");

    fftw_complex *in, *out;
    in = (fftw_complex* ) fftw_alloc_complex(xSize*sizeof(fftw_complex));
    out = (fftw_complex* ) fftw_alloc_complex(xSize*sizeof(fftw_complex));

    fftw_plan plan;
    plan = fftw_plan_dft_1d(xSize, in, out, FFTW_FORWARD, FFTW_ESTIMATE);


    for( int j = 0; j < xSize ; j++){
        in[j][0] = zData[yIndex][j];
        in[j][1] = 0.;
    }

    fftw_execute(plan);

    for(int j = 0; j < xSize; j++){
        qDebug("%d (%f, %f)", j, out[j][0], out[j][1]);
    }


    fftw_destroy_plan(plan);
    fftw_free(out);
    fftw_free(in);

    SendMsg("Single Fouier Transform - Forward. Done.");
}

void FileIO::FouierBackwardSingle(int yIndex)
{
    SendMsg("Single Fouier Transform - Backward.");

    fftw_complex *in, *out;
    in = (fftw_complex* ) fftw_alloc_complex(xSize*sizeof(fftw_complex));
    out = (fftw_complex* ) fftw_alloc_complex(xSize*sizeof(fftw_complex));

    fftw_plan plan;
    plan = fftw_plan_dft_1d(xSize, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);

    for( int j = 0; j < xSize ; j++){
        in[j][0] = zData[yIndex][j];
        in[j][1] = 0.;
    }

    fftw_execute(plan);

    for(int j = 0; j < xSize; j++){
        qDebug("%d (%f, %f)", j, out[j][0], out[j][1]);
    }


    fftw_destroy_plan(plan);
    fftw_free(out);
    fftw_free(in);

    SendMsg("Single Fouier Transform - Backward. Done.");
}

void FileIO::FouierBackward()
{
    SwapFFTData(0);

    SendMsg("Fouier Transform - Backward.");

    fftw_complex *in, *out;
    in = (fftw_complex* ) fftw_alloc_complex(ySize*xSize*sizeof(fftw_complex));
    out = (fftw_complex* ) fftw_alloc_complex(ySize*xSize*sizeof(fftw_complex));

    for( int i = 0; i < ySize ; i++){
        for( int j = 0; j < xSize ; j++){
            if( i == 0 ){
                in[i*xSize + j][0] = 0;
                in[i*xSize + j][1] = 0;
            }else{
                in[i*xSize + j][0] = fZDataA[i][j];
                in[i*xSize + j][1] = fZDataP[i][j];
            }

        }
    }

    fftw_plan plan;
    plan = fftw_plan_dft_2d(ySize, xSize, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);

    fftw_execute(plan);

    for( int i = 0; i < ySize; i++){
            zData[i].clear();
        for(int j = 0; j < xSize; j++){
            zData[i].push_back(out[i*xSize + j][0]);
        }
    }

    fftw_destroy_plan(plan);
    fftw_free(out);
    fftw_free(in);

    SendMsg("Fouier Transform - Backward. Done.");
}

QVector<double> FileIO::Shift(QVector<double> list, int d)
{
    int n = list.length();
    QVector<double> temp = list.mid(n-d);
    for( int i = n-1; i > d-1; i-- ){
        list[i] = list[i-d];
    }
    for( int i = 0; i < temp.length(); i++){
        list[i] =  temp[i];
    }
    return list;
}

void FileIO::SwapFFTData(bool dir)
{
    // dir = 1 , forward swap; dir = 0, backward

    if( dir == 1){
        SendMsg("Swap data.");
    }else{
        SendMsg("UnSwap data.");
    }

    // Swap X-data
    int d = 0;
    if( xSize % 2 == 0){
        d = xSize /2;
    }else{
        d = (xSize-1)/2;
        if( dir == 0 ) d = xSize - d;
    }
    for(int i = 0; i < ySize ; i++){
        fZDataA[i] = Shift(fZDataA[i], d);
        fZDataP[i] = Shift(fZDataP[i], d);
    }

    // Swap Y-data
    if( ySize % 2 == 0){
        d = ySize /2;
    }else{
        d = (ySize-1)/2;
        if( dir == 0 ) d = ySize - d;
    }
    QVector<double> *tempA = new QVector<double> [d];
    QVector<double> *tempP = new QVector<double> [d];
    for(int i = 0 ; i < d; i++){
        tempA[i] = fZDataA[i+ySize-d];
        tempP[i] = fZDataP[i+ySize-d];
    }

    for(int i = ySize-1 ; i > d-1; i--){
        fZDataA[i].clear();
        fZDataP[i].clear();
        fZDataA[i] = fZDataA[i-d];
        fZDataP[i] = fZDataP[i-d];
    }
    for(int i = 0 ; i < d; i++){
        fZDataA[i].clear();
        fZDataP[i].clear();
        fZDataA[i] = tempA[i];
        fZDataP[i] = tempP[i];
    }

    delete [] tempA;
    delete [] tempP;

}

void FileIO::FFTWFilters(int filterID)
{

}

double FileIO::ExtractYValue(QString str){
    int pos = str.lastIndexOf("_") ;

    int pos2 = str.lastIndexOf("FUNCTION");
    if( pos2 == -1){
        pos2 = str.lastIndexOf("mT");
    }
    if( pos2 == -1){
        pos2 = str.lastIndexOf("mV");
    }

    //qDebug() << str << ", " << pos << ";" << pos2;
    QString strY;
    if( pos2 == -1){
        strY = str.mid(pos+1, 4);
    }else{
        strY = str.mid(pos+1, pos2-pos-1);
    }
    //qDebug() << str << ", " << pos << ", " << strY;
    return strY.toDouble();
}

double FileIO::FindMax(QVector<double> vec)
{
    double max = vec[0];
    for( int i = 1; i < vec.size(); i++){
        if( vec[i] > max) max = vec[i];
    }
    return max;
}

double FileIO::FindMin(QVector<double> vec)
{
    double min = vec[0];
    for( int i = 1; i < vec.size(); i++){
        if( vec[i] < min) min = vec[i];
    }
    return min;
}

void FileIO::CalMeanVector()
{

    this->zMean.clear();

    double goal = -1; // us
    int xEnd = 0; // the index of x near goal usec
    for(int i = 0; i < xData.size() ; i++){
        if( xData[i] >= goal){
            xEnd = i;
            break;
        }
    }

    double mean = 0;
    zMeanMean = 0;

    for(int yIndex = 0; yIndex < ySize; yIndex++){
        for( int i = 0; i < xEnd; i ++){
            mean += zData[yIndex][i];
        }
        mean = mean / xEnd;
        this->zMean.push_back(mean);
        zMeanMean += mean / ySize;
    }  

    QString msg;
    msg.sprintf("total mean z for x < -1 us: %f", zMeanMean);
    SendMsg(msg);
}

void FileIO::RescaleData()
{
    this->multi = - qFloor(log(zMax)/log(10))+1;
    //qDebug("data was multiplied 10^%d", multi);
    for(int i = 0; i < xSize; i++){
        for(int j = 0; j < ySize; j++){
            zData[j][i] = zData[j][i] * pow(10,multi);
            backUpData[j][i] = backUpData[j][i] * pow(10,multi);
        }
    }
    zMax = zMax * pow(10, multi);
    zMin = zMin * pow(10, multi);

    xMin = FindMin(xData);
    xMax = FindMax(xData);
}
