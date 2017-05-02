#include "waveletplot.h"
#include "ui_waveletplot.h"

WaveletPlot::WaveletPlot(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WaveletPlot)
{
    ui->setupUi(this);

    file = NULL;
    wave = NULL;

    plot_W = ui->plot_W;
    plot_W->xAxis->setLabel("time [us]");
    plot_W->yAxis->setLabel("Octave");
    plot_W->yAxis->setAutoTickStep(false);
    plot_W->yAxis->setTickStep(1.);
    connect(plot_W, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(ShowMousePosition(QMouseEvent*)));
    connect(plot_W, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(SetLineByMouseClick(QMouseEvent*)));
    //plot_W->setInteraction(QCP::iRangeZoom, true);

    plot_V = ui->plot_V;
    plot_V->xAxis->setLabel("time [us]");
    plot_V->yAxis->setLabel("Octave");
    plot_V->yAxis->setAutoTickStep(false);
    plot_V->yAxis->setTickStep(1.);

    colorMap_W = new QCPColorMap(plot_W->xAxis, plot_W->yAxis);
    colorMap_V = new QCPColorMap(plot_V->xAxis, plot_V->yAxis);
    colorMap_W->clearData();
    colorMap_V->clearData();
    colorMap_W->setInterpolate(false);
    colorMap_V->setInterpolate(false);

    QCPColorScale *colorScale_W = new QCPColorScale(plot_W);
    plot_W->plotLayout()->addElement(0, 1, colorScale_W);
    colorScale_W->setType(QCPAxis::atRight);
    colorMap_W->setColorScale(colorScale_W);
    colorMap_W->setGradient(QCPColorGradient::gpJet );

    plot_W->addGraph(); // left line gate
    plot_W->addGraph(); // right line gate
    plot_W->graph(0)->setPen(QPen(Qt::white));
    plot_W->graph(0)->clearData();
    plot_W->graph(1)->setPen(QPen(Qt::white));
    plot_W->graph(1)->clearData();

    QCPColorScale *colorScale_V = new QCPColorScale(plot_V);
    plot_V->plotLayout()->addElement(0, 1, colorScale_V);
    colorScale_V->setType(QCPAxis::atRight);
    colorMap_V->setColorScale(colorScale_V);
    colorMap_V->setGradient(QCPColorGradient::gpJet );

    plot = ui->plot;
    plot->addGraph(); //original data
    plot->addGraph(); //denoised data
    plot->graph(0)->setPen(QPen(Qt::blue));
    plot->graph(1)->setPen(QPen(Qt::red));
    plot->xAxis->setLabel("time [us]");
    plot->graph(0)->clearData();
    plot->setInteraction(QCP::iRangeDrag,true);
    plot->setInteraction(QCP::iRangeZoom,true);
    plot->axisRect()->setRangeDrag(Qt::Horizontal);
    plot->axisRect()->setRangeZoom(Qt::Horizontal);
    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(plotXAxisChanged(QCPRange)));

    plot_Woct = ui->plot_Woct;
    plot_Woct->addGraph();
    plot_Woct->graph(0)->setPen(QPen(Qt::blue));
    plot_Woct->xAxis->setLabel("time [us]");
    plot_Woct->yAxis->setLabel("Volatge [a.u.]");
    plot_Woct->graph(0)->clearData();
    //plot_Woct->yAxis->setAutoTickStep(false);
    //plot_Woct->yAxis->setTickStep(1.0);

    plot_Voct = ui->plot_Voct;
    plot_Voct->addGraph();
    plot_Voct->graph(0)->setPen(QPen(Qt::blue));
    plot_Voct->xAxis->setLabel("time [us]");
    plot_Voct->yAxis->setLabel("Volatge [a.u.]");
    plot_Voct->graph(0)->clearData();
    //plot_Voct->yAxis->setAutoTickStep(false);
    //plot_Voct->yAxis->setTickStep(1.0);

    plot_energy = ui->plot_energy;
    plot_energy->addGraph(plot_energy->yAxis, plot_energy->xAxis);
    plot_energy->graph(0)->setPen(QPen(Qt::blue));
    plot_energy->addGraph(plot_energy->yAxis, plot_energy->xAxis);
    plot_energy->graph(1)->setPen(QPen(Qt::red));
    plot_energy->xAxis->setLabel("Fraction of Energy [%]");
    plot_energy->yAxis->setLabel("octave");
    //plot_energy->graph(0)->setLineStyle(QCPGraph::lsLine);
    plot_energy->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
    plot_energy->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
    plot_energy->xAxis->setAutoTickStep(false);
    plot_energy->yAxis->setAutoTickStep(false);
    plot_energy->xAxis->setTickStep(20.);
    plot_energy->yAxis->setTickStep(1.);
    plot_energy->yAxis->setLabelPadding(0);
    plot_energy->graph(0)->clearData();
    plot_energy->graph(1)->clearData();

    enableVerticalBar = 0;
    enableControl = true;

    x1 = -20;
    x2 = 50;
    ui->lineEdit_x1->setText(QString::number(x1));
    ui->lineEdit_x2->setText(QString::number(x2));

    QStringList WaveletList;
    WaveletList << "Haar" << "Daubechies" << "Symlet" << "Coiflet";
    ui->comboBox_Wavelet->addItems(WaveletList);
    ui->comboBox_Wavelet->setCurrentIndex(0);
    ui->spinBox_WaveletIndex->setEnabled(false);


    QStringList Thresholding;
    Thresholding << "Hard Threshold";
    Thresholding << "Hard + linear Octave";
    Thresholding << "Soft Threshold (linear)";
    //Thresholding << "Soft Threshold (x^n)";
    ui->comboBox_Thresholding->addItems(Thresholding);
    ui->comboBox_Thresholding->setCurrentIndex(0);

    ui->pushButton_Clean->setEnabled(false);
    ui->verticalSlider_Threshold->setEnabled(false);

}

