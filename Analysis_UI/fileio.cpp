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
    hallParFile = NULL;

    zData = NULL;
    backUpData = NULL;
    fZDataA = NULL;
    fZDataP = NULL;

    xData.clear();
    yData_CV.clear();
    zMean.clear();

    fxData.clear();
    fyData.clear();

    hallPar.clear();

    xMax = 0;
    xMin = 0;
    yMax_CV = 0;
    yMin_CV = 0;
    zMax = 0;
    zMin = 0;
    zMeanMean = 0;

    colwise = 0;
    openState = 0;
    isOutFileOpened = 0;
    yRevered = 0;
    hadBG = 0;

    multi = 0;

    xSize = 0;
    ySize = 0;
}

FileIO::~FileIO(){
    SendMsg("release mem.");
    delete myfile;
    if( outfile != NULL ) delete outfile;
    if( hallParFile != NULL ) delete hallParFile;
    if( zData != NULL ) delete [] zData;
    if( backUpData != NULL ) delete [] backUpData;
    if( fZDataA != NULL ) delete [] fZDataA;
    if( fZDataP != NULL ) delete [] fZDataP;
}

void FileIO::OpenSaveFileforFit(){
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


void FileIO::OpenHV2MagParametersFile()
{
    QString parFilePath = HALL_PATH;

    hallParFile = new QFile(parFilePath);
    hallParFile->open(QIODevice::ReadOnly);

    if( hallParFile->isOpen() ){
        SendMsg("Openned Hall parameter file.");
        SendMsg(HALL_PATH);

        QTextStream stream(hallParFile);
        QString line;

        while(stream.readLineInto(&line)){
            hallPar.push_back(line.toDouble());
        }

        hallParFile->close();
    }else{

        SendMsg("For Hall <-> B-field convertion, please add");
        SendMsg(parFilePath);
        SendMsg("and parameters in single coloumn.");
        SendMsg("==== Use no convertion 1 mV = 1 mT."); //TODO tell mainWindows no magnetic field.

        hallPar.push_back(0.);
        hallPar.push_back(1.);

    }

    if( hallPar.size() <= 1){
        SendMsg("The Hall Parameters Files is not valid. Use Default Parameters.");
        hallPar.clear();
        hallPar.push_back(0.);
        hallPar.push_back(1.);
    }

    QString msg = "Hall Parameters : ";
    QString tmp;
    int n = hallPar.size();
    for( int i = 0; i < n-1; i++){
        tmp.sprintf("%8e, ", hallPar[i]);
        msg.append(tmp);
    }
    tmp.sprintf("%8e #", hallPar[n-1]);
    msg.append(tmp);
    SendMsg(msg);

}

void FileIO::OpenDoubleXCSVData(){

    //TODO add myfile->isOpen check

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
    yData_CV.clear();
    xData.clear();
    myfile->seek(0);
    int rows = 0;
    while(stream.readLineInto(&line) && rows <= xSize+1){
        rows ++;
        QStringList lineList = line.split(",");
        if( rows == 1){ // get yDatax
            for( int i = 1 ; i < lineList.size() ; i++ ){
                if( i % 2 == 1) {
                    double temp = ExtractYValue(lineList[i],-1);
                    yString.push_back(lineList[i]);
                    yData_CV.push_back(temp) ; // get data from string.
                    yData_HV.push_back(temp); // for CSV data, HV = CV
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
    qDebug("Y: %d , %d", yData_CV.size(), ySize);

    xMin = FindMin(xData);
    xMax = FindMax(xData);
    xStep = xData[1]-xData[0];

    //============== check is BG data exist by checking Hall probe volatge,
    QVector<double> newYData_CV = yData_CV;
    QVector<double> newYData_HV = yData_HV;

    //if Control volatge == 0 V, it is bg
    bgIndex = -1;
    for( int i = 0; i < ySize; i++){
        if( yData_CV[i] == 0 ) {
            bgIndex = i;
        }else{
            break;
        }
    }

    if( bgIndex != -1 ) {
        hadBG = 1;
        newYData_CV.remove(0);
        newYData_HV.remove(0);
    }

    yMin_CV = FindMin(newYData_CV);
    yMax_CV = FindMax(newYData_CV);
    yMin_HV = FindMin(newYData_HV);
    yMax_HV = FindMax(newYData_HV);

    yStep_CV = fabs(yData_CV[ySize-1]- yData_CV[ySize-2]);
    yStep_HV = yStep_CV;

    //=========== check is data revered.
    if(newYData_HV[0] > newYData_HV[1]) yRevered = 1;

    //=========== the open file complete, show message
    openState = 1;

    QString msg;
    msg.sprintf("X:(%7.3f, %7.3f) sizeX:%d",xMin, xMax, xSize);
    SendMsg(msg);
    msg.sprintf("Y:(%7.3f, %7.3f) sizeY:%d",yMin_CV, yMax_CV, ySize);
    SendMsg(msg);
    if( hadBG ){
        msg.sprintf("    BG data exist. At yIndex = %d", bgIndex);
        SendMsg(msg);
    }
    msg.sprintf("Z:(%7.3f, %7.3f)",zMin, zMax);
    SendMsg(msg);
    myfile->close();

    //=========== calculate mean vector for mean correction, and estimated c-parameter
    CalMeanVector();

}

void FileIO::OpenTxtData_col()
{
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
    fZDataA = new QVector<double> [ySize];
    fZDataP = new QVector<double> [ySize];
    double ** z ;
    z = new double *[ySize];
    for(int i = 0; i < ySize; i++){
        z[i] = new double [xSize];
    }

    //get data
    multi = 3;
    yData_CV.clear();
    xData.clear();
    myfile->seek(0);
    int rows = 0;
    while(stream.readLineInto(&line) && rows <= xSize + 1){
        rows ++;
        QStringList lineList = line.split(",");
        if( rows == 1){ // get yDatax
            for( int i = 1 ; i < lineList.size() ; i++ ){
                yString.push_back(lineList[i]);
                double temp = ExtractYValue(lineList[i]);
                yData_CV.push_back(temp);
                temp = ExtractYValue(lineList[i],1);
                yData_HV.push_back(temp);
            }
        }else{
            xData.push_back((lineList[0]).toDouble() * 1e6) ;
            int yIndex = 0;
            for( int i = 1 ; i < lineList.size() ; i++ ){
                z[yIndex][rows-2] = (lineList[i]).toDouble() * pow(10,3);
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
    qDebug("Y: %d , %d", yData_CV.size(), ySize);

    xMin = FindMin(xData);
    xMax = FindMax(xData);
    xStep = xData[1]-xData[0];

    //=============== check is BG data exist by checking Hall voltage
    QVector<double> newYData_CV = yData_CV;
    QVector<double> newYData_HV = yData_HV;

    //if control voltage == 0 V it is bg
    bgIndex = -1;
    for( int i = 0; i < ySize; i++){
        if( yData_CV[i] == 0){
            bgIndex = i;
        }else{
            break;
        }
    }

    if( bgIndex != -1){
        hadBG = 1;
        newYData_CV.remove(0);
        newYData_HV.remove(0);
    }

    yMax_CV = FindMax(newYData_CV);
    yMin_CV = FindMin(newYData_CV);
    yMax_HV = FindMax(newYData_HV);
    yMin_HV = FindMin(newYData_HV);

    yStep_CV = fabs(yData_CV[ySize-1]- yData_CV[ySize-2]);
    yStep_HV = fabs(yData_HV[ySize-1]- yData_HV[ySize-2]);

    if(newYData_HV[0] > newYData_HV[1]) yRevered = 1;

    openState = 1;

    QString msg;
    msg.sprintf("X:(%7.3f, %7.3f) sizeX:%d",xMin, xMax, xSize);
    SendMsg(msg);

    msg.sprintf("Y:(%7.3f, %7.3f) sizeY:%d",yMin_CV, yMax_CV, ySize);
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
        if( line.left(4) == "####" ) break;

        ySize ++;
        if( ySize == 0){ // get yDatax
            QStringList lineList = line.split(",");
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
    while(stream.readLineInto(&line) && rows <= ySize ){
        rows ++;
        QStringList lineList = line.split(",");

        if( rows == 1){ // get xDatax
            for( int i = 1 ; i < lineList.size() ; i++ ){
                xData.push_back((lineList[i]).toDouble()) ;
            }
        }else{
            double temp = ExtractYValue(lineList[0],0);
            yString.push_back(lineList[0]);
            yData_CV.push_back(temp);
            temp = ExtractYValue(lineList[0],1);
            yData_HV.push_back(temp);
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

    xMin = FindMin(xData);
    xMax = FindMax(xData);
    xStep = xData[1]-xData[0];

    //=========== cal the rescale factor
    RescaleZData();

    //============== check is BG data exist by checking Hall probe volatge,
    QVector<double> newYData_CV = yData_CV;
    QVector<double> newYData_HV = yData_HV;

    //if Control volatge == 0 V, it is bg
    bgIndex = -1;
    for( int i = 0; i < ySize; i++){
        if( yData_CV[i] == 0 ) {
            bgIndex = i;
        }else{
            break;
        }
    }

    if( bgIndex != -1 ) {
        hadBG = 1;
        newYData_CV.remove(0);
        newYData_HV.remove(0);
    }

    yMin_CV = FindMin(newYData_CV);
    yMax_CV = FindMax(newYData_CV);
    yMin_HV = FindMin(newYData_HV);
    yMax_HV = FindMax(newYData_HV);

    yStep_CV = fabs(yData_CV[ySize-1]- yData_CV[ySize-2]);
    yStep_HV = fabs(yData_HV[ySize-1]- yData_HV[ySize-2]);

    //=========== check is data revered.
    if(newYData_HV[0] > newYData_HV[1]) yRevered = 1;

    //=========== the open file complete, show message
    openState = 1;

    QString msg;
    msg.sprintf("X   :(%7.3f, %7.3f) sizeX:%d",xMin, xMax, xSize);
    SendMsg(msg);
    msg.sprintf("Y_CV:(%7.3f, %7.3f) sizeY:%d",yMin_CV, yMax_CV, ySize);
    SendMsg(msg);
    msg.sprintf("Y_HV:(%7.3f, %7.3f) sizeY:%d",yMin_HV, yMax_HV, yData_HV.size());
    SendMsg(msg);
    if( hadBG ){
        msg.sprintf("    BG data exist. At yIndex = %d", bgIndex);
        SendMsg(msg);
    }
    msg.sprintf("Z   :(%7.3f, %7.3f), multiplied : 10^%d",zMin, zMax, multi);
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

void FileIO::SaveCSV(bool doubleX, bool origin)
{
    // the simplified txt can be used for Analysis::Gnufit
    this->csvFilePath = this->filePath;
    csvFilePath.chop(4);
    if( origin ){
        csvFilePath.append("_origin");
    }
    if( doubleX ){
        csvFilePath.append("_col_doubleX.csv");
    }else{
        csvFilePath.append("_col_singleX.csv");
    }
    //this->simFilePath = OPENPATH;
    //this->simFilePath.append("test.dat");

    QFile out(csvFilePath);
    out.open( QIODevice::WriteOnly );
    QTextStream stream(&out);
    QString str, tmp;

    int yStart = 0;
    //if( hadBG ) yStart = bgIndex + 1;

    //output y-String
    tmp.sprintf("%*s,", 8, "time [s]"); str = tmp;
    for(int i = yStart; i < ySize-1; i++){
        //tmp.sprintf("%10.4f,", yData[i]); str += tmp;
        QString ytext = yString[i];
        tmp.sprintf("%s", ytext.toStdString().c_str()); str += tmp;
        tmp.sprintf("_%7.3fmT,", HV2Mag(yData_HV[i])); str += tmp;
        if( doubleX ) str += " X,";
    }
    tmp.sprintf("%s\n", yString[ySize-1].toStdString().c_str()); str += tmp;
    stream << str;

    //output x and z
    for(int i = 0; i < xSize; i++){
        tmp.sprintf("%8e,", xData[i]*1e-6); str = tmp;
        for(int j = yStart; j < ySize-1; j++){
            if( origin){
                tmp.sprintf("%11.4e,", backUpData[j][i]  * pow(10, -multi)); str += tmp;
            }else{
                tmp.sprintf("%11.4e,", zData[j][i]  * pow(10, -multi)); str += tmp;
            }

            if(doubleX ) {
                tmp.sprintf("%11.4e,", xData[i]*1e-6); str += tmp;
            }
        }
        if( origin ){
            tmp.sprintf("%11.4e\n", backUpData[ySize-1][i] * pow(10, -multi)); str += tmp;
        }else{
            tmp.sprintf("%11.4e\n", zData[ySize-1][i] * pow(10, -multi)); str += tmp;
        }
        stream << str;
    }

    out.close();

    QString msg;
    msg = "Saved ";
    if( origin ) {
        msg.append("original date as ");
    }else{
        msg.append("modifed data as ");
    }
    if( doubleX ){
        msg.append(" a double-X ");
    }else{
        msg.append(" a single-X ");
    }
    msg.append("CSV file as :");
    SendMsg(msg);
    SendMsg(csvFilePath);

}

void FileIO::SaveTxtData_row()
{
    QString fileName = this->filePath;
    fileName.chop(4);
    fileName.append("_modified.dat");

    QFile out(fileName);
    out.open(QIODevice::WriteOnly);
    QTextStream stream(&out);
    QString str, tmp;

    int nameLenght = 40;

    // Set x
    tmp.sprintf("%*s,", nameLenght, "time [us]"); str = tmp;
    for(int i = 0; i < xSize - 1 ; i++){
        tmp.sprintf("%11.4e,", xData[i]); str += tmp;
    }
    tmp.sprintf("10e\n", xData[xSize-1]); str += tmp;
    stream << str;

    // Set data
    int yStart = 0;
    if( hadBG ) yStart = bgIndex + 1;
    for( int y = yStart; y < ySize; y++){
        tmp.sprintf("%*s,", nameLenght, yString[y].toStdString().c_str()); str = tmp;
        double bg = 0;
        for( int i = 0; i < xSize - 1 ; i++){
            if( hadBG ) bg = zData[y][i];
            tmp.sprintf("%11.4e,", (zData[y][i] - bg) * pow(10, -multi)); str += tmp;
        }
        if( hadBG ) bg = zData[y][xSize - 1];
        tmp.sprintf("%11.4e\n", (zData[y][xSize -1] - bg) * pow(10, -multi)); str += tmp;
        stream << str;
    }

    out.close();

    QString msg;
    msg.sprintf("Saved modified data as :");
    SendMsg(msg);
    SendMsg(fileName);
}

void FileIO::ManipulateData(int id, int bgIndex, int n)
{
    // id is bitwise
    //    0 = restore data
    //    1 = bg substraction
    //   10 = mean correction
    //  110 = moving average

    QString msg;

    // restore data
    //SendMsg("restoring Data.");
    for( int y = 0; y < ySize; y++){
        zData[y].clear();
        for(int x = 0; x < xSize; x++){
            zData[y].push_back(backUpData[y][x]);
        }
    }
    CalMeanVector();

    if( (id & 1) == 1 && bgIndex > -1){
        QVector<double> ref = zData[bgIndex];
        //msg.sprintf("Using the %d(th)-data as background.", bgIndex);
        //SendMsg(msg);
        for( int y = 0; y < ySize; y++){
            zData[y].clear();
            for(int x = 0; x < xSize; x++){
                zData[y].push_back(backUpData[y][x] - ref[x]);
            }
        }
        CalMeanVector();
    }

    if( (id & 2) == 2 ){
        //SendMsg("Mean correction.");
        for( int y = 0; y < ySize; y++){
            for(int x = 0; x < xSize; x++){
                zData[y][x] = zData[y][x] - zMean[y];
            }
        }
        CalMeanVector();
    }

    // moving average
    if( (id & 4) == 4 && n > -1  && (n % 2) == 1 ) {
        //msg.sprintf("Moving Average n = %d.", n);
        //SendMsg(msg);
        int m = (n-1)/2;
        for( int y = 0; y < ySize; y++){
            QVector<double> temp = zData[y];
            for(int x = 0; x < xSize; x++){
                double mean = 0;
                for( int k = x-m; k <= x+m; k++ ){
                    if( k < 0 || k >= xSize) continue;
                    mean += temp[k]/n;
                }
                zData[y][x] = mean;
            }
        }
        CalMeanVector();
    }

}

int FileIO::GetXIndex(double x)
{
    return FindIndex(xData, xStep, x, 0);
}

int FileIO::GetYIndex_CV(double y)
{
    int yIndex = FindIndex(yData_CV, yStep_CV, y, yRevered);
    return yIndex;
}

int FileIO::GetYIndex_HV(double y)
{
    int yIndex = FindIndex(yData_HV, yStep_HV, y, yRevered);
    return yIndex;
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

    //======================================
    SendMsg("Cal. freqeuncy.");
    this->fFreqResol = 1000./(xMax - xMin); // MHz
    int d = 0;
    if( xSize % 2 == 0){
        d = xSize /2;
    }else{
        d = (xSize+1)/2;
    }

    fxData.clear();
    for( int i = 0; i < xSize; i++){
        fxData.push_back( (-d + i) * this->fFreqResol );
    }

    fxMin = -d * this->fFreqResol;
    fxMax = (xSize-1-d) * this->fFreqResol;

    if( ySize % 2 == 0){
        d = ySize /2;
    }else{
        d = (ySize+1)/2;
    }
    fyData.clear();
    for( int i = 0; i < ySize; i++){
        fyData.push_back( (-d + i) );
    }

    fyMin = -d;
    fyMax = (ySize-1-d);

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
            in[i*xSize + j][0] = fZDataA[i][j];
            in[i*xSize + j][1] = fZDataP[i][j];
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

    SwapFFTData(1);
}

QVector<double> FileIO::Shift(QVector<double> list, int d)
{
    int n = list.length();           // list = {0,1,2,3,4,5,6,7,8,9}. n = 10
    QVector<double> temp = list.mid(n-d); //say, d = 5, store temp = {5,6,7,8,9}
    for( int i = n-1; i > d-1; i-- ){ // from i = 9, list[9] = list[9-5 = 4]
        list[i] = list[i-d];          //             list[8] = list[8-5 = 3]
    }                                 //             list[5] = list[0]
    for( int i = 0; i < temp.length(); i++){
        list[i] =  temp[i];           // replace list[i = 0 to 4] with temp.
    }
    return list;
}

void FileIO::SwapFFTData(bool dir)
{
    // dir = 1 , forward swap; dir = 0, backward

    if( dir == 1){
        SendMsg("Swapping data.");
    }else{
        SendMsg("UnSwapping data.");
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

void FileIO::FFTWFilters(int filterID, QVector<double> par, QVector<double>funcX, QVector<double> funcY, bool rev)
{
    // 1 = low pass sharpe cut
    // 2 = low pass
    // 3 = Guassian
    // 4 = retangle

    QString msg;

    if( filterID == 1){
        int x2 = FindIndex(fxData, fFreqResol, par[0], 0);
        if( !rev ){
            msg.sprintf("FFTW: Low-Pass Sharp Filter. |freq| <= %f kHz", fxData[x2]);
        }else{
            msg.sprintf("FFTW: High-Pass Sharp Filter. |freq| >= %f kHz", fxData[x2]);
        }
        SendMsg(msg);
    }

    if( filterID == 2){
        if( !rev ){
            msg.sprintf("FFTW: Low-Pass Filter. freq =  %f kHz", par[0]);
        }else{
            msg.sprintf("FFTW: High-Pass Filter. freq =  %f kHz", par[0]);
        }
        SendMsg(msg);
    }

    if( filterID == 3){
        int xH2 = FindIndex(fxData, fFreqResol, par[0], 0);
        int xL2 = FindIndex(fxData, fFreqResol, par[1], 0);

        if( !rev ){
            msg.sprintf("FFTW: Band-Pass Sharp Filter. %f kHz <= |freq| <= %f kHz", fxData[xL2], fxData[xH2]);
        }else{
            msg.sprintf("FFTW: Band-Block Sharp Filter. %f kHz <= |freq| <= %f kHz", fxData[xL2], fxData[xH2]);
        }

        SendMsg(msg);

    }

    if( filterID == 4){
        if( !rev ){
            msg.sprintf("FFTW: Gaussian-Pass Filter. peak = %f kHz, width = %f kHz", par[0], par[1]);
        }else{
            msg.sprintf("FFTW: Gaussian-Block Filter. peak = %f kHz, width = %f kHz", par[0], par[1]);
        }
        SendMsg(msg);
    }

    //Apply filter
    for(int i = 0 ; i < ySize ; i++){
        for(int j = 0 ; j < xSize ; j++){
            fZDataA[i][j] = fZDataA[i][j] * funcX[j] * funcY[i];
            fZDataP[i][j] = fZDataP[i][j] * funcX[j] * funcY[i];
        }
    }

}

void FileIO::RemoveYConstant()
{
    int d = ySize / 2;
    if( ySize % 2 == 1){
        d = (ySize-1)/2;
    }

    for(int i = 0; i < xSize ; i++){
        fZDataA[d][i] = 0;
        fZDataP[d][i] = 0;
    }

}

void FileIO::MovingAvgonFFTW(int n)
{
    if( n % 2 == 0 ) return;


}

int FileIO::ChangeZData(int yIndex, QVector<double> newZ)
{
    //qDebug() << newZ.size() << ", " << zData[yIndex].size();
    if( newZ.size() != zData[yIndex].size()) return 0;

    //qDebug() << "rewrite";

    for( int i = 0; i < zData[yIndex].size() ; i++){
        this->zData[yIndex][i] = newZ[i];
    }

    return 1;
}

double FileIO::ExtractYValue(QString str, int index){
    // when index == 0; find  V, the Control voltage
    // when index == 1; find mV, the Hall voltage

    QStringList strList = str.split("_");
    QString temp;
    for( int i = 0; i < strList.size(); i++){
        if( index == 0 && strList[i].right(2) != "mV" && strList[i].right(1) == "V" ) {
            temp = strList[i];
            temp.chop(1);
        }
        if( index == 1 && strList[i].right(2) == "mV"){
            temp = strList[i];
            temp.chop(2);
        }
    }

    //qDebug()<< strList << ", "  << temp << ", " << temp.toDouble();

    return temp.toDouble();
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

    //QString msg;
    //msg.sprintf("total mean z for x < -1 us: %f", zMeanMean);
    //SendMsg(msg);
}

void FileIO::RescaleZData()
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
}

int FileIO::FindIndex(QVector<double> vec, double torr, double goal, bool dir)
{
    int index = -1;

    if( openState == 0) return index;

    if( dir == 0 ){
        for(int i = 0; i < vec.size() ; i++){
            //if( vec[i] >= goal){
            if( fabs(vec[i] - goal) < torr){
                index = i;
                break;
            }
        }
    }else{
        for(int i = vec.size() - 1; i > -1 ; i--){
            //if( vec[i] <= goal){
            if( fabs(vec[i] - goal) < torr){
                index = i;
                break;
            }
        }
    }
    return index;
}
