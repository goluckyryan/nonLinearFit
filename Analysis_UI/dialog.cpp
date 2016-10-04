#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    fitPar(NULL),
    fitParError(NULL),
    SSR(NULL)
{
    ui->setupUi(this);
    plot = ui->widget;

    //set plot interaction
    plot->setInteraction(QCP::iRangeDrag,true);
    plot->setInteraction(QCP::iRangeZoom,true);
    plot->axisRect()->setRangeDrag(Qt::Vertical);
    plot->axisRect()->setRangeZoom(Qt::Vertical);

    //set yaxis label
    plot->xAxis->setLabel("yIndex");

    //set 5 plots.
    for(int i = 0; i < 5 ; i++) {
        plot->addGraph();
        plot->graph(i)->setLineStyle(QCPGraph::lsNone);
        plot->graph(i)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle , 3));
        plot->graph(i)->setErrorType(QCPGraph::etValue);
    }

    //set plot color
    plot->graph(0)->setPen(QPen(Qt::blue));
    plot->graph(1)->setPen(QPen(Qt::red));
    plot->graph(2)->setPen(QPen(Qt::green));
    plot->graph(3)->setPen(QPen(Qt::magenta));
    plot->graph(4)->setPen(QPen(Qt::darkYellow ));
    plot->graph(0)->setErrorPen(QPen(Qt::blue));
    plot->graph(1)->setErrorPen(QPen(Qt::red));
    plot->graph(2)->setErrorPen(QPen(Qt::green));
    plot->graph(3)->setErrorPen(QPen(Qt::magenta));
    plot->graph(4)->setErrorPen(QPen(Qt::darkYellow));

    fixedSize = 0;
}

Dialog::~Dialog()
{
    delete ui;
    delete plot;
    if( fitPar != NULL) delete fitPar;
    if( fitParError != NULL) delete fitParError;
    if( SSR != NULL) delete SSR;
}

void Dialog::ClearData()
{
    fixedSize = 0;
    if( fitPar != NULL) delete fitPar;
    if( fitParError != NULL) delete fitParError;
    if( SSR != NULL) delete SSR;

    fitPar = NULL;
    fitParError = NULL;
    SSR = NULL;

    parSize = 4;

}

void Dialog::SetDataSize(int n)
{
    if( fixedSize == 1) return;

    dataSize = n;
    fitPar = new QVector<double> [n];
    fitParError = new QVector<double> [n];
    SSR = new double [n];

    fixedSize = 1;
}

void Dialog::SetAvalibleData(int n)
{
    parSize = n;
    if( n == 3){
        ui->checkBox_b->setEnabled(0);
        ui->checkBox_Tb->setEnabled(0);
        ui->checkBox_c->setEnabled(1);

        ui->checkBox_b->setChecked(0);
        ui->checkBox_Tb->setChecked(0);

    }else if( n == 4){
        ui->checkBox_b->setEnabled(1);
        ui->checkBox_Tb->setEnabled(1);
        ui->checkBox_c->setEnabled(0);

        ui->checkBox_c->setChecked(0);

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
    }
}

void Dialog::FillData(Analysis *ana)
{
    int yIndex = ana->GetYIndex();
    fitPar[yIndex] = ana->GetParameters();
    fitParError[yIndex] = ana->GetParError();
    SSR[yIndex] = ana->GetSSR();

    //qDebug() << fitPar[yIndex];
}

void Dialog::PlotData()
{
    if( fixedSize == 0) return;

    //get what to plot
    int plotFlag = 0;
//    /bool p = ui->checkBox_a->isChecked();
    on_checkBox_a_clicked(ui->checkBox_a->isChecked());
    //if( ui->checkBox_a->isChecked() ){
    //    PlotSingleData(0);
    //}else{
    //    plot->graph(0)->clearData();
    //    plot->replot();
    //}

    if( ui->checkBox_Ta->isChecked() ){
        PlotSingleData(1);
    }else{
        plot->graph(1)->clearData();
        plot->replot();
    }

    if( ui->checkBox_b->isChecked() ){
        PlotSingleData(2);
    }else{
        plot->graph(2)->clearData();
        plot->replot();
    }

    if( ui->checkBox_Tb->isChecked() ){
        PlotSingleData(3);
    }else{
        plot->graph(3)->clearData();
        plot->replot();
    }

    if( ui->checkBox_c->isChecked() ){
        PlotSingleData(4);
    }else{
        plot->graph(4)->clearData();
        plot->replot();
    }

}

void Dialog::PlotSingleData(int plotID){
    // plotID, 1 = a, 2 = Ta, 3 = b, 4 = Tb, 5 = c

    int index = plotID;
    if( plotID == 2 || plotID == 3){ // plot b or Tb
        if( parSize == 2) return; // coz no b
        if( parSize == 3) return; // only a , Ta, c
        if( parSize == 4 || parSize == 5) index = plotID;
    }else if(plotID == 4){ // plat c
        if( parSize == 2) return;
        if( parSize == 3) index = 2;
        if( parSize == 4) return;
        if( parSize == 5) index = 4;
    }

    QVector<double> x, y, ye;
    for(int i = 0; i < dataSize; i++){
        x.push_back(i);
        if( fitPar[i].size() != parSize){
            y.push_back(0);
            ye.push_back(0);
        }else{
            y.push_back(fitPar[i][index]);
            ye.push_back(fitParError[i][index]);
        }
    }

    plot->graph(plotID)->setDataValueError(x,y, ye);
    plot->xAxis->setRange(x[0],x[dataSize-1]);
    plot->graph(plotID)->rescaleAxes(true);
    plot->replot();

}

void Dialog::on_pushButton_clicked()
{
    PlotData();
}

void Dialog::on_checkBox_a_clicked(bool checked)
{
    if( fixedSize == 0) return;
    if(checked){
        PlotSingleData(0);
    }else{
        plot->graph(0)->clearData();
    }
    plot->replot();
}

void Dialog::on_checkBox_Ta_clicked(bool checked)
{
    if( fixedSize == 0) return;
    if(checked){
        PlotSingleData(1);
    }else{
        plot->graph(1)->clearData();
    }
    plot->replot();
}

void Dialog::on_checkBox_b_clicked(bool checked)
{
    if( fixedSize == 0) return;
    if(checked){
        PlotSingleData(2);
    }else{
        plot->graph(2)->clearData();
    }
    plot->replot();
}

void Dialog::on_checkBox_Tb_clicked(bool checked)
{
    if( fixedSize == 0) return;
    if(checked){
        PlotSingleData(3);
    }else{
        plot->graph(3)->clearData();
    }
    plot->replot();
}

void Dialog::on_checkBox_c_clicked(bool checked)
{
    if( fixedSize == 0) return;
    if(checked){
        PlotSingleData(4);
    }else{
        plot->graph(4)->clearData();
    }
    plot->replot();
}