WaveletPlot::~WaveletPlot()
{
    delete ui;
    delete colorMap_W;
    delete plot_W;
    delete colorMap_V;
    delete plot_V;
    delete plot;
    delete plot_Woct;
    delete plot_Voct;

    file = NULL;

    delete wave;
}

void WaveletPlot::SetData(FileIO *file, int yIndex)
{
    this->file = file;
    this->yIndex = yIndex;

    //QVector<double> y;
    //for( int i = 0; i < 1000; i++){
    //    y.push_back(qCos(i/20.));
    //}

    QVector<double> y = file->GetDataSetZ(yIndex);
    QVector<double> x = file->GetDataSetX();

    plot->graph(0)->clearData();
    plot->graph(0)->addData(x,y);
    plot->rescaleAxes();
    plot->replot();

    QString msg;
    msg.sprintf("======== Discrete Wavelet Analysis, y-Index = %d.", yIndex );
    SendMsg(msg);

    //wavelet decomposition
    int waveletID  = ui->comboBox_Wavelet->currentIndex();
    int waveletPar = 0;
    if( waveletID != 0 ) waveletPar = ui->spinBox_WaveletIndex->value();
    wave = new WaveletAnalysis(x, y); // 0 for Haar
    SendMsg(wave->GetMsg());
    wave->setWaveletPar(waveletID, waveletPar);
    SendMsg(wave->GetMsg());
    if( ui->checkBox_normalized->isChecked() ) {
        wave->setNormFactor( sqrt(2.));
    }else{
        wave->setNormFactor(1.);
    }
    wave->Decompose();
    SendMsg(wave->GetMsg());
    wave->CalculateEnergy();

    //setPlot
    int nx = wave->GetSize();
    int ny = wave->GetMaxScale();
    colorMap_W->data()->setRange(QCPRange(x[0],x[nx-1]), QCPRange(0,-ny));
    colorMap_W->data()->setSize(nx, ny+1);
    colorMap_V->data()->setRange(QCPRange(x[0],x[nx-1]), QCPRange(0,-ny));
    colorMap_V->data()->setSize(nx, ny+1);

    ui->spinBox_octave->setMinimum(-ny+1);
    ui->spinBox_octave->setMaximum(-1);
    ui->spinBox_octave->setValue(-ny+1);
    PlotWV();
    PlotEnergy();

    //set lines;
    QVector<double> xline_y, xline_x1, xline_x2;
    xline_y.push_back(-ny);
    xline_y.push_back(0);
    xline_x1.push_back(x1);
    xline_x1.push_back(x1);
    xline_x2.push_back(x2);
    xline_x2.push_back(x2);
    plot_W->graph(0)->addData(xline_x1, xline_y);
    plot_W->graph(1)->addData(xline_x2, xline_y);
    plot_W->replot();


    int zMax = qCeil(wave->GetWAbsMax())*100;
    ui->verticalSlider_Threshold->setSingleStep(1);
    ui->verticalSlider_Threshold->setRange(0, zMax);
    ui->verticalSlider_Threshold->setValue(0);
    ui->lineEdit_Threshold->setText("0.0");

    ui->verticalSlider_Octave->setSingleStep(1);
    ui->verticalSlider_Octave->setRange(-ny+1, 0);
    ui->verticalSlider_Octave->setValue(0);
    ui->lineEdit_Octave->setText("0");

    enableVerticalBar = 1;

    plot->graph(1)->clearData();
    plot->graph(1)->addData(x, y);
    //wave->Recontruct();
    //plot->graph(1)->addData(x, wave->GetV0oct(0));
    plot->rescaleAxes();
    plot->replot();

}

