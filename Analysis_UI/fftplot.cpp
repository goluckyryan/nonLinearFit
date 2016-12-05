#include "fftplot.h"
#include "ui_fftplot.h"

FFTPlot::FFTPlot(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FFTPlot)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window);

    filterFuncX.clear();
    filterFuncY.clear();

    plot_X = ui->widget_X;
    plot_X->addGraph(); // id = 0 amp or phase
    plot_X->addGraph(); // id = 1 filter
    plot_X->graph(0)->setPen(QPen(Qt::blue));
    plot_X->graph(1)->setPen(QPen(Qt::red));

    plot_X->xAxis->setLabel("freq [kHz]");
    //plot_X->xAxis2->setVisible(true);
    plot_X->xAxis2->setLabel("index");
    plot_X->yAxis->setLabel("Sum of Amp [a.u.]");
    plot_X->setInteraction(QCP::iRangeDrag,true);
    plot_X->setInteraction(QCP::iRangeZoom,true);
    plot_X->axisRect()->setRangeDrag(Qt::Horizontal);
    plot_X->axisRect()->setRangeZoom(Qt::Horizontal);
    //plot_X->yAxis->setScaleType(QCPAxis::stLogarithmic);

    plot_A = ui->widget_A;
    plot_A->axisRect()->setupFullAxesBox(true);
    plot_A->xAxis->setLabel("freq [kHz]");
    plot_A->yAxis->setLabel("index [a.u.]");

    colorMap_A = new QCPColorMap(plot_A->xAxis, plot_A->yAxis);
    colorMap_A->clearData();

    plot_P = ui->widget_P;
    plot_P->axisRect()->setupFullAxesBox(true);
    plot_P->xAxis->setLabel("freq [kHz]");
    plot_P->yAxis->setLabel("index [a.u.]");

    colorMap_P = new QCPColorMap(plot_P->xAxis, plot_P->yAxis);
    colorMap_P->clearData();

    QCPColorScale *colorScale_A = new QCPColorScale(plot_A);
    QCPColorScale *colorScale_P = new QCPColorScale(plot_P);
    plot_A->plotLayout()->addElement(0, 1, colorScale_A); // add it to the right of the main axis rect
    plot_P->plotLayout()->addElement(0, 1, colorScale_P); // add it to the right of the main axis rect
    colorScale_A->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorScale_P->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap_A->setColorScale(colorScale_A); // associate the color map with the color scale
    colorMap_P->setColorScale(colorScale_P); // associate the color map with the color scale

    colorMap_A->setGradient(QCPColorGradient::gpJet ); //color scheme
    colorMap_P->setGradient(QCPColorGradient::gpJet ); //color scheme

    //colorMap_A->setDataScaleType(QCPAxis::stLogarithmic);
    //colorMap_P->setDataScaleType(QCPAxis::stLogarithmic);

    plot_A->setInteraction(QCP::iRangeDrag,true);
    plot_A->setInteraction(QCP::iRangeZoom,true);
    plot_A->axisRect()->setRangeDrag(Qt::Horizontal);
    plot_A->axisRect()->setRangeZoom(Qt::Horizontal);

    plot_P->setInteraction(QCP::iRangeDrag,true);
    plot_P->setInteraction(QCP::iRangeZoom,true);
    plot_P->axisRect()->setRangeDrag(Qt::Horizontal);
    plot_P->axisRect()->setRangeZoom(Qt::Horizontal);

    plot_A->addGraph();
    plot_A->addGraph();
    plot_A->graph(0)->setPen(QPen(Qt::black));
    plot_A->graph(1)->setPen(QPen(Qt::black));
    plot_P->addGraph();
    plot_P->addGraph();
    plot_P->graph(0)->setPen(QPen(Qt::black));
    plot_P->graph(1)->setPen(QPen(Qt::black));


    filterID = 0;
    filterApplied = 0;
    filterChanged = 0;

    enableSliderFunction = 0;

}

FFTPlot::~FFTPlot()
{
    delete plot_X;
    delete colorMap_A;
    delete plot_A;
    delete colorMap_P;
    delete plot_P;

    delete ui;
}

