#include "fftplot.h"
#include "ui_fftplot.h"

FFTPlot::FFTPlot(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FFTPlot)
{
    ui->setupUi(this);

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

}

FFTPlot::~FFTPlot()
{

    delete colorMap_A;
    delete plot_A;
    delete colorMap_P;
    delete plot_P;

    delete ui;
}

void FFTPlot::SetData(FileIO *file)
{
    this->file = file;

    colorMap_A->data()->clear();
    colorMap_P->data()->clear();

    plot_A->replot();
    plot_P->replot();


    ui->checkBox_RemoveConstant->setEnabled(0);
    ui->checkBox_RemoveConstant->setChecked(false);
    ui->checkBox_MovingAvg->setEnabled(0);
    ui->spinBox_MovingAvg->setEnabled(0);

    ui->lineEdit_freqL->setEnabled(0);
    ui->lineEdit_freqH->setEnabled(0);
    ui->horizontalSlider_freqH->setEnabled(0);
    ui->horizontalSlider_freqL->setEnabled(0);

    ui->radioButton_LowPass->setEnabled(0);
    ui->radioButton_LowPassSharp->setEnabled(0);
    ui->radioButton_BandPassSharp->setEnabled(0);
    ui->radioButton_Guassian->setEnabled(0);

    ui->pushButton_ApplyFilter->setEnabled(0);

    ui->pushButton_FFTWBackward->setEnabled(0);
}

void FFTPlot::SetFrequency()
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

    ui->horizontalSlider_freqH->setMinimum(1);
    ui->horizontalSlider_freqH->setMaximum(xMax);
    ui->horizontalSlider_freqH->setValue(xMax);
    //ui->lineEdit_freqH->setText(QString::number(xMax) + " kHz");

    ui->horizontalSlider_freqL->setMinimum(0);
    ui->horizontalSlider_freqL->setMaximum(xMax);
    ui->horizontalSlider_freqL->setValue(0);
    //ui->lineEdit_freqL->setText("");

}

void FFTPlot::ContourPlot()
{

    int nx = file->GetDataSize();
    int ny = file->GetDataSetSize();

    QVector<double> *dataA = file->GetFFTDataA();
    QVector<double> *dataP = file->GetFFTDataP();

    double freqH = ui->horizontalSlider_freqH->value();
    double freqL = 0;
    if( filterID == 3 || filterID == 4){
        freqL = ui->horizontalSlider_freqL->value();
        //qDebug("%f, %f", freqL, freqH);
    }


    for( int j = 0; j < ny; j++){
        for( int i = 0; i < nx; i++){

            double fx = file->GetfXData(i);

            if( filterID == 1 || filterID == 3){
                if( fabs(fx) <= freqH && fabs(fx) >= freqL){
                    colorMap_A->data()->setCell(i,j, dataA[j][i]);
                    colorMap_P->data()->setCell(i,j, dataP[j][i]);
                }else{
                    colorMap_A->data()->setCell(i,j, 0);
                    colorMap_P->data()->setCell(i,j, 0);
                }
            }

            if (filterID == 2){
                colorMap_A->data()->setCell(i,j, dataA[j][i] / sqrt(1 + pow(fx/freqH,2)));
                colorMap_P->data()->setCell(i,j, dataP[j][i] / sqrt(1 + pow(fx/freqH,2)));
            }

            if( filterID == 4){
                if( fabs(fx) > freqH ){
                    colorMap_A->data()->setCell(i,j, dataA[j][i] * exp( - pow((fabs(fx) - freqH)/freqL,2) / 2 ) );
                    colorMap_P->data()->setCell(i,j, dataP[j][i] * exp( - pow((fabs(fx) - freqH)/freqL,2) / 2 ) );
                }else{
                    colorMap_A->data()->setCell(i,j, dataA[j][i]);
                    colorMap_P->data()->setCell(i,j, dataP[j][i]);
                }
            }

        }
    }

    dataA = NULL;
    dataP = NULL;

    colorMap_A->rescaleDataRange();
    colorMap_P->rescaleDataRange();

    plot_A->replot();
    plot_P->replot();
}

void FFTPlot::on_pushButton_CalFFT_clicked()
{
    file->FouierForward();
    SetFrequency();


    ui->checkBox_RemoveConstant->setEnabled(1);
    ui->checkBox_RemoveConstant->setChecked(false);

    //ui->checkBox_MovingAvg->setEnabled(1);
    //ui->spinBox_MovingAvg->setEnabled(1);

    ui->radioButton_LowPassSharp->setEnabled(1);
    ui->radioButton_LowPass->setEnabled(1);
    ui->radioButton_BandPassSharp->setEnabled(1);
    ui->radioButton_Guassian->setEnabled(1);

    ui->horizontalSlider_freqH->setEnabled(1);
    ui->lineEdit_freqH->setEnabled(1);

    ui->radioButton_LowPassSharp->setChecked(true);
    on_radioButton_LowPassSharp_clicked();
    on_horizontalSlider_freqH_sliderMoved(ui->horizontalSlider_freqH->maximum());

    ui->pushButton_ApplyFilter->setEnabled(1);
    ui->pushButton_FFTWBackward->setEnabled(1);

    filterApplied = 0;
    filterChanged = 0;

    ContourPlot();
}

void FFTPlot::on_checkBox_RemoveConstant_clicked()
{
    file->RemoveYConstant();
    ContourPlot();

    ui->checkBox_RemoveConstant->setEnabled(0);
    ui->lineEdit_Msg->setText("The constant Y was removed from FFTW data.");
}

void FFTPlot::on_horizontalSlider_freqH_sliderMoved(int position)
{
    if( filterID != 4){
        ui->horizontalSlider_freqL->setMaximum(position);
    }

    double freq = ui->horizontalSlider_freqH->value();

    ui->lineEdit_freqH->setText(QString::number(freq) + " kHz");
    ContourPlot();

    filterChanged = 1;
    ui->lineEdit_Msg->setText("The FFTW data did not changed. chlick \"Apply Filter\" ");
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


void FFTPlot::on_horizontalSlider_freqL_sliderMoved(int position)
{
    if(filterID != 4){
        ui->horizontalSlider_freqH->setMinimum(position);
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

    file->FFTWFilters(filterID, par);

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


void FFTPlot::on_pushButton_clicked()
{
    colorMap_A->rescaleKeyAxis();
    colorMap_P->rescaleKeyAxis();
    plot_A->replot();
    plot_P->replot();
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
    ui->horizontalSlider_freqH->setMinimum(ui->horizontalSlider_freqL->value());
    double freq = ui->horizontalSlider_freqL->value();
    ui->lineEdit_freqL->setText(QString::number(freq) + " kHz");

    filterID = 3;
    ContourPlot();
}

void FFTPlot::on_radioButton_Guassian_clicked()
{
    ui->label_freqH->setText("freqeuncy Mean");
    ui->label_freqL->setText("frequency Width");

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