void WaveletPlot::PlotWV()
{
    QVector<double> * w = wave->GetW();
    QVector<double> * v = wave->GetV();
    QVector<int> * wk = wave->GetWk();
    QVector<int> * vk = wave->GetVk();

    int size = wave->GetSize();
    int ny = wave->GetMaxScale();

    //plot
    QVector<double> temp_W, temp_V;
    for( int s = 1; s < ny ; s++){
        //Filling space
        temp_W.clear();
        temp_V.clear();

        //qDebug() << s << "," << w[s].size();

        for( int k = 0; k < w[s].size(); k++){
            if( wk[s][k] < 0 ) continue;

            for( int d = 0; d < qPow(2,s); d++){
                temp_W.push_back( w[s][k] );
            }
        }

        //if( s == 1) qDebug() << temp_W.size() << "--" << temp_W;

        for( int k = 0; k < v[s].size(); k++){
            if( vk[s][k] < 0 ) continue;
            for( int d = 0; d < qPow(2,s); d++){
                temp_V.push_back( v[s][k] );
            }
        }

        for( int k = 0; k < size; k++){
            colorMap_W->data()->setCell(k,ny-s, qAbs(temp_W[k]));
            colorMap_V->data()->setCell(k,ny-s, qAbs(temp_V[k]));
        }
    }

    colorMap_W->rescaleDataRange();
    colorMap_W->rescaleAxes();
    colorMap_W->setDataRange(QCPRange(0,wave->GetWAbsMax()*1.2));
    plot_W->replot();

    colorMap_V->rescaleDataRange();
    colorMap_V->rescaleAxes();
    colorMap_V->setDataRange(QCPRange(0,wave->GetVAbsMax()*1.2));
    plot_V->replot();

    w = NULL;
    delete [] w;
    v = NULL;
    delete [] v;
}

void WaveletPlot::PlotReconstructedData(bool Original)
{
    QVector<double> y;
    if( Original){
         y = wave->GetV0octave(0);
    }else{
        y = wave->GetVoctave(0);
    }
    QVector<double> x = file->GetDataSetX();

    //qDebug() << y;

    plot->graph(1)->clearData();
    plot->graph(1)->addData(x, y);
    //wave->Recontruct();
    //plot->graph(1)->addData(x, wave->GetV0oct(0));
    plot->rescaleAxes();
    plot->replot();
}

void WaveletPlot::PlotWVoctave(int octave)
{
    if( octave == 0) {
        plot_Woct->graph(0)->clearData();
        plot_Voct->graph(0)->clearData();
        plot_Woct->replot();
        plot_Voct->replot();
        return;
    }

    octave = qAbs(octave);
    //qDebug() << "Plot WV oct " << s;
    if( wave == NULL ) return;
    QVector<double> w = wave->GetWoctave(octave);
    QVector<double> v = wave->GetVoctave(octave);
    QVector<double> x = file->GetDataSetX();
    QVector<int> wk = wave->GetWkoctave(octave);
    QVector<int> vk = wave->GetVkoctave(octave);

    QVector<double> vy, wy;

    int n = x.size();

    int count = 0;
    for( int k = 0; k < w.size(); k++){
        if( wk[k] < 0 ) continue;

        for( int d = 0; d < qPow(2,octave); d++){
            wy.push_back( w[k] );
            count ++;
        }

        if( count >= n) break;
    }

    count = 0;
    for( int k = 0; k < v.size(); k++){
        if( vk[k] < 0 ) continue;

        for( int d = 0; d < qPow(2,octave); d++){
            vy.push_back( v[k] );
            count ++;
        }

        if( count >= n) break;
    }

    plot_Woct->graph(0)->clearData();
    plot_Woct->graph(0)->addData(x, wy);
    plot_Woct->rescaleAxes();
    plot_Woct->replot();

    plot_Voct->graph(0)->clearData();
    plot_Voct->graph(0)->addData(x, vy);
    plot_Voct->rescaleAxes();
    plot_Voct->replot();

}

