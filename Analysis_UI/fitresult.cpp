#include "fitresult.h"
#include "ui_fitresult.h"

FitResult::FitResult(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FitResult),
    fitPar(NULL),
    fitParError(NULL),
    chisq(NULL),
    file(NULL)
{
    ui->setupUi(this);
    plot = ui->widget;
    this->setWindowFlags(Qt::Window);

    plotComb = 0; // see CheckPlotComb()

    parSize = 4; // initial value

    //set plot interaction
    plot->setInteraction(QCP::iRangeDrag,true);
    plot->setInteraction(QCP::iRangeZoom,true);
    plot->axisRect()->setRangeDrag(Qt::Vertical);
    plot->axisRect()->setRangeZoom(Qt::Vertical);

    //set xaxis label
    plot->xAxis->setLabel("Ctrl. Vol. [V]");
    plot->xAxis2->setVisible(true);
    plot->xAxis2->setLabel("y-Index");
    plot->yAxis->setRange(-120,120);
    plot->yAxis2->setVisible(true);
    plot->yAxis2->setTickLabels(false);
    plot->yAxis2->setTicks(false);
    plot->setAutoAddPlottableToLegend(true);

    //set 7 plots.
    for(int i = 0; i < 7 ; i++) {
        plot->addGraph();
        plot->graph(i)->setLineStyle(QCPGraph::lsNone);
        plot->graph(i)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle , 3));
        plot->graph(i)->setErrorType(QCPGraph::etValue);
    }

    //set plot color
    plot->graph(0)->setPen(QPen(Qt::blue));
    plot->graph(1)->setPen(QPen(Qt::cyan));
    plot->graph(2)->setPen(QPen(Qt::green));
    plot->graph(3)->setPen(QPen(Qt::magenta));
    plot->graph(4)->setPen(QPen(Qt::darkYellow ));
    plot->graph(5)->setPen(QPen(Qt::red ));
    plot->graph(6)->setPen(QPen(Qt::darkGreen ));

    plot->graph(0)->setErrorPen(QPen(Qt::blue));
    plot->graph(1)->setErrorPen(QPen(Qt::cyan));
    plot->graph(2)->setErrorPen(QPen(Qt::green));
    plot->graph(3)->setErrorPen(QPen(Qt::magenta));
    plot->graph(4)->setErrorPen(QPen(Qt::darkYellow));
    plot->graph(5)->setErrorPen(QPen(Qt::red));
    plot->graph(6)->setErrorPen(QPen(Qt::darkGreen));

    //Set Plot Name
    plot->graph(0)->setName("a");
    plot->graph(1)->setName("Ta");
    plot->graph(2)->setName("b");
    plot->graph(3)->setName("Tb");
    plot->graph(4)->setName("c");
    plot->graph(5)->setName("Pol.");
    plot->graph(6)->setName("SSR/ndf");

    //Set Legend
    //plot->legend->setVisible(true);

    fixedSize = 0;
    plotUnit = 0;
}

FitResult::~FitResult()
{
    delete ui;
    delete plot;
    if( fitPar != NULL) delete [] fitPar;
    if( fitParError != NULL) delete [] fitParError;
    if( chisq != NULL) delete chisq;
}

void FitResult::ClearData()
{
    fixedSize = 0;
    if( fitPar != NULL) delete [] fitPar;
    if( fitParError != NULL) delete [] fitParError;
    if( chisq != NULL) delete chisq;

    fitPar = NULL;
    fitParError = NULL;
    chisq = NULL;

    file = NULL;

}

void FitResult::SetDataSize(FileIO *file)
{
    if( fixedSize == 1) return;

    this->file = file;

    int n = file->GetDataSetSize();

    dataSize = n;
    fitPar = new QVector<double> [n];
    fitParError = new QVector<double> [n];
    chisq = new double [n];
    plot->xAxis->setRange(file->GetYMin_CV(), file->GetYMax_CV());
    plot->xAxis2->setRange(0,n-1);
    if( file->HasBackGround()) plot->xAxis2->setRange(1,n-1);

    if(file->IsYRevered()) {
        plot->xAxis2->setRangeReversed(1);
    }else{
        plot->xAxis2->setRangeReversed(0);
    }

    fixedSize = 1;

    QVector<double> temp(5);
    for(int i = 0; i < n; i++){
        fitPar[i] = temp;
        fitParError[i] = temp;
        chisq[i] = 0;
    }

    QString msg;
    msg.sprintf("Initaliate fitPar array. size : %d", n);
    SendMsg(msg);

    plot->disconnect();
    connect(plot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(ShowPlotValue(QMouseEvent*)));
}