void FFTPlot::SetData(FileIO *file)
{
    this->file = file;

    EnablePlanels(0);

    filterFuncX.clear();
    filterFuncY.clear();

    colorMap_A->clearData();
    colorMap_P->clearData();
    plot_A->replot();
    plot_P->replot();

    plot_X->graph(0)->clearData();
    plot_X->graph(1)->clearData();
    plot_X->replot();

    ui->lineEdit_freqResol->setText("");
    ui->lineEdit_XfreqH->setText("");

    needToCalFilterFuncX = 1;
    needToCalFilterFuncY = 1;

    enableSliderFunction = 0;

    EnablePlanels(0);
}

void FFTPlot::EnablePlanels(bool IO)
{
    ui->checkBox_RemoveConstant->setEnabled(IO);
    ui->checkBox_RemoveConstant->setChecked(0);
    ui->checkBox_MovingAvg->setEnabled(0);
    ui->spinBox_MovingAvg->setEnabled(0);

    ui->checkBox_Reverse->setEnabled(IO);
    ui->checkBox_EnableY->setEnabled(IO);

    ui->lineEdit_XfreqL->setEnabled(IO);
    ui->lineEdit_XfreqH->setEnabled(IO);
    ui->horizontalSlider_freqH->setEnabled(IO);
    ui->horizontalSlider_freqL->setEnabled(IO);

    ui->radioButton_LowPass->setEnabled(IO);
    ui->radioButton_LowPassSharp->setEnabled(IO);
    ui->radioButton_BandPassSharp->setEnabled(IO);
    ui->radioButton_Guassian->setEnabled(IO);

    ui->pushButton_ApplyFilter->setEnabled(IO);
    ui->pushButton_FFTWBackward->setEnabled(IO);

    ui->pushButton_ResetPlot->setEnabled(IO);
    ui->checkBox_toAP->setEnabled(IO);
    ui->checkBox_LogY->setEnabled(IO);
}

void FFTPlot::SetPlots()
{    
    enableSliderFunction = 0;

    xMin = file->GetfXMin();
    xMax = file->GetfXMax();
    yMin = file->GetfYMin();
    yMax = file->GetfYMax();

    nx = file->GetDataSize();
    ny = file->GetDataSetSize();

    //qDebug("x:(%f, %f) %d", xMin, xMax, nx);
    //qDebug("y:(%f, %f) %d", yMin, yMax, ny);

    plot_A->xAxis->setRange(xMin, xMax);
    plot_P->xAxis->setRange(xMin, xMax);

    plot_A->yAxis->setRange(yMin, yMax);
    plot_P->yAxis->setRange(yMin, yMax);

    plot_A->rescaleAxes();
    plot_P->rescaleAxes();

    //==========
    colorMap_A->data()->setRange(QCPRange(xMin, xMax), QCPRange(yMin, yMax));
    colorMap_P->data()->setRange(QCPRange(xMin, xMax), QCPRange(yMin, yMax));

    colorMap_A->data()->setSize(nx,ny);
    colorMap_P->data()->setSize(nx,ny);

    //==========
    plot_X->xAxis->setRange(xMin, xMax);
    plot_X->xAxis2->setRange(0,nx-1);
    plot_X->yAxis->setRangeLower(0);
    plot_X->rescaleAxes();

    //==========
    ui->horizontalSlider_freqH->setMinimum(0);
    ui->horizontalSlider_freqH->setMaximum(xMax);
    ui->horizontalSlider_freqH->setValue(xMax);

    ui->horizontalSlider_freqL->setMinimum(0);
    ui->horizontalSlider_freqL->setMaximum(xMax);
    ui->horizontalSlider_freqL->setValue(0);

    ui->verticalSlider_freqH->setMinimum(0);
    ui->verticalSlider_freqH->setMaximum(yMax);
    ui->verticalSlider_freqH->setValue(yMax);

    ui->verticalSlider_freqL->setMinimum(0);
    ui->verticalSlider_freqL->setMaximum(yMax);
    ui->verticalSlider_freqL->setValue(0);

    enableSliderFunction = 1;
}

