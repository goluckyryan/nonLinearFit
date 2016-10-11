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

    parSize = 4; // initial value

    //set plot interaction
    plot->setInteraction(QCP::iRangeDrag,true);
    plot->setInteraction(QCP::iRangeZoom,true);
    plot->axisRect()->setRangeDrag(Qt::Vertical);
    plot->axisRect()->setRangeZoom(Qt::Vertical);

    //set yaxis label
    plot->xAxis->setLabel("yIndex");

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

    fixedSize = 0;
}

Dialog::~Dialog()
{
    delete ui;
    delete plot;
    if( fitPar != NULL) delete [] fitPar;
    if( fitParError != NULL) delete [] fitParError;
    if( SSR != NULL) delete SSR;
}

void Dialog::ClearData()
{
    fixedSize = 0;
    if( fitPar != NULL) delete [] fitPar;
    if( fitParError != NULL) delete [] fitParError;
    if( SSR != NULL) delete SSR;

    fitPar = NULL;
    fitParError = NULL;
    SSR = NULL;

}

void Dialog::SetDataSize(int n)
{
    if( fixedSize == 1) return;

    dataSize = n;
    fitPar = new QVector<double> [n];
    fitParError = new QVector<double> [n];
    SSR = new double [n];

    fixedSize = 1;

    QVector<double> temp(5);
    for(int i = 0; i < n; i++){
        fitPar[i] = temp;
        fitParError[i] = temp;
        SSR[i] = 0;
    }

    QString msg;
    msg.sprintf("Initaliate fitPar array. size : %d", n);
    SendMsg(msg);
}

void Dialog::SetAvalibleData(int n)
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

void Dialog::FillData(Analysis *ana)
{
    int yIndex = ana->GetYIndex();
    this->parSize = ana->GetParametersSize(); // safty
    fitPar[yIndex] = ReSizeVector(ana->GetParameters());
    fitParError[yIndex] = ReSizeVector(ana->GetParError());
    int nDF = ana->GetNDF();
    SSR[yIndex] = ana->GetSSR()/nDF;
}

void Dialog::PlotData()
{
    if( fixedSize == 0) return;

    //get what to plot
    on_checkBox_a_clicked(ui->checkBox_a->isChecked());
    on_checkBox_Ta_clicked(ui->checkBox_Ta->isChecked());
    on_checkBox_b_clicked(ui->checkBox_b->isChecked());
    on_checkBox_Tb_clicked(ui->checkBox_Tb->isChecked());
    on_checkBox_c_clicked(ui->checkBox_c->isChecked());
    on_checkBox_abc_clicked(ui->checkBox_abc->isChecked());
    on_checkBox_SSR_clicked(ui->checkBox_SSR->isChecked());

}

void Dialog::PlotSingleData(int plotID){
    // plotID, 1 = a, 2 = Ta, 3 = b, 4 = Tb, 5 = c

    QVector<double> x, y, ye;
    for(int i = 0; i < dataSize; i++){
        x.push_back(i);
        y.push_back(fitPar[i][plotID]);
        ye.push_back(fitParError[i][plotID]);
    }

    plot->graph(plotID)->setDataValueError(x,y, ye);
    plot->xAxis->setRange(x[0], x[dataSize-1]);
    plot->graph(plotID)->rescaleAxes(true);
    plot->replot();

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

void Dialog::on_checkBox_abc_clicked(bool checked)
{
    if( fixedSize == 0 ) return;
    if( parSize < 3 ) return;

    if(checked){

        QVector<double> x, y, ye;
        for(int i = 0; i < dataSize; i++){
            x.push_back(i);

            double a = fitPar[i][0];
            double b = fitPar[i][2];
            double c = fitPar[i][4];
            double ea = fitParError[i][0];
            double eb = fitParError[i][2];
            double ec = fitParError[i][4];

            double ee = sqrt(ea*ea + eb*eb + ec*ec);
            y.push_back(a+b-c);
            ye.push_back(ee);

        }

        plot->graph(5)->setDataValueError(x,y, ye);
        plot->xAxis->setRange(x[0], x[dataSize-1]);
        plot->graph(5)->rescaleAxes(true);

    }else{
        plot->graph(5)->clearData();
    }
    plot->replot();

}

void Dialog::on_checkBox_SSR_clicked(bool checked)
{
    if( fixedSize == 0 ) return;
    if(checked){

        QVector<double> x, y;
        for(int i = 0; i < dataSize; i++){
            x.push_back(i);
            if( fitPar[i].size() != parSize){
                y.push_back(0);
            }else{
                y.push_back(SSR[i]);
            }
        }

        plot->graph(6)->setData(x,y);
        plot->xAxis->setRange(x[0], x[dataSize-1]);
        plot->graph(6)->rescaleAxes(true);

    }else{
        plot->graph(6)->clearData();
    }
    plot->replot();
}

void Dialog::on_pushButton_Save_clicked()
{
    if( fixedSize == 0 ) return;

    QString filename = OPENPATH;
    filename += "save.txt";
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
    tmp.sprintf("%10s \n", "SSR");
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

        tmp.sprintf("%10.3f \n", SSR[i]); lineout += tmp;

        stream << lineout;
    }

    savefile.close();
}

void Dialog::on_pushButton_ResetScale_clicked()
{
    plot->yAxis->setRange(-60,60);
    plot->replot();
}

QVector<double> Dialog::ReSizeVector(QVector<double> vec){
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