void FitResult::SetAvalibleData(int n)
{
    this->parSize = n;
    QString msg;
    msg.sprintf("parSize: %d", parSize);
    SendMsg(msg);
    if( n == 3){
        ui->checkBox_b->setEnabled(0);
        ui->checkBox_Tb->setEnabled(0);
        ui->checkBox_c->setEnabled(1);

        ui->checkBox_b->setChecked(0);
        on_checkBox_b_clicked(0);
        ui->checkBox_Tb->setChecked(0);
        on_checkBox_Tb_clicked(0);

    }else if( n == 4){
        ui->checkBox_b->setEnabled(1);
        ui->checkBox_Tb->setEnabled(1);
        ui->checkBox_c->setEnabled(0);

        ui->checkBox_c->setChecked(0);
        on_checkBox_c_clicked(0);

    }else if( n == 5){
        ui->checkBox_b->setEnabled(1);
        ui->checkBox_Tb->setEnabled(1);
        ui->checkBox_c->setEnabled(1);
    }else{
        ui->checkBox_b->setEnabled(0);
        ui->checkBox_Tb->setEnabled(0);
        ui->checkBox_c->setEnabled(0);

        ui->checkBox_b->setChecked(0);
        ui->checkBox_Tb->setChecked(0);
        ui->checkBox_c->setChecked(0);

        on_checkBox_b_clicked(0);
        on_checkBox_Tb_clicked(0);
        on_checkBox_c_clicked(0);
    }
}

void FitResult::FillData(Analysis *ana)
{
    if( ana->GetFitFlag() != 0){
        SendMsg("fit not good.");
        return;
    }

    int yIndex = ana->GetYIndex();
    this->parSize = ana->GetParametersSize(); // safty
    fitPar[yIndex] = ReSizeVector(ana->GetParameters());
    fitParError[yIndex] = ReSizeVector(ana->GetParError());
    int nDF = ana->GetNDF();
    chisq[yIndex] = ana->GetSSR()/nDF/ana->GetSampleVariance();

}

void FitResult::PlotData()
{
    if( fixedSize == 0) return;

    //get what to plot
    on_checkBox_a_clicked(ui->checkBox_a->isChecked());
    on_checkBox_Ta_clicked(ui->checkBox_Ta->isChecked());
    on_checkBox_b_clicked(ui->checkBox_b->isChecked());
    on_checkBox_Tb_clicked(ui->checkBox_Tb->isChecked());
    on_checkBox_c_clicked(ui->checkBox_c->isChecked());
    on_checkBox_P_clicked(ui->checkBox_P->isChecked());
    on_checkBox_SSR_clicked(ui->checkBox_SSR->isChecked());

    double xMin, xMax;
    switch (plotUnit) {
    case 0:
        xMin = file->GetYMin_CV();
        xMax = file->GetYMax_CV();
        plot->xAxis->setLabel("Ctrl. Vol. [V]");
        break;
    case 1:
        xMin = file->GetYMin_HV();
        xMax = file->GetYMax_HV();
        plot->xAxis->setLabel("Hall Vol. [mV]");
        break;
    case 2:
        xMin = file->HV2Mag(file->GetYMin_HV());
        xMax = file->HV2Mag(file->GetYMax_HV());
        plot->xAxis->setLabel("B-field [mT]");
        break;
    }
    plot->xAxis->setRange(xMin, xMax);

    if( plotUnit != 0){
        plot->xAxis2->setTickLabels(false);
        plot->xAxis2->setTicks(false);
        plot->xAxis2->setLabel("");
    }else{
        plot->xAxis2->setTickLabels(true);
        plot->xAxis2->setTicks(true);
        plot->xAxis2->setLabel("y-Index");
    }

    plot->replot();

}

void FitResult::PlotSingleData(int plotID){
    // plotID, 1 = a, 2 = Ta, 3 = b, 4 = Tb, 5 = c

    QVector<double> x, y, ye;
    int iStart = 0;
    if( file->HasBackGround()) iStart = file->GetBGIndex()+1;
    for(int i = iStart; i < dataSize; i++){

        double xtemp = file->GetDataY_HV(i);
        switch (plotUnit) {
        case 1: x.push_back(xtemp);break;
        case 2: x.push_back(file->HV2Mag(xtemp));break;
        default: x.push_back(file->GetDataY_CV(i));break;
        }
        y.push_back(fitPar[i][plotID]);
        ye.push_back(fitParError[i][plotID]);
    }

    plot->graph(plotID)->setDataValueError(x,y, ye);
    //plot->graph(plotID)->rescaleAxes(true);
    plot->replot();

}

