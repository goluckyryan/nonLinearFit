#include "fftplot.h"
#include "ui_fftplot.h"

FFTPlot::FFTPlot(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FFTPlot)
{
    ui->setupUi(this);

    filterFunc.clear();

    plot_X = ui->widget_X;
    plot_X->addGraph(); // id = 0 amp or phase
    plot_X->addGraph(); // id = 1 filter
    plot_X->graph(0)->setPen(QPen(Qt::blue));
    plot_X->graph(1)->setPen(QPen(Qt::red));

    plot_X->xAxis->setLabel("freq [kHz]");
    //plot_X->xAxis2->setVisible(true);
    plot_X->xAxis2->setLabel("index");
    plot_X->yAxis->setLabel("FFTW Amp [a.u.]");
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

    colorMap_A->setDataScaleType(QCPAxis::stLogarithmic);
    colorMap_P->setDataScaleType(QCPAxis::stLogarithmic);

    plot_A->setInteraction(QCP::iRangeDrag,true);
    plot_A->setInteraction(QCP::iRangeZoom,true);
    plot_A->axisRect()->setRangeDrag(Qt::Horizontal);
    plot_A->axisRect()->setRangeZoom(Qt::Horizontal);

    plot_P->setInteraction(QCP::iRangeDrag,true);
    plot_P->setInteraction(QCP::iRangeZoom,true);
    plot_P->axisRect()->setRangeDrag(Qt::Horizontal);
    plot_P->axisRect()->setRangeZoom(Qt::Horizontal);

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

    filterFunc.clear();
}

void FFTPlot::EnablePlanels(bool IO)
{
    ui->checkBox_RemoveConstant->setEnabled(IO);
    ui->checkBox_RemoveConstant->setChecked(0);
    ui->checkBox_MovingAvg->setEnabled(0);
    ui->spinBox_MovingAvg->setEnabled(0);
    ui->checkBox_Reverse->setEnabled(IO);

    ui->lineEdit_freqL->setEnabled(IO);
    ui->lineEdit_freqH->setEnabled(IO);
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
    double xMin = file->GetfXMin();
    double xMax = file->GetfXMax();
    double yMin = file->GetfYMin();
    double yMax = file->GetfYMax();

    plot_A->xAxis->setRange(xMin, xMax);
    plot_P->xAxis->setRange(xMin, xMax);

    plot_A->yAxis->setRange(yMin, yMax);
    plot_P->yAxis->setRange(yMin, yMax);

    colorMap_A->data()->setRange(QCPRange(xMin, xMax), QCPRange(yMin, yMax));
    colorMap_P->data()->setRange(QCPRange(xMin, xMax), QCPRange(yMin, yMax));

    int nx = file->GetDataSize();
    int ny = file->GetDataSetSize();

    colorMap_A->data()->setSize(nx,ny);
    colorMap_P->data()->setSize(nx,ny);

    plot_A->rescaleAxes();
    plot_P->rescaleAxes();

    //==========
    plot_X->xAxis->setRange(xMin, xMax);
    plot_X->xAxis2->setRange(0,nx-1);
    plot_X->yAxis->setRangeLower(0);
    plot_X->rescaleAxes();

    //==========
    ui->horizontalSlider_freqH->setMinimum(1);
    ui->horizontalSlider_freqH->setMaximum(xMax);
    ui->horizontalSlider_freqH->setValue(xMax);

    ui->horizontalSlider_freqL->setMinimum(0);
    ui->horizontalSlider_freqL->setMaximum(xMax);
    ui->horizontalSlider_freqL->setValue(0);

    enableSliderFunction = 1;
}