void FFTPlot::ContourPlot()
{
    if( needToCalFilterFuncX ) CalFilterFunctionX();
    if( needToCalFilterFuncY ) CalFilterFunctionY();

    //===================== Apply to contour;

    QVector<double> *dataA = file->GetFFTDataA();
    QVector<double> *dataP = file->GetFFTDataP();

    QVector<double> amp;

    for( int i = 0; i < nx; i++){
        double temp = 0;
        for( int j = 0; j < ny; j++){

            double za = dataA[j][i] * filterFuncX[i] * filterFuncY[j];
            double zp = dataP[j][i] * filterFuncX[i] * filterFuncY[j];

            double ta = za;
            double tp = zp;

            if( ui->checkBox_toAP->isChecked()){
                ta = sqrt(za*za + zp*zp);
                tp = atan2(zp, za) * 180 / M_PI;
            }
            colorMap_A->data()->setCell(i,j, ta);
            colorMap_P->data()->setCell(i,j, tp);

            temp += sqrt( pow(za,2) + pow(zp,2) );

        }

        amp.push_back(temp);
    }

    dataA = NULL;
    dataP = NULL;

    plot_A->replot();
    plot_P->replot();

    //=============================
    QVector<double> x = file->GetfXDataSet();

    plot_X->graph(0)->clearData();
    plot_X->graph(0)->addData(x, amp);

    plot_X->graph(1)->clearData();
    plot_X->graph(1)->addData(x, filterFuncX);

    plot_X->replot();
}

void FFTPlot::RescalePlots()
{
    colorMap_A->setDataScaleType(QCPAxis::stLogarithmic);
    colorMap_A->setDataRange(QCPRange(1e-3,1));
    colorMap_A->rescaleDataRange();
    if( ui->checkBox_toAP->isChecked()){
        colorMap_P->setDataScaleType(QCPAxis::stLinear);
        colorMap_P->setDataRange(QCPRange(-180,180));
    }else{
        colorMap_P->setDataScaleType(QCPAxis::stLogarithmic);
        colorMap_A->setDataRange(QCPRange(1e-3,1));
    }
    colorMap_P->rescaleDataRange();

    plot_A->replot();
    plot_P->replot();

    plot_X->yAxis->rescale();
    if(ui->checkBox_LogY->isChecked()){
        plot_X->yAxis->setRangeLower(1e-2);
    }else{
        plot_X->yAxis->setRangeLower(0);
    }
    plot_X->replot();
}

void FFTPlot::CalFilterFunctionX()
{
    filterFuncX.clear();

    //================= x filter
    double freqH = ui->horizontalSlider_freqH->value();
    double freqL = 0;
    if( filterID == 3 || filterID == 4){
        freqL = ui->horizontalSlider_freqL->value();
    }

    for( int i = 0; i < nx; i++){
        double fx = file->GetfXData(i);

        if( filterID == 1 || filterID == 3){
            if( fabs(fx) <= freqH && fabs(fx) >= freqL){
                if( ui->checkBox_Reverse->isChecked()){
                    filterFuncX.push_back(0);
                }else{
                    filterFuncX.push_back(1);
                }
            }else{
                if( ui->checkBox_Reverse->isChecked()){
                    filterFuncX.push_back(1);
                }else{
                    filterFuncX.push_back(0);
                }
            }
        }

        if (filterID == 2){
            if( ui->checkBox_Reverse->isChecked()){
                filterFuncX.push_back( fabs(fx/freqH) / sqrt(1 + pow(fx/freqH,2) ) );
            }else{
                filterFuncX.push_back( 1. / sqrt(1 + pow(fx/freqH,2) ) );
            }

        }

        if( filterID == 4){
            double func = exp( - pow((fabs(fx) - freqH)/freqL,2) / 2 );
            if( ui->checkBox_Reverse->isChecked()){
                if( fabs(fx) < freqH ){
                    filterFuncX.push_back( func );
                }else{
                    filterFuncX.push_back(1);
                }
            }else{
                if( fabs(fx) > freqH ){
                    filterFuncX.push_back( func );
                }else{
                    filterFuncX.push_back(1);
                }
            }
        }
    }
    needToCalFilterFuncX = 0;
}