void WaveletPlot::PlotEnergy()
{
    if( wave == NULL) return;

    plot_energy->graph(0)->clearData();
    plot_energy->graph(1)->clearData();

    QVector<double> energy0 = wave->GetEnergy0();
    double totalEnergy0 = wave->GetTotalEnergy0();

    QVector<double> energy = wave->GetEnergy();
    double totalEnergy = wave->GetTotalEnergy();

    QVector<double> octave;
    for( int i = 0;  i < energy0.size(); i++){
        octave.push_back(-i-1);
        energy0[i] = energy0[i]/totalEnergy0*100.;
        energy[i] = energy[i]/totalEnergy*100.;
    }

    ui->lineEdit_totalEnergy0->setText("" + QString::number(totalEnergy0));
    ui->lineEdit_totalEnergy->setText("" + QString::number(totalEnergy));

    plot_energy->graph(0)->addData(octave, energy0);
    plot_energy->graph(1)->addData(octave, energy);
    plot_energy->xAxis->setRange(-5, 50);
    //plot_energy->rescaleAxes();
    plot_energy->yAxis->setRange(-wave->GetMaxScale(), 0);
    plot_energy->replot();
}

void WaveletPlot::plotXAxisChanged(QCPRange range)
{
    if( file == NULL ) return;
    double xMin = file->GetXMin();
    double xMax = file->GetXMax();

    //regulate the xAxis
    if( range.upper > xMax){
        plot->xAxis->setRangeUpper(xMax);
        range = plot->xAxis->range();
    }

    if( range.lower < xMin){
        plot->xAxis->setRangeLower(xMin);
        range = plot->xAxis->range();
    }
}


void WaveletPlot::on_verticalSlider_Threshold_valueChanged(int value)
{
    if( !enableControl ) return;
    if( enableVerticalBar){
        wave->RestoreData();

        ui->lineEdit_Threshold->setText(QString::number(value/100.));
        int sLimit = ui->verticalSlider_Octave->value();
        int octave = qAbs(ui->verticalSlider_Octave->value());

        if( !(sLimit == 0 || value == 0)){
            //qDebug() << "cal." << sLimit << "," << value/100.;
            if( ui->comboBox_Thresholding->currentIndex() == 0){
                wave->HardThresholding(value/100., sLimit);
            }
            if( ui->comboBox_Thresholding->currentIndex() == 1){
                wave->HardThresholding(value/100., sLimit, 1);
            }
            if( ui->comboBox_Thresholding->currentIndex() == 2){
                wave->SoftThresholding(value/100., sLimit);
            }
            //SendMsg(wave->GetMsg());
            wave->Reconstruct(octave+1);
            //SendMsg(wave->GetMsg());
            wave->CalculateEnergy(0);

            PlotWV();
            PlotEnergy();
            PlotWVoctave(ui->verticalSlider_Octave->value());
            PlotReconstructedData();

        }

        if( value == 0 || sLimit == 0){
            //wave->Reconstruct();
            wave->CalculateEnergy();
            PlotWV();
            PlotEnergy();
            PlotReconstructedData(1);
        }

        //ui->pushButton_Clean->setEnabled(true);
    }
}

void WaveletPlot::on_verticalSlider_Octave_valueChanged(int value)
{
    if( !enableControl ) return;
    ui->lineEdit_Octave->setText(QString::number(value));
    if( value == 0) {
        ui->pushButton_Clean->setEnabled(false);
        ui->verticalSlider_Threshold->setEnabled(false);
        ui->lineEdit_Threshold->setEnabled(false);
    }else{
        ui->pushButton_Clean->setEnabled(true);
        ui->verticalSlider_Threshold->setEnabled(true);
        ui->lineEdit_Threshold->setEnabled(true);
    }
    int val = ui->verticalSlider_Threshold->value();
    on_verticalSlider_Threshold_valueChanged(val);

    PlotWVoctave(value);
}