void FFTPlot::ContourPlot()
{
    //SetPlots();

    int nx = file->GetDataSize();
    int ny = file->GetDataSetSize();

    QVector<double> *dataA = file->GetFFTDataA();
    QVector<double> *dataP = file->GetFFTDataP();

    filterFunc.clear();

    QVector<double> amp;

    double freqH = ui->horizontalSlider_freqH->value();
    double freqL = 0;
    if( filterID == 3 || filterID == 4){
        freqL = ui->horizontalSlider_freqL->value();
        //qDebug("%f, %f", freqL, freqH);
    }

    for( int i = 0; i < nx; i++){
        double fx = file->GetfXData(i);

        if( filterID == 1 || filterID == 3){
            if( fabs(fx) <= freqH && fabs(fx) >= freqL){
                if( ui->checkBox_Reverse->isChecked()){
                    filterFunc.push_back(0);
                }else{
                    filterFunc.push_back(1);
                }
            }else{
                if( ui->checkBox_Reverse->isChecked()){
                    filterFunc.push_back(1);
                }else{
                    filterFunc.push_back(0);
                }
            }
        }

        if (filterID == 2){
            if( ui->checkBox_Reverse->isChecked()){
                filterFunc.push_back( fabs(fx/freqH) / sqrt(1 + pow(fx/freqH,2) ) );
            }else{
                filterFunc.push_back( 1. / sqrt(1 + pow(fx/freqH,2) ) );
            }

        }

        if( filterID == 4){
            if( ui->checkBox_Reverse->isChecked()){
                if( fabs(fx) < freqH ){
                    filterFunc.push_back( exp( - pow((fabs(fx) - freqH)/freqL,2) / 2 ) );
                }else{
                    filterFunc.push_back(1);
                }
            }else{
                if( fabs(fx) > freqH ){
                    filterFunc.push_back( exp( - pow((fabs(fx) - freqH)/freqL,2) / 2 ) );
                }else{
                    filterFunc.push_back(1);
                }
            }
        }
    }

    for( int i = 0; i < nx; i++){
        double temp = 0;
        for( int j = 0; j < ny; j++){

            double za = dataA[j][i] * filterFunc[i];
            double zp = dataP[j][i] * filterFunc[i];

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

    colorMap_A->rescaleDataRange();
    if( ui->checkBox_toAP->isChecked()){
        colorMap_P->setDataScaleType(QCPAxis::stLinear);
    }else{
        colorMap_P->setDataScaleType(QCPAxis::stLogarithmic);
    }
    colorMap_P->rescaleDataRange();

    plot_A->replot();
    plot_P->replot();

    QVector<double> x = file->GetfXDataSet();

    plot_X->graph(0)->clearData();
    plot_X->graph(0)->addData(x, amp);

    plot_X->graph(1)->clearData();
    plot_X->graph(1)->addData(x, filterFunc);

    plot_X->yAxis->rescale();
    if(ui->checkBox_LogY->isChecked()){
        plot_X->yAxis->setRangeLower(1e-2);
    }else{
        plot_X->yAxis->setRangeLower(0);
    }
    plot_X->replot();

}


void FFTPlot::on_pushButton_CalFFT_clicked()
{
    file->FouierForward();
    SetPlots();

    EnablePlanels(1);
    ui->checkBox_LogY->setChecked(0);
    ui->checkBox_RemoveConstant->setChecked(false);
    ui->checkBox_Reverse->setChecked(false);

    ui->radioButton_LowPassSharp->setChecked(true);
    on_radioButton_LowPassSharp_clicked();
    on_horizontalSlider_freqH_valueChanged(ui->horizontalSlider_freqH->maximum());

    filterApplied = 0;
    filterChanged = 0;

    ui->lineEdit_freqResol->setText("freq. Resol. = "+ QString::number( file->GetfResol() )+" kHz");

    ContourPlot();
}

void FFTPlot::on_checkBox_RemoveConstant_clicked()
{
    file->RemoveYConstant();
    ContourPlot();

    ui->checkBox_RemoveConstant->setEnabled(0);
    ui->lineEdit_Msg->setText("The constant Y was removed from FFTW data.");
}

void FFTPlot::on_horizontalSlider_freqH_valueChanged(int value)
{
    if( enableSliderFunction == 0) return;
    if( filterID != 4){
        ui->horizontalSlider_freqL->setMaximum(value);
    }

    double freq = ui->horizontalSlider_freqH->value();

    ui->lineEdit_freqH->setText(QString::number(freq) + " kHz");
    ContourPlot();

    filterChanged = 1;
    ui->lineEdit_Msg->setText("The FFTW data did not changed. click \"Apply Filter\" ");
}


void FFTPlot::on_lineEdit_freqH_editingFinished()
{
    QString temp = ui->lineEdit_freqH->text();
    double freq = ExtractNumber(temp);
    ui->lineEdit_freqH->setText(QString::number(freq) + " kHz");
    ui->horizontalSlider_freqH->setValue(qCeil(freq));
    ui->horizontalSlider_freqL->setMaximum(qCeil(freq));
    ContourPlot();

    filterChanged = 1;
    ui->lineEdit_Msg->setText("The FFTW data did not changed. chlick \"Apply Filter\" ");
}


void FFTPlot::on_horizontalSlider_freqL_valueChanged(int value)
{
    if( enableSliderFunction == 0) return;
    if(filterID != 4){
        ui->horizontalSlider_freqH->setMinimum(value);
    }
    double freq = ui->horizontalSlider_freqL->value();
    ui->lineEdit_freqL->setText(QString::number(freq) + " kHz");
    ContourPlot();

    filterChanged = 1;
    ui->lineEdit_Msg->setText("The FFTW data did not changed. chlick \"Apply Filter\" ");
}

void FFTPlot::on_lineEdit_freqL_editingFinished()
{
    QString temp = ui->lineEdit_freqL->text();
    double freq = ExtractNumber(temp);
    ui->lineEdit_freqL->setText(QString::number(freq) + " kHz");
    ui->horizontalSlider_freqL->setValue(qCeil(freq));
    ui->horizontalSlider_freqH->setMinimum(qCeil(freq));
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

    file->FFTWFilters(filterID, par, filterFunc, ui->checkBox_Reverse->isChecked());

    ContourPlot();

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
}

void FFTPlot::on_radioButton_LowPassSharp_clicked()
{
    ui->horizontalSlider_freqH->setMinimum(1);
    ui->horizontalSlider_freqL->setEnabled(0);
    ui->lineEdit_freqL->setEnabled(0);
    ui->lineEdit_freqL->setText("");

    ui->label_freqH->setText("freqeuncy High");
    ui->label_freqL->setText("");

    filterID = 1;
    ContourPlot();
}

void FFTPlot::on_radioButton_LowPass_clicked()
{
    ui->horizontalSlider_freqH->setMinimum(1);

    ui->horizontalSlider_freqL->setEnabled(0);
    ui->lineEdit_freqL->setEnabled(0);
    ui->lineEdit_freqL->setText("");

    ui->label_freqH->setText("freqeuncy High");
    ui->label_freqL->setText("");

    filterID = 2;
    ContourPlot();
}

void FFTPlot::on_radioButton_BandPassSharp_clicked()
{
    ui->horizontalSlider_freqL->setEnabled(1);
    ui->lineEdit_freqL->setEnabled(1);
    ui->label_freqH->setText("freqeuncy High");
    ui->label_freqL->setText("freqeuncy Low");
    ui->horizontalSlider_freqL->setMinimum(0);
    ui->horizontalSlider_freqL->setValue(0);
    ui->horizontalSlider_freqH->setMinimum(ui->horizontalSlider_freqL->value());
    double freq = ui->horizontalSlider_freqL->value();
    ui->lineEdit_freqL->setText(QString::number(freq) + " kHz");

    filterID = 3;
    ContourPlot();
}

void FFTPlot::on_radioButton_Guassian_clicked()
{
    ui->label_freqH->setText("freqeuncy Mean");
    ui->label_freqL->setText("freqeuncy Width");

    ui->horizontalSlider_freqL->setEnabled(1);
    ui->horizontalSlider_freqL->setMaximum(ui->horizontalSlider_freqH->maximum());
    ui->lineEdit_freqL->setEnabled(1);
    ui->horizontalSlider_freqL->setMinimum(1);
    ui->horizontalSlider_freqL->setValue(50);
    ui->lineEdit_freqL->setText(QString::number(50) + " kHz");

    filterID = 4;
    ContourPlot();
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
        ui->radioButton_BandPassSharp->setText("Band-Black Sharp Filter");
        ui->radioButton_Guassian->setText("Guassian-Block Filter");
    }else{
        ui->radioButton_LowPassSharp->setText("Low-Pass Sharp Filter");
        ui->radioButton_LowPass->setText("Low-Pass Filter");
        ui->radioButton_BandPassSharp->setText("Band-Pass Sharp Filter");
        ui->radioButton_Guassian->setText("Guassian-Pass Filter");
    }

    ContourPlot();
}

void FFTPlot::on_checkBox_toAP_clicked(bool checked)
{
    ContourPlot();
}