void FFTPlot::CalFilterFunctionY()
{
    filterFuncY.clear();

    //=================== y filter
    if( !ui->checkBox_EnableY->isChecked()){
        for(int j = 0; j < ny; j++){
            filterFuncY.push_back(1);
        }
        return;
    }


    double freqH = ui->verticalSlider_freqH->value();
    double freqL = 0;
    if(filterID == 3 || filterID == 4 ) freqL = ui->verticalSlider_freqL->value();

    for(int j = 0; j < ny; j++){
        double fy = file->GetfYData(j);

        if( filterID == 1 || filterID == 3){
            if( fabs(fy) <= freqH && fabs(fy) >= freqL){
                if( ui->checkBox_Reverse->isChecked()){
                    filterFuncY.push_back(0);
                }else{
                    filterFuncY.push_back(1);
                }
            }else{
                if( ui->checkBox_Reverse->isChecked()){
                    filterFuncY.push_back(1);
                }else{
                    filterFuncY.push_back(0);
                }
            }
        }

        if( filterID == 2){
            if( ui->checkBox_Reverse->isChecked()){
                filterFuncY.push_back( fabs(fy/freqH) / sqrt(1 + pow(fy/freqH,2) ) );
            }else{
                filterFuncY.push_back( 1. / sqrt(1 + pow(fy/freqH,2) ) );
            }

        }

        if( filterID == 4){
            double func = exp( - pow((fabs(fy) - freqH)/freqL ,2) / 2 );
            if( ui->checkBox_Reverse->isChecked()){
                if( fabs(fy) < freqH ){
                    filterFuncY.push_back( func );
                }else{
                    filterFuncY.push_back(1);
                }
            }else{
                if( fabs(fy) > freqH ){
                    filterFuncY.push_back( func );
                }else{
                    filterFuncY.push_back(1);
                }
            }
        }
    }

    needToCalFilterFuncY = 0;
}


void FFTPlot::on_pushButton_CalFFT_clicked()
{
    file->FouierForward();
    SetPlots();

    EnablePlanels(1);
    ui->checkBox_LogY->setChecked(0);
    ui->checkBox_RemoveConstant->setChecked(false);
    ui->checkBox_Reverse->setChecked(false);
    ui->checkBox_EnableY->setChecked(false);
    ui->checkBox_toAP->setChecked(true);


    enableSliderFunction = 0;
    ui->lineEdit_XfreqH->setText(QString::number(ui->horizontalSlider_freqH->value())+" kHz");
    on_checkBox_EnableY_clicked(false);
    on_checkBox_Reverse_clicked(false);
    enableSliderFunction = 1;

    ui->radioButton_LowPassSharp->setChecked(true);
    on_radioButton_LowPassSharp_clicked();


    filterApplied = 0;
    filterChanged = 0;
    needToCalFilterFuncX = 0;
    needToCalFilterFuncY = 0;

    ui->lineEdit_freqResol->setText("freq. Resol. = "+ QString::number( file->GetfResol() )+" kHz");

    ContourPlot();
    RescalePlots();

    plot_A->disconnect();
    plot_P->disconnect();
    connect(plot_A, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(GetPosOnPlotA(QMouseEvent*)));
    connect(plot_P, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(GetPosOnPlotP(QMouseEvent*)));
    connect(plot_A, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(GetPosOnPlotA(QMouseEvent*)));
    connect(plot_P, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(GetPosOnPlotP(QMouseEvent*)));
}

void FFTPlot::on_checkBox_RemoveConstant_clicked()
{
    file->RemoveYConstant();
    ContourPlot();

    ui->checkBox_RemoveConstant->setEnabled(0);
    ui->lineEdit_Msg->setText("The constant Y was removed from FFTW data.");

    filterApplied = 1;
}

void FFTPlot::on_horizontalSlider_freqH_valueChanged(int value)
{
    if( enableSliderFunction == 0) return;
    if( filterID == 3){
        ui->horizontalSlider_freqL->setMaximum(value-1);
    }

    double freq = ui->horizontalSlider_freqH->value();
    ui->lineEdit_XfreqH->setText(QString::number(freq) + " kHz");

    needToCalFilterFuncX = 1;
    ContourPlot();

    filterChanged = 1;
    ui->lineEdit_Msg->setText("The FFTW data did not changed. click \"Apply Filter\" ");
}