void WaveletPlot::on_pushButton_ApplyThreshold_clicked()
{
    int changed = file->ChangeZData(yIndex, wave->GetVoctave(0));
    Replot();

    if( changed ){
        QString msg;
        msg.sprintf("Apply Hard Thresholding, level<%4s && scale>=%2s, Method: %s",
                    ui->lineEdit_Threshold->text().toStdString().c_str(),
                    ui->lineEdit_Octave->text().toStdString().c_str(),
                    ui->comboBox_Thresholding->currentText().toStdString().c_str());
        SendMsg(msg);
    }
}

void WaveletPlot::on_pushButton_ApplyToAll_clicked()
{
    int waveletID = ui->comboBox_Wavelet->currentIndex();
    int waveletPar = 0;
    if( waveletID != 0 ) waveletPar = ui->spinBox_WaveletIndex->value();

    int octave = ui->verticalSlider_Octave->value();
    double threshold = ui->verticalSlider_Threshold->value();

    int yIndexStart = 0;
    if( file->HasBackGround() ) yIndexStart = 1;

    int thresholdType = ui->comboBox_Thresholding->currentIndex();

    enableControl = false;
    QString str;
    str.sprintf("Applying WT to all Data (%s-%d), octave >= %d, threshold < %4.2f, method: %s",
                ui->comboBox_Wavelet->currentText().toStdString().c_str(),
                waveletPar, octave, threshold,
                ui->comboBox_Thresholding->currentText().toStdString().c_str() );
    SendMsg(str);

    int n = file->GetYDataSize();
    QProgressDialog progress("", "Abort", yIndexStart, n-1);
    str.sprintf("WT to all Data ...");
    progress.setWindowTitle(str);
    progress.setWindowModality(Qt::WindowModal);

    for(int yIndex = yIndexStart ; yIndex < file->GetYDataSize(); yIndex++){
    //for(int yIndex = yIndexStart ; yIndex < 10; yIndex++){
        str.sprintf("  Applying WT to all Data (%s-%d),       %d/%d.    ",
                    ui->comboBox_Wavelet->currentText().toStdString().c_str(),
                    waveletPar, yIndex, n-1);
        progress.setLabelText(str);
        progress.setValue(yIndex);
        if(progress.wasCanceled()) {
            SendMsg("=========== Aborted.");
            break;
        }

        wave->ClearData();
        wave->SetData(file->GetDataSetX(), file->GetDataSetZ(yIndex));
        wave->setWaveletPar(waveletID, waveletPar);
        wave->Decompose();

        if( thresholdType == 1){
            wave->HardThresholding(threshold/100., octave, 1);
        }else if( thresholdType == 2){
            wave->SoftThresholding(threshold/100., octave);
        }else{
            wave->HardThresholding(threshold/100., octave);
        }

        wave->Reconstruct(qAbs(octave)+1);
        file->ChangeZData(yIndex, wave->GetVoctave(0));
        //qDebug() << "WT done for yIndex = " << yIndex;
    }
    SendMsg("=========== Finished.");

    Replot();
    enableControl = true;
    this->hide();
}

void WaveletPlot::ShowMousePosition(QMouseEvent *mouse)
{
    QPoint pt = mouse->pos();
    double x = plot_W->xAxis->pixelToCoord(pt.rx());
    double y = plot_W->yAxis->pixelToCoord(pt.ry());

    int xIndex = file->GetXIndex(x);
    int s =  qFloor( wave->GetMaxScale()+ y + 0.5) ;
    double z = colorMap_W->data()->cell(xIndex, s );

    QString msg;
    msg.sprintf("(k, s, w) = (%7.4f, %7.4f, %7.4f), x-index = %4d", x, y, z, xIndex);
    statusBar()->showMessage(msg);
}