void FitResult::on_checkBox_a_clicked(bool checked)
{
    if( fixedSize == 0) return;
    if(checked){
        PlotSingleData(0);
    }else{
        plot->graph(0)->clearData();
    }
    plot->replot();
    CheckPlotComb();
}

void FitResult::on_checkBox_Ta_clicked(bool checked)
{
    if( fixedSize == 0) return;
    if(checked){
        PlotSingleData(1);
    }else{
        plot->graph(1)->clearData();
    }
    plot->replot();
    CheckPlotComb();
}

void FitResult::on_checkBox_b_clicked(bool checked)
{
    if( fixedSize == 0) return;
    if(checked){
        PlotSingleData(2);
    }else{
        plot->graph(2)->clearData();
    }
    plot->replot();
    CheckPlotComb();
}

void FitResult::on_checkBox_Tb_clicked(bool checked)
{
    if( fixedSize == 0) return;
    if(checked){
        PlotSingleData(3);
    }else{
        plot->graph(3)->clearData();
    }
    plot->replot();
    CheckPlotComb();
}

void FitResult::on_checkBox_c_clicked(bool checked)
{
    if( fixedSize == 0) return;
    if(checked){
        PlotSingleData(4);
    }else{
        plot->graph(4)->clearData();
    }
    plot->replot();
    CheckPlotComb();
}

void FitResult::on_checkBox_P_clicked(bool checked)
{
    if( fixedSize == 0 ) return;

    if(checked){
        QVector<double> x, y, ye;
        int iStart = 0;
        if( file->HasBackGround()) iStart = file->GetBGIndex()+1;
        for(int i = iStart; i < dataSize; i++){

            double xtemp = file->GetDataY_HV(i);

            switch (plotUnit) {
            case 1: x.push_back(xtemp);break;
            case 2: x.push_back(file->HV2Mag(xtemp));break;
            default: x.push_back(file->GetDataY_CV(i));break;
            }

            double a = fabs(fitPar[i][0]);
            double b = fabs(fitPar[i][2]);
            double ea = fitParError[i][0];
            double eb = fitParError[i][2];

            double P = (a-b)/(a+b)*100;
            double eP = 100/pow(a+b,2)*sqrt(pow(ea,2)+ pow(eb,2)) * sqrt(2*(pow(a,2)+pow(b,2)));
            y.push_back(P);
            ye.push_back(eP);

        }

        plot->graph(5)->setDataValueError(x, y, ye);
        //plot->xAxis->setRange(x[0], x[dataSize-1]);
        //plot->graph(5)->rescaleAxes(true);

    }else{
        plot->graph(5)->clearData();
    }
    plot->replot();
    CheckPlotComb();

}

void FitResult::on_checkBox_SSR_clicked(bool checked)
{
    if( fixedSize == 0 ) return;
    if(checked){
        QVector<double> x, y;
        int iStart = 0;
        if( file->HasBackGround()) iStart = file->GetBGIndex()+1;
        for(int i = iStart; i < dataSize; i++){
            double xtemp = file->GetDataY_HV(i);

            switch (plotUnit) {
            case 1: x.push_back(xtemp);break;
            case 2: x.push_back(file->HV2Mag(xtemp));break;
            default: x.push_back(file->GetDataY_CV(i));break;
            }
            y.push_back(chisq[i]);
        }

        plot->graph(6)->setData(x,y);
        //plot->xAxis->setRange(x[0], x[dataSize-1]);
        //plot->graph(6)->rescaleAxes(true);

    }else{
        plot->graph(6)->clearData();
    }
    plot->replot();
    CheckPlotComb();
}

void FitResult::on_pushButton_Save_clicked()
{
    if( fixedSize == 0 ) return;

    QString filename = filePath;
    filename.chop(4);
    filename += "_";
    filename += ui->lineEdit->text();
    filename += ".txt";
    QFile savefile(filename);

    savefile.open(QIODevice::WriteOnly);

    QString msg;
    msg.sprintf("Saving all to : %s", filename.toStdString().c_str());
    SendMsg(msg);

    QTextStream stream(&savefile);

    QString lineout, tmp;

    //1st line
    lineout.sprintf("%10s, ", "y-index");
    tmp.sprintf("%10s, %10s, %10s, %10s, %10s, ", "a", "Ta", "b", "Tb", "c");
    lineout += tmp;
    tmp.sprintf("%10s, %10s, %10s, %10s, %10s, ", "s(a)", "s(Ta)", "s(b)", "s(Tb)", "s(c)");
    lineout += tmp;
    tmp.sprintf("%10s \n", "SSR/ndf");
    lineout += tmp;

    stream << lineout;

    //data
    for(int i = 0 ; i < dataSize ; i++){
        lineout.sprintf("%10d, ", i);

        for(int j = 0; j < 5; j++){
            tmp.sprintf("%10.3f, ", fitPar[i][j]); lineout += tmp;
        }
        for(int j = 0; j < 5; j++){
            tmp.sprintf("%10.3f, ", fitParError[i][j]); lineout += tmp;
        }

        tmp.sprintf("%10.3f \n", chisq[i]); lineout += tmp;

        stream << lineout;
    }

    savefile.close();
}