void FFTPlot::on_lineEdit_XfreqH_editingFinished()
{
    QString temp = ui->lineEdit_XfreqH->text();
    double freq = ExtractNumber(temp);
    ui->lineEdit_XfreqH->setText(QString::number(freq) + " kHz");
    ui->horizontalSlider_freqH->setValue(qCeil(freq));
    if( filterID == 3) ui->horizontalSlider_freqL->setMaximum(qCeil(freq)-1);

    needToCalFilterFuncX = 1;
    ContourPlot();

    filterChanged = 1;
    ui->lineEdit_Msg->setText("The FFTW data did not changed. chlick \"Apply Filter\" ");
}


void FFTPlot::on_horizontalSlider_freqL_valueChanged(int value)
{
    if( enableSliderFunction == 0) return;
    if(filterID == 3){
        ui->horizontalSlider_freqH->setMinimum(value+1);
    }
    double freq = ui->horizontalSlider_freqL->value();
    ui->lineEdit_XfreqL->setText(QString::number(freq) + " kHz");

    needToCalFilterFuncX = 1;
    ContourPlot();

    filterChanged = 1;
    ui->lineEdit_Msg->setText("The FFTW data did not changed. chlick \"Apply Filter\" ");
}

void FFTPlot::on_lineEdit_XfreqL_editingFinished()
{
    QString temp = ui->lineEdit_XfreqL->text();
    double freq = ExtractNumber(temp);
    ui->lineEdit_XfreqL->setText(QString::number(freq) + " kHz");
    ui->horizontalSlider_freqL->setValue(qCeil(freq));
    if(filterID == 3) ui->horizontalSlider_freqH->setMinimum(qCeil(freq)+1);

    needToCalFilterFuncX = 1;
    ContourPlot();

    filterChanged = 1;
    ui->lineEdit_Msg->setText("The FFTW data did not changed. chlick \"Apply Filter\" ");
}

void FFTPlot::on_pushButton_ApplyFilter_clicked()
{
    QVector<double> par;
    par.clear();
    par.push_back(ui->horizontalSlider_freqH->value());
    par.push_back(ui->horizontalSlider_freqL->value());

    file->FFTWFilters(filterID, par, filterFuncX, filterFuncY, ui->checkBox_Reverse->isChecked());

    ContourPlot();
    RescalePlots();

    filterApplied = 1;
    filterChanged = 0;

    ui->lineEdit_Msg->setText("Filter applied on the FFTW data.");

}

void FFTPlot::on_pushButton_FFTWBackward_clicked()
{
    if( filterApplied){
        file->FouierBackward();
        PlotData();
        filterApplied = 0;
        filterChanged = 0;
    }else{
        ui->lineEdit_Msg->setText("Please Apply filter.");
    }

}


void FFTPlot::on_pushButton_ResetPlot_clicked()
{
    colorMap_A->rescaleKeyAxis();
    colorMap_P->rescaleKeyAxis();
    plot_A->replot();
    plot_P->replot();

    plot_X->rescaleAxes();
    if(ui->checkBox_LogY->isChecked()){
        plot_X->yAxis->setRangeLower(1e-2);
    }else{
        plot_X->yAxis->setRangeLower(0);
    }
    plot_X->replot();

    RescalePlots();
}

void FFTPlot::on_radioButton_LowPassSharp_clicked()
{
    ui->horizontalSlider_freqH->setMinimum(0);
    ui->horizontalSlider_freqL->setEnabled(0);

    ui->lineEdit_XfreqL->setEnabled(0);
    ui->lineEdit_XfreqL->setText("");

    ui->label_XfreqH->setText("freq.");
    ui->label_XfreqL->setText("");

    if( ui->checkBox_EnableY->isChecked()){
        ui->verticalSlider_freqH->setMinimum(0);
        ui->verticalSlider_freqL->setEnabled(0);

        ui->lineEdit_YfreqL->setEnabled(0);
        ui->lineEdit_YfreqL->setText("");

        ui->label_YfreqH->setText("freq.");
        ui->label_YfreqL->setText("");
    }

    filterID = 1;
    needToCalFilterFuncX = 1;
    needToCalFilterFuncY = 1;
    ContourPlot();
    RescalePlots();
}