void WaveletPlot::SetLineByMouseClick(QMouseEvent *mouse)
{
    QPoint pt = mouse->pos();
    double x = plot_W->xAxis->pixelToCoord(pt.rx());

    QVector<double> xline_y, xline_x1, xline_x2;
    xline_y.push_back(-wave->GetMaxScale());
    xline_y.push_back(0);

    if( mouse->button() == Qt::LeftButton){
        x1 = x;
        ui->lineEdit_x1->setText(QString::number(x1));
    }else if(mouse->button() == Qt::RightButton){
        x2 = x;
        ui->lineEdit_x2->setText(QString::number(x2));
    }

    xline_x1.push_back(x1);
    xline_x1.push_back(x1);
    plot_W->graph(0)->clearData();
    plot_W->graph(0)->addData(xline_x1, xline_y);
    xline_x2.push_back(x2);
    xline_x2.push_back(x2);
    plot_W->graph(1)->clearData();
    plot_W->graph(1)->addData(xline_x2, xline_y);

    plot_W->replot();
}

void WaveletPlot::on_pushButton_Clean_clicked()
{
    int sLimit = ui->verticalSlider_Octave->value();
    if( sLimit == 0) return;
    if( x1 < x2) {
        wave->CleanOutsider(x1, x2, sLimit);
    }else{
        wave->CleanOutsider(x2, x1, sLimit);
    }

    int octave = qAbs(ui->verticalSlider_Octave->value());
    wave->Reconstruct(octave+1);

    QVector<double> v0 = wave->GetVoctave(0);

    PlotWV();
    PlotReconstructedData();

}

void WaveletPlot::on_lineEdit_Octave_editingFinished()
{
    int value = ui->lineEdit_Octave->text().toInt();
    value = - qAbs(value);
    ui->lineEdit_Octave->setText(QString::number(value));
    ui->verticalSlider_Octave->setValue(value);
}

void WaveletPlot::on_lineEdit_Threshold_editingFinished()
{
    double value = ui->lineEdit_Threshold->text().toDouble();
    value = qAbs(value);
    ui->verticalSlider_Threshold->setValue(value*100);
}

void WaveletPlot::on_comboBox_Wavelet_currentIndexChanged(int index)
{
    if( !enableControl ) return;
    if( wave == NULL) return;
    qDebug() << "combox wavelet : " << index;

    wave->setWaveletPar(index, 2);
    SendMsg(wave->GetMsg());
    wave->Decompose();
    SendMsg(wave->GetMsg());
    wave->CalculateEnergy();
    wave->Reconstruct(1);

    enableSpinBoxWaveletIndex = false;
    int numberOfKind = wave->GetWaveletNumberOfKind();
    if(index == 1 || index == 2){
        ui->spinBox_WaveletIndex->setMinimum(2);
        ui->spinBox_WaveletIndex->setMaximum(numberOfKind);
        ui->spinBox_WaveletIndex->setValue(2);
        ui->spinBox_WaveletIndex->setEnabled(true);
    }else if(index == 3){
        ui->spinBox_WaveletIndex->setMinimum(1);
        ui->spinBox_WaveletIndex->setMaximum(numberOfKind);
        ui->spinBox_WaveletIndex->setValue(1);
        ui->spinBox_WaveletIndex->setEnabled(true);
    }else{
        ui->spinBox_WaveletIndex->setEnabled(false);
    }
    enableSpinBoxWaveletIndex = true;

    enableVerticalBar = false;
    ui->verticalSlider_Threshold->setValue(0);
    ui->lineEdit_Threshold->setText("0.0");
    enableVerticalBar = true;

    PlotWV();
    PlotEnergy();
    PlotWVoctave(ui->verticalSlider_Octave->value());
    PlotReconstructedData();
}

void WaveletPlot::on_spinBox_WaveletIndex_valueChanged(int arg1)
{
    if( !enableControl ) return;
    if( enableSpinBoxWaveletIndex == false ) return;
    if( wave == NULL ) return;
    int waveletID = ui->comboBox_Wavelet->currentIndex();
    wave->setWaveletPar(waveletID, arg1);
    SendMsg(wave->GetMsg());
    wave->Decompose();
    SendMsg(wave->GetMsg());
    wave->CalculateEnergy();
    wave->Reconstruct(1);

    enableVerticalBar = false;
    ui->verticalSlider_Threshold->setValue(0);
    ui->lineEdit_Threshold->setText("0.0");
    enableVerticalBar = true;

    PlotWV();
    PlotEnergy();
    PlotWVoctave(ui->verticalSlider_Octave->value());
    PlotReconstructedData();
}