void FitResult::on_pushButton_ResetScale_clicked()
{
    //CheckPlotComb();
    qDebug() << "plotComb : " << plotComb << "," << file->GetZMin() << "," << file->GetZMax();
    if( (plotComb & 2) == 2) plot->yAxis->setRange(-120,120);
    if( (plotComb & 1) == 1) plot->yAxis->setRange(-0.2,3.0);
    if( (plotComb & 64) == 64 || (plotComb & 16) == 16) {
        plot->yAxis->setRange(file->GetZMin() * 1.2 ,file->GetZMax() * 1.2);
    }
    plot->replot();
}

void FitResult::ShowPlotValue(QMouseEvent *mouse)
{
    QPoint pt = mouse->pos();
    double x = plot->xAxis->pixelToCoord(pt.rx());
    double y = plot->yAxis->pixelToCoord(pt.ry());

    int yIndex = -1;
    switch (plotUnit) {
    case 1:
        yIndex = file->GetYIndex_HV(x);
        break;
    case 2:
        yIndex = file->GetYIndex_HV(file->Mag2HV(x));
        break;
    default:
        yIndex = file->GetYIndex_CV(x);
        break;
    }

    QString msg, tmp;
    msg.sprintf("(x, y) = (%7.4f, %7.4f), y-index = %4d, ", x, y, yIndex);

    if(yIndex > -1){
        for( int i = 0; i < parSize-1; i ++ ){
            tmp.sprintf("%7.4f(%7.4f), ", fitPar[yIndex][i], fitParError[yIndex][i]);
            msg.append(tmp);
        }
        tmp.sprintf("%7.4f(%7.4f) ", fitPar[yIndex][parSize-1], fitParError[yIndex][parSize-1]);
        msg.append(tmp);
    }

    ui->lineEdit_Msg->setText(msg);
}

void FitResult::CheckPlotComb()
{
    plotComb = 0;
    if(ui->checkBox_a->isChecked()) plotComb += 64;
    if(ui->checkBox_Ta->isChecked()) plotComb += 32;
    if(ui->checkBox_b->isChecked()) plotComb += 16;
    if(ui->checkBox_Tb->isChecked()) plotComb += 8;
    if(ui->checkBox_c->isChecked()) plotComb += 4;
    if(ui->checkBox_P->isChecked()) plotComb += 2;
    if(ui->checkBox_SSR->isChecked()) plotComb += 1;

}

QVector<double> FitResult::ReSizeVector(QVector<double> vec){
    int vecSize = vec.size();
    QVector<double> out(5);

    if( vecSize != parSize ) return out;

    out.clear();

    if( vecSize == 2){
        out = vec;
        out.push_back(0);
        out.push_back(0);
        out.push_back(0);
    }

    if( vecSize == 3){
        out.push_back(vec[0]);
        out.push_back(vec[1]);
        out.push_back(0);
        out.push_back(0);
        out.push_back(vec[2]);
    }

    if( vecSize == 4){
        out.push_back(vec[0]);
        out.push_back(vec[1]);
        out.push_back(vec[2]);
        out.push_back(vec[3]);
        out.push_back(0);
    }

    if( vecSize == 5){
        out = vec;
    }

    return out;
}

void FitResult::on_pushButton_SavePlot_clicked()
{
    QFileDialog fileDialog(this);
    fileDialog.setNameFilter("pdf (*pdf)");
    fileDialog.setDirectory(DESKTOP_PATH);
    fileDialog.setReadOnly(0);
    QString fileName;
    if( fileDialog.exec()){
        fileName = fileDialog.selectedFiles()[0];
    }

    if( fileName.right(4) != ".pdf" ) fileName.append(".pdf");

    int ph = plot->geometry().height();
    int pw = plot->geometry().width();

    bool ok = plot->savePdf(fileName, pw, ph );

    if( ok ){
        SendMsg("Saved Fit-Result Plot as " + fileName);
    }else{
        SendMsg("Save Failed.");
    }
}