void FFTPlot::on_radioButton_LowPass_clicked()
{
    //============ X
    ui->horizontalSlider_freqH->setMinimum(1);
    ui->horizontalSlider_freqL->setEnabled(0);

    ui->lineEdit_XfreqL->setEnabled(0);
    ui->lineEdit_XfreqL->setText("");

    ui->label_XfreqH->setText("freq.");
    ui->label_XfreqL->setText("");

    //============ Y
    if( ui->checkBox_EnableY->isChecked()){
        ui->verticalSlider_freqH->setMinimum(1);
        ui->verticalSlider_freqL->setEnabled(0);

        ui->lineEdit_YfreqL->setEnabled(0);
        ui->lineEdit_YfreqL->setText("");

        ui->label_YfreqH->setText("freq.");
        ui->label_YfreqL->setText("");
    }

    filterID = 2;
    needToCalFilterFuncX = 1;
    needToCalFilterFuncY = 1;
    ContourPlot();
    RescalePlots();
}

void FFTPlot::on_radioButton_BandPassSharp_clicked()
{
    //=========== X
    ui->label_XfreqH->setText("freq. High");
    ui->label_XfreqL->setText("freq. Low");

    ui->horizontalSlider_freqL->setEnabled(1);
    ui->horizontalSlider_freqL->setMinimum(0);
    ui->horizontalSlider_freqL->setValue(0);
    ui->horizontalSlider_freqH->setMinimum(ui->horizontalSlider_freqL->value());

    ui->lineEdit_XfreqL->setEnabled(1);
    double freq = ui->horizontalSlider_freqL->value();
    ui->lineEdit_XfreqL->setText(QString::number(freq) + " kHz");

    //=========== Y
    if( ui->checkBox_EnableY->isChecked()){
        ui->label_YfreqH->setText("freq. High");
        ui->label_YfreqL->setText("freq. Low");

        ui->verticalSlider_freqL->setEnabled(1);
        ui->verticalSlider_freqL->setMinimum(0);
        ui->verticalSlider_freqL->setValue(0);
        ui->verticalSlider_freqH->setMinimum(ui->verticalSlider_freqL->value());

        ui->lineEdit_YfreqL->setEnabled(1);
        freq = ui->verticalSlider_freqL->value();
        ui->lineEdit_YfreqL->setText(QString::number(freq));
    }

    filterID = 3;
    needToCalFilterFuncX = 1;
    needToCalFilterFuncY = 1;
    ContourPlot();
    RescalePlots();
}

void FFTPlot::on_radioButton_Guassian_clicked()
{
    //=========== X
    ui->label_XfreqH->setText("freq. Peak");
    ui->label_XfreqL->setText("freq. Width");

    ui->horizontalSlider_freqH->setMinimum(0);
    ui->horizontalSlider_freqL->setEnabled(1);
    ui->horizontalSlider_freqL->setMinimum(1);
    ui->horizontalSlider_freqL->setMaximum(xMax);
    ui->horizontalSlider_freqL->setValue(50);

    ui->lineEdit_XfreqL->setEnabled(1);
    ui->lineEdit_XfreqL->setText(QString::number(50) + " kHz");

    //=========== Y
    if( ui->checkBox_EnableY->isChecked()){
        ui->label_YfreqH->setText("freq. Peak");
        ui->label_YfreqL->setText("freq. Width");

        ui->verticalSlider_freqH->setMinimum(0);
        ui->verticalSlider_freqL->setEnabled(1);
        ui->verticalSlider_freqL->setMinimum(1);
        ui->verticalSlider_freqL->setMaximum(yMax);
        ui->verticalSlider_freqL->setValue(10);

        ui->lineEdit_XfreqL->setEnabled(1);
        ui->lineEdit_XfreqL->setText(QString::number(10));
    }

    filterID = 4;
    needToCalFilterFuncX = 1;
    needToCalFilterFuncY = 1;
    ContourPlot();
    RescalePlots();
}

double FFTPlot::ExtractNumber(QString str)
{
    int pos = str.size();
    for(int i = 0; i < str.size() ; i++){
        if( str[i].isLetter()){
            pos = i;
            break;
        }
    }
    str.chop(str.size()-pos);
    return str.toDouble();
}