void WaveletPlot::on_comboBox_Thresholding_currentIndexChanged(int index)
{
    if( !enableControl ) return;
    int value = ui->verticalSlider_Threshold->value();
    on_verticalSlider_Threshold_valueChanged(value);
}

void WaveletPlot::on_lineEdit_x1_editingFinished()
{
    double x = ui->lineEdit_x1->text().toDouble();
    QVector<double> xline_y, xline_x1;
    xline_y.push_back(-wave->GetMaxScale());
    xline_y.push_back(0);

    x1 = x;

    xline_x1.push_back(x1);
    xline_x1.push_back(x1);
    plot_W->graph(0)->clearData();
    plot_W->graph(0)->addData(xline_x1, xline_y);

    plot_W->replot();
}

void WaveletPlot::on_lineEdit_x2_editingFinished()
{
    double x = ui->lineEdit_x2->text().toDouble();
    QVector<double> xline_y, xline_x2;
    xline_y.push_back(-wave->GetMaxScale());
    xline_y.push_back(0);

    x2 = x;

    xline_x2.push_back(x2);
    xline_x2.push_back(x2);
    plot_W->graph(1)->clearData();
    plot_W->graph(1)->addData(xline_x2, xline_y);

    plot_W->replot();
}

void WaveletPlot::on_checkBox_normalized_clicked(bool checked)
{
    if(checked ) {
        wave->setNormFactor( sqrt(2.));
        SendMsg("Set normalized wavelet Transform.");
    }else{
        wave->setNormFactor(1.);
        SendMsg("Set uneven wavelet Transform.");
    }
    wave->Decompose();
    wave->CalculateEnergy();
    wave->Reconstruct(1);

    enableVerticalBar = false;
    int zMax = qCeil(wave->GetWAbsMax())*100;
    ui->verticalSlider_Threshold->setSingleStep(1);
    ui->verticalSlider_Threshold->setRange(0, zMax);
    ui->verticalSlider_Threshold->setValue(0);
    enableVerticalBar = true;

    PlotWV();
    PlotEnergy();
    PlotWVoctave(ui->verticalSlider_Octave->value());
    PlotReconstructedData();
}

