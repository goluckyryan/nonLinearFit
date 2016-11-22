#include "fftplot.h"
#include "ui_fftplot.h"

FFTPlot::FFTPlot(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FFTPlot)
{
    ui->setupUi(this);

    plot_A = ui->widget_A;
    plot_A->axisRect()->setupFullAxesBox(true);
    plot_A->xAxis->setLabel("x");
    plot_A->yAxis->setLabel("y");

    colorMap_A = new QCPColorMap(plot_A->xAxis, plot_A->yAxis);
    colorMap_A->clearData();

    plot_P = ui->widget_P;
    plot_P->axisRect()->setupFullAxesBox(true);
    plot_P->xAxis->setLabel("x");
    plot_P->yAxis->setLabel("y");

    colorMap_P = new QCPColorMap(plot_P->xAxis, plot_P->yAxis);
    colorMap_P->clearData();

}

FFTPlot::~FFTPlot()
{
    delete colorMap_A;
    delete plot_A;
    delete colorMap_P;
    delete plot_P;

    delete ui;
}

void FFTPlot::ContourPlot(int nx, int ny, QVector<double> *dataA, QVector<double> *dataP)
{

    colorMap_A->data()->setSize(nx,ny);
    colorMap_P->data()->setSize(nx,ny);

    for( int i = 0; i < nx; i++){
        for( int j = 0; j < ny; j++){
            double z = dataA[j][i];
            colorMap_A->data()->setCell(i,j, z);
            z = dataP[j][i];
            colorMap_P->data()->setCell(i,j, z);
        }
    }

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

    colorMap_A->rescaleDataRange();
    colorMap_P->rescaleDataRange();

    plot_A->rescaleAxes();
    plot_P->rescaleAxes();
    plot_A->replot();
    plot_P->replot();
}