void FFTPlot::on_checkBox_LogY_clicked(bool checked)
{
    if( checked ){
        plot_X->yAxis->setScaleType(QCPAxis::stLogarithmic);
        plot_X->yAxis->setRangeLower(1e-2);
    }else{
        plot_X->yAxis->setScaleType(QCPAxis::stLinear);
        plot_X->yAxis->rescale();
    }

    plot_X->replot();
}

void FFTPlot::on_checkBox_Reverse_clicked(bool checked)
{
    if( checked ){
        ui->radioButton_LowPassSharp->setText("High-Pass Sharp Filter");
        ui->radioButton_LowPass->setText("High-Pass Filter");
        ui->radioButton_BandPassSharp->setText("Band-Block Sharp Filter");
        ui->radioButton_Guassian->setText("Guassian-Block Filter");

    }else{
        ui->radioButton_LowPassSharp->setText("Low-Pass Sharp Filter");
        ui->radioButton_LowPass->setText("Low-Pass Filter");
        ui->radioButton_BandPassSharp->setText("Band-Pass Sharp Filter");
        ui->radioButton_Guassian->setText("Guassian-Pass Filter");
    }

    needToCalFilterFuncX = 1;
    needToCalFilterFuncY = 1;

    ContourPlot();
}

void FFTPlot::on_checkBox_toAP_clicked()
{
    ContourPlot();
    RescalePlots();
}

void FFTPlot::on_checkBox_EnableY_clicked(bool checked)
{
    ui->checkBox_EnableY->setChecked(checked);
    ui->lineEdit_YfreqH->setEnabled(checked);
    ui->verticalSlider_freqH->setEnabled(checked);

    if( filterID == 3 || filterID == 4){
        ui->lineEdit_YfreqL->setEnabled(checked);
        ui->verticalSlider_freqL->setEnabled(checked);
    }
    if( checked){
        ui->label_YfreqH->setText("freq.");
        ui->lineEdit_YfreqH->setText(QString::number(ui->verticalSlider_freqH->value()));
        switch (filterID) {
        case 1:
            ui->label_YfreqL->setText("");
            ui->lineEdit_YfreqL->setText("");
            break;
        case 2:
            ui->label_YfreqL->setText("");
            ui->lineEdit_YfreqL->setText("");
            break;
        case 3:
            ui->label_YfreqH->setText("freq. High");
            ui->label_YfreqL->setText("freq. Low");
            ui->lineEdit_YfreqL->setText(QString::number(ui->verticalSlider_freqL->value()));
            break;
        case 4:
            ui->label_YfreqH->setText("freq. Peak");
            ui->label_YfreqL->setText("freq. Width");
            ui->lineEdit_YfreqL->setText(QString::number(ui->verticalSlider_freqL->value()));
            break;
        }
    }else{
        ui->label_YfreqH->setText("");
        ui->label_YfreqL->setText("");
        ui->lineEdit_YfreqH->setText("");
        ui->lineEdit_YfreqL->setText("");
    }

    needToCalFilterFuncY = 1;

    plot_A->graph(1)->clearData();
    plot_P->graph(1)->clearData();

    ContourPlot();
}

void FFTPlot::on_verticalSlider_freqH_valueChanged(int value)
{
    if( enableSliderFunction == 0 ) return;
    if( filterID == 3){
        ui->verticalSlider_freqL->setMaximum(value-1);
    }

    ui->lineEdit_YfreqH->setText(QString::number(value));

    needToCalFilterFuncY = 1;
    ContourPlot();

    filterChanged = 1;
    ui->lineEdit_Msg->setText("The FFTW data did not changed. chlick \"Apply Filter\" ");
}

void FFTPlot::on_lineEdit_YfreqH_editingFinished()
{
    double freq = ui->lineEdit_YfreqH->text().toDouble();
    ui->verticalSlider_freqH->setValue(freq);
    if( filterID == 3) ui->verticalSlider_freqL->setMaximum(freq-1);

    needToCalFilterFuncY = 1;
    ContourPlot();

    filterChanged = 1;
    ui->lineEdit_Msg->setText("The FFTW data did not changed. chlick \"Apply Filter\" ");
}