void WaveletPlot::on_pushButton_PlotID_clicked()
{
    if( wave == NULL ) return;

    int bottomMargin = 40;
    int rightMargin = 10;
    int leftMargin = 30;
    int plotLenght = 900;

    QDialog * listPlotDialog = new QDialog(this);

    listPlotDialog->resize(500, plotLenght);
    listPlotDialog->setWindowTitle("ID plot");

    //QWidget * plotID = new QWidget(listPlotDialog);
    QCustomPlot * plotID = new QCustomPlot();
    plotID->plotLayout()->clear();
    //const int numPlot = wave->GetMaxScale() ;
    const int numPlot = qAbs(ui->spinBox_octave->value()) + 1 ;
    QCPAxisRect * * axisRect = new QCPAxisRect* [numPlot];
    for( int i = 0; i < numPlot; i++){
        axisRect[i] = new QCPAxisRect(plotID);
        plotID->plotLayout()->addElement(i,0, axisRect[i]);
    }


    plotID->plotLayout()->setAutoMargins(QCP::msNone);
    plotID->plotLayout()->setMargins(QMargins(0,0,rightMargin,0));
    plotID->plotLayout()->setRowSpacing(0);
    QList<double> factor;
    for( int i = 0; i < numPlot-1; i++){
        factor.push_back(1.0);
    }
    factor.push_back(1.0 + bottomMargin*1.0/(plotLenght-bottomMargin)*numPlot);
    const QList<double> sf = factor;
    plotID->plotLayout()->setRowStretchFactors( sf );

    QCPPlotTitle * plotTitle = new QCPPlotTitle(plotID, "title");
    plotTitle->setText(ui->comboBox_Wavelet->currentText() + "-" + QString::number(ui->spinBox_WaveletIndex->value()));
    plotTitle->setFont(QFont("sans", 16, QFont::Bold));
    plotID->plotLayout()->insertRow(0);
    plotID->plotLayout()->addElement(0,0, plotTitle);

    //fill data
    QVector<double> x = file->GetDataSetX();
    for( int i = 0; i < numPlot; i++){

        plotID->addGraph(axisRect[i]->axis(QCPAxis::atBottom), axisRect[i]->axis(QCPAxis::atLeft) );
        //plotID->graph(i)->setPen(QPen(Qt::blue));
        axisRect[i]->setAutoMargins(QCP::msNone);
        axisRect[i]->setMargins(QMargins(leftMargin,0,0,0));
        axisRect[i]->axis(QCPAxis::atTop)->setVisible(true);
        axisRect[i]->axis(QCPAxis::atTop)->setTickLabels(false);
        axisRect[i]->axis(QCPAxis::atRight)->setVisible(true);
        axisRect[i]->axis(QCPAxis::atRight)->setTickLabels(false);
        if( i == numPlot -1 ) axisRect[i]->setMargins(QMargins(leftMargin,0,0,bottomMargin));
        if( i < numPlot -1 ) {
            axisRect[i]->axis(QCPAxis::atBottom)->setTickLabels(false);
        }else{
            axisRect[i]->axis(QCPAxis::atBottom)->setLabel("time [us]");
        }

        QVector<double> w ;
        QVector<int> wk;

        int octave;
        if( i < numPlot - 1) {
            octave = i+1;
            w = wave->GetWoctave(octave);
            wk = wave->GetWkoctave(octave);
        }else{
            octave = i;
            w = wave->GetVoctave(octave);
            wk = wave->GetWkoctave(octave);
        }

        QVector<double> wy;
        int n = x.size();

        int count = 0;
        for( int k = 0; k < w.size(); k++){
            if( wk[k] < 0 ) continue;

            for( int d = 0; d < qPow(2,octave); d++){
                wy.push_back( w[k] );
                count ++;
            }

            if( count >= n) break;
        }

        plotID->graph(i)->clearData();
        plotID->graph(i)->addData(x, wy);

    }
    plotID->rescaleAxes();

    double lowest = 0, uppest = 0;
    for( int i = 0; i < numPlot ; i++){
        double l = axisRect[i]->axis(QCPAxis::atLeft)->range().lower;
        double u = axisRect[i]->axis(QCPAxis::atLeft)->range().upper;

        axisRect[i]->axis(QCPAxis::atTop)->setRange(axisRect[i]->axis(QCPAxis::atBottom)->range());

        if(!ui->checkBox_sameYrange->isChecked()) {
            axisRect[i]->axis(QCPAxis::atLeft)->setRange(l *1.2, u*1.2);
            axisRect[i]->axis(QCPAxis::atRight)->setRange(l*1.2, u*1.2);
        }
        if( l < lowest) lowest = l;
        if( u > uppest) uppest = u;

    }

    if( ui->checkBox_sameYrange->isChecked() ){
        for( int i = 0; i < numPlot ; i++){
            axisRect[i]->axis(QCPAxis::atLeft)->setRange(lowest *1.2, uppest*1.2);
            axisRect[i]->axis(QCPAxis::atRight)->setRange(lowest *1.2, uppest*1.2);
        }
    }

    QCPItemText ** textLabel = new QCPItemText* [numPlot];
    for(int i = 0; i < numPlot; i++){

        textLabel[i] = new QCPItemText(plotID);
        plotID->addItem(textLabel[i]);

        textLabel[i]->setClipToAxisRect(false);
        textLabel[i]->setPositionAlignment(Qt::AlignTop|Qt::AlignRight);
        textLabel[i]->position->setType( QCPItemPosition::ptAxisRectRatio);
        textLabel[i]->position->setAxisRect(axisRect[i]);
        //textLabel[i]->position->setAxes(axisRect[i]->axis(QCPAxis::atBottom), axisRect[i]->axis(QCPAxis::atLeft));
        textLabel[i]->position->setCoords(1.0,0.);
        if( i < numPlot -1 ){
            textLabel[i]->setText("W"+QString::number(i+1));
        }else{
            textLabel[i]->setText("V"+QString::number(i));
        }
        textLabel[i]->setFont(QFont(font().family(), 16)); // make font a bit larger
        textLabel[i]->setPen(QPen(Qt::black));
    }

    plotID->replot();

    //QPushButton * closeButton = new QPushButton("Close");
    //connect( closeButton, SIGNAL(clicked(bool)), listPlotDialog, SLOT(close())  );

    QVBoxLayout * mainLayout = new QVBoxLayout;
    mainLayout->addWidget(plotID);
    //mainLayout->addWidget(closeButton);
    listPlotDialog->setLayout(mainLayout);

    listPlotDialog->show();

}
