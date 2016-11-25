#include "fftplot.h"
#include "ui_fftplot.h"

FFTPlot::FFTPlot(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FFTPlot)
{
    ui->setupUi(this);

    plot_A = ui->widget_A;
    plot_A->axisRect()->setupFullAxesBox(true);
    plot_A->xAxis->setLabel("freq [MHz]");
    plot_A->yAxis->setLabel("index [a.u.]");

    colorMap_A = new QCPColorMap(plot_A->xAxis, plot_A->yAxis);
    colorMap_A->clearData();

    plot_P = ui->widget_P;
    plot_P->axisRect()->setupFullAxesBox(true);
    plot_P->xAxis->setLabel("freq [MHz]");
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

    ui->lineEdit->setEnabled(0);
    ui->radioButton_LowPass->setEnabled(0);
    ui->radioButton_LowPassSharp->setEnabled(0);
    ui->horizontalSlider->setEnabled(0);

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

    ui->horizontalSlider->setMinimum(1);
    ui->horizontalSlider->setMaximum(xMax);
    ui->horizontalSlider->setValue(xMax);

    ui->lineEdit->setText(QString::number(xMax) + " MHz");

}

void FFTPlot::ContourPlot()
{

    int nx = file->GetDataSize();
    int ny = file->GetDataSetSize();

    //colorMap_A->data()->clear();
    //colorMap_P->data()->clear();

    QVector<double> *dataA = file->GetFFTDataA();
    QVector<double> *dataP = file->GetFFTDataP();

    for( int j = 0; j < ny; j++){
        for( int i = 0; i < nx; i++){
            colorMap_A->data()->setCell(i,j, dataA[j][i]);
            colorMap_P->data()->setCell(i,j, dataP[j][i]);
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
    ContourPlot();

    ui->checkBox_RemoveConstant->setEnabled(1);
    ui->checkBox_RemoveConstant->setChecked(false);
    //ui->checkBox_MovingAvg->setEnabled(1);
    //ui->spinBox_MovingAvg->setEnabled(1);

    ui->lineEdit->setEnabled(1);
    //ui->radioButton_LowPass->setEnabled(1);
    ui->radioButton_LowPassSharp->setEnabled(1);
    ui->radioButton_LowPassSharp->setChecked(true);
    ui->horizontalSlider->setEnabled(1);

    ui->pushButton_ApplyFilter->setEnabled(1);

    ui->pushButton_FFTWBackward->setEnabled(1);
}

void FFTPlot::on_checkBox_RemoveConstant_clicked()
{
    file->RemoveYConstant();
    ContourPlot();

    ui->checkBox_RemoveConstant->setEnabled(0);
}

void FFTPlot::on_pushButton_FFTWBackward_clicked()
{
    file->FouierBackward();

    PlotData();
}

void FFTPlot::on_horizontalSlider_sliderMoved(int position)
{
    ui->lineEdit->setText(QString::number(position) + " MHz");
}


void FFTPlot::on_pushButton_ApplyFilter_clicked()
{
    QVector<double> par;
    par.clear();

    if(ui->radioButton_LowPassSharp->isChecked()){
        par.push_back(ui->horizontalSlider->value());
        file->FFTWFilters(1, par);
    }

    ContourPlot();

}

void FFTPlot::on_lineEdit_editingFinished()
{
    double freq = ui->lineEdit->text().toDouble();
    ui->lineEdit->setText(QString::number(freq) + " MHz");

    ui->horizontalSlider->setValue(qCeil(freq));
}