void FFTPlot::on_verticalSlider_freqL_valueChanged(int value)
{
    if( enableSliderFunction == 0 ) return;
    if( filterID == 3){
        ui->verticalSlider_freqH->setMinimum(value+1);
    }
    ui->lineEdit_YfreqL->setText(QString::number(value));

    needToCalFilterFuncY = 1;
    ContourPlot();

    filterChanged = 1;
    ui->lineEdit_Msg->setText("The FFTW data did not changed. chlick \"Apply Filter\" ");
}


void FFTPlot::on_lineEdit_YfreqL_editingFinished()
{
    double freq = ui->lineEdit_YfreqL->text().toDouble();
    ui->verticalSlider_freqL->setValue(freq);
    if( filterID == 3) ui->verticalSlider_freqH->setMinimum(freq+1);

    needToCalFilterFuncY = 1;
    ContourPlot();

    filterChanged = 1;
    ui->lineEdit_Msg->setText("The FFTW data did not changed. chlick \"Apply Filter\" ");

}

void FFTPlot::GetPosOnPlotA(QMouseEvent *mouse)
{
    QPoint pt = mouse->pos();
    double x = plot_A->xAxis->pixelToCoord(pt.rx());
    double y = plot_A->yAxis->pixelToCoord(pt.ry());

    QPoint coord(x,y);

    ShowLinesOnPlotA(coord);
    ShowLinesOnPlotP(coord);

    if( mouse->button() == Qt::LeftButton){
        ui->horizontalSlider_freqH->setValue(fabs(x));
        if(ui->checkBox_EnableY->isChecked()){
            ui->verticalSlider_freqH->setValue(fabs(y));
        }
    }else if(mouse->button() == Qt::RightButton){
        if( filterID == 3 || filterID == 4){
            ui->horizontalSlider_freqL->setValue(fabs(x));
            if(ui->checkBox_EnableY->isChecked()){
                ui->verticalSlider_freqL->setValue(fabs(y));
            }
        }
    }

}

void FFTPlot::GetPosOnPlotP(QMouseEvent *mouse)
{
    QPoint pt = mouse->pos();
    double x = plot_P->xAxis->pixelToCoord(pt.rx());
    double y = plot_P->yAxis->pixelToCoord(pt.ry());

    QPoint coord(x,y);

    ShowLinesOnPlotA(coord);
    ShowLinesOnPlotP(coord);
}

void FFTPlot::ShowLinesOnPlotA(QPoint pt)
{
    QVector<double> lineX, lineY;
    lineX.push_back(pt.rx());
    lineX.push_back(pt.rx());

    lineY.push_back(plot_A->yAxis->range().upper);
    lineY.push_back(plot_A->yAxis->range().lower);

    plot_A->graph(0)->clearData();
    plot_A->graph(0)->addData(lineX, lineY);

    if(ui->checkBox_EnableY->isChecked()){
        lineX.clear();
        lineY.clear();
        lineX.push_back(plot_A->xAxis->range().upper);
        lineX.push_back(plot_A->xAxis->range().lower);
        lineY.push_back(pt.ry());
        lineY.push_back(pt.ry());
        plot_A->graph(1)->clearData();
        plot_A->graph(1)->addData(lineX, lineY);
    }

    plot_A->replot();

}

void FFTPlot::ShowLinesOnPlotP(QPoint pt)
{
    QVector<double> lineX, lineY;
    lineX.push_back(pt.rx());
    lineX.push_back(pt.rx());

    lineY.push_back(plot_P->yAxis->range().upper);
    lineY.push_back(plot_P->yAxis->range().lower);

    plot_P->graph(0)->clearData();
    plot_P->graph(0)->addData(lineX, lineY);

    if(ui->checkBox_EnableY->isChecked()){
        lineX.clear();
        lineY.clear();
        lineX.push_back(plot_P->xAxis->range().upper);
        lineX.push_back(plot_P->xAxis->range().lower);
        lineY.push_back(pt.ry());
        lineY.push_back(pt.ry());
        plot_P->graph(1)->clearData();
        plot_P->graph(1)->addData(lineX, lineY);
    }

    plot_P->replot();
}

