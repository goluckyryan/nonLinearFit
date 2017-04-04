#include "databasewindow.h"
#include "ui_databasewindow.h"

DataBaseWindow::DataBaseWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DataBaseWindow)
{
    ui->setupUi(this);
    this->resize(1300, 600);

    maxImageSize = 250;

    db = QSqlDatabase::addDatabase("QSQLITE");
    if( QFile::exists(DB_PATH) ){
        msg = "database exist : " + DB_PATH;
        isDBOpened = true;
    }else{
        msg = "No database : " + DB_PATH;
        isDBOpened = false;
        //TODO created a db
        return;
    }

    db.setDatabaseName(DB_PATH);
    db.open();
    if( !db.isOpen()){
        msg = "Database open Error : " + DB_PATH;
        isDBOpened = false;
        return;
    }

    QStringList tableList = db.tables();
    qDebug() << tableList;

    QStringList dataColName = GetTableColName("Data");
    for(int i = 0; i < dataColName.size() ; i++){
        if( dataColName[i] == "PATH"){
            dataPathCol = i;
            qDebug("data Path Col : %d", i);
            break;
        }
    }

    //===================== set up the sample-table
    sample = new QSqlRelationalTableModel(ui->sampleView);
    SetupSampleTableView();

    connect(ui->sampleView->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this,
            SLOT(showSampleSpectrum(QModelIndex))
            );

    //====================== set up the data-table    
    data = new QSqlRelationalTableModel(ui->dataView);
    SetupDataTableView();

    //====================== Other things
    editorChemical = NULL;
    editorHost = NULL;
    editorSolvent = NULL;
    editorLaser = NULL;

    //ShowTable("Chemical");
    //ShowTable("Sample");
    //ShowTable("Data");

    updateChemicalCombox("Chemical");

    ui->checkBox_sortData->setChecked(true);
    ui->dataView->sortByColumn(2, Qt::DescendingOrder);

}

DataBaseWindow::~DataBaseWindow()
{
    delete ui;
}

QStringList DataBaseWindow::GetTableColEntries(QString tableName, int col)
{
    QSqlQuery query;
    query.exec("SELECT *FROM " + tableName);

    QStringList entries;

    while(query.next()){
        entries << query.value(col).toString();
    }

    return entries;
}

int DataBaseWindow::GetTableColNumber(QString tableName)
{
    QSqlQuery query;
    query.exec("SELECT *FROM " + tableName);
    return query.record().count();
}

QStringList DataBaseWindow::GetTableColName(QString tableName)
{
    QSqlQuery query;
    query.exec("SELECT *FROM " + tableName);

    int col = query.record().count();
    QStringList fieldNameList;
    for(int j = 0; j< col; j++){
        fieldNameList << query.record().fieldName(j);
    }
    return fieldNameList;
}

void DataBaseWindow::ShowTable(QString tableName)
{
    QSqlQuery query;
    query.exec("SELECT *FROM " + tableName);

    QString msg, temp;

    int col = query.record().count();
    query.last();
    int row = query.at() + 1;

    msg.sprintf("Table Name : %s, size = %d x %d", tableName.toStdString().c_str(), col, row);
    qDebug() << msg;
    msg.clear();

    QStringList fieldNameList = GetTableColName(tableName);
    qDebug() << fieldNameList;

    query.first();
    query.previous();
    while(query.next()){
        for(int j = 0; j< col; j++){
            fieldNameList << query.record().fieldName(j);
            temp = query.value(j).toString();
            msg += temp;
            msg += " | ";
        }
        qDebug() << msg;
        msg.clear();
    }
}

void DataBaseWindow::SetupSampleTableView()
{
    sample->clear();
    sample->setTable("Sample");
    sample->setEditStrategy(QSqlTableModel::OnManualSubmit);
    sample->select();

    //set relation, so that can choose directly on the table
    int chemicalIdx = sample->fieldIndex("Chemical");
    sample->setRelation(chemicalIdx, QSqlRelation("Chemical", "NAME", "NAME"));
    int hostIdx = sample->fieldIndex("Host");
    sample->setRelation(hostIdx, QSqlRelation("Host", "NAME", "NAME"));
    int solventIdx = sample->fieldIndex("Solvent");
    sample->setRelation(solventIdx, QSqlRelation("Solvent", "NAME", "NAME"));

    ui->sampleView->setModel(sample);
    ui->sampleView->resizeColumnsToContents();
    ui->sampleView->setItemDelegate(new QSqlRelationalDelegate(ui->sampleView));
    ui->sampleView->setItemDelegateForColumn(6, new DateFormatDelegate() );
    ui->sampleView->setItemDelegateForColumn(9, new OpenFileDelegate() );
    //ui->sampleView->setColumnHidden(sample->fieldIndex("ID"), true);
    ui->sampleView->setSelectionMode(QAbstractItemView::SingleSelection);
    //for some unknown reasons, the column header names are needed to rename;
    sample->setHeaderData(2, Qt::Horizontal, "Chemical");
    sample->setHeaderData(3, Qt::Horizontal, "Host");
    sample->setHeaderData(4, Qt::Horizontal, "Solvent");

    ui->sampleView->setColumnWidth(1, 100);
    ui->sampleView->setColumnWidth(2, 100);
    ui->sampleView->setColumnWidth(3, 100);
    ui->sampleView->setColumnWidth(4, 100);
    ui->sampleView->setColumnWidth(6, 100);

    //connect(ui->pushButton_sumbitSample, SIGNAL(clicked()), this, SLOT(submit()));
}

void DataBaseWindow::SetupDataTableView()
{
    data->clear();
    data->setTable("Data");
    data->setEditStrategy(QSqlTableModel::OnManualSubmit);
    //data->setJoinMode( QSqlRelationalTableModel::LeftJoin );

    data->setHeaderData(3, Qt::Horizontal, "Laser");
    //data->setHeaderData(3, Qt::Horizontal, "Laser\nWaveLenght [nm]");
    data->setHeaderData(4, Qt::Horizontal, "Repeat.\nRate");
    data->setHeaderData(5, Qt::Horizontal, "Average");
    data->setHeaderData(6, Qt::Horizontal, "Point");
    data->setHeaderData(7, Qt::Horizontal, "Temp.\n[K]");
    data->setHeaderData(8, Qt::Horizontal, "Time\nRange[us]");

    data->select();

    ui->dataView->setModel(data);
    ui->dataView->resizeColumnsToContents();

    int sampleIdx = data->fieldIndex("Sample");
    if(ui->checkBox_showChemical->isChecked() == false){
        data->setRelation(sampleIdx, QSqlRelation("Sample", "NAME", "NAME"));
        data->setHeaderData(1, Qt::Horizontal, "Sample");
    }else{
        data->setRelation(sampleIdx, QSqlRelation("Sample", "NAME", "Chemical"));
        data->setHeaderData(1, Qt::Horizontal, "Chemcial");
    }
    int laserIdx = data->fieldIndex("Laser");
    data->setRelation(laserIdx, QSqlRelation("Laser", "Name", "Name"));

    int dateIdx = data->fieldIndex("Date");
    ui->dataView->setItemDelegate(new QSqlRelationalDelegate(ui->sampleView));
    ui->dataView->setItemDelegateForColumn(dateIdx, new DateFormatDelegate());
    ui->dataView->setItemDelegateForColumn(dataPathCol, new OpenFileDelegate());

    ui->dataView->setColumnWidth(sampleIdx, 100);
    ui->dataView->setColumnWidth(dateIdx, 100);
    ui->dataView->setColumnWidth(laserIdx, 60);

    if( ui->checkBox_sortData->isChecked()) {
        ui->dataView->sortByColumn(2, Qt::DescendingOrder);
    }else{
        ui->dataView->sortByColumn(2, Qt::AscendingOrder);
    }

}

void DataBaseWindow::updateChemicalCombox(QString tableName)
{
    QStringList hostList = GetTableColEntries(tableName, 1);
    ui->comboBox_chemical->clear();
    ui->comboBox_chemical->addItem("All");
    ui->comboBox_chemical->addItems(hostList);
}

void DataBaseWindow::on_comboBox_chemical_currentTextChanged(const QString &arg1)
{
    if(arg1 == "All") {
        sample->setFilter("");
        data->setFilter("");
        ui->lineEdit_ChemicalFormula->setText("-----");
        ui->label_Picture->clear();
        ui->label_Picture->setText("Chemical Structure");
        return;
    }

    QStringList nameList = GetTableColEntries("Chemical", 1);
    QStringList formulaList = GetTableColEntries("Chemical", 2);
    QStringList picNameList = GetTableColEntries("Chemical", 3);


    for(int i = 0; i < nameList.size(); i ++ ){
        if( nameList[i] == arg1) {
            ui->lineEdit_ChemicalFormula->setText(formulaList[i]);
            QString picPath = ChemicalPicture_PATH + picNameList[i];
            QImage image(picPath);
            //ui->label_Picture->setPixmap(QPixmap::fromImage(image));
            int imageSize = image.height();
            if( imageSize > maxImageSize) imageSize = maxImageSize;
            QImage scaledImage = image.scaledToHeight(imageSize);
            ui->label_Picture->setPixmap(QPixmap::fromImage(scaledImage));
            break;
        }
    }

    //select sample
    QString filter = "Chemical='" + arg1 + "'";
    //qDebug() << filter  ;
    sample->setFilter(filter);
    ui->label_spectrum->clear();
    ui->label_spectrum->setText("Sample Picture.");

    //showSampleSpectrum(ui->sampleView->model()->index(0,1));

}

void DataBaseWindow::on_pushButton_editChemical_clicked()
{
    editorChemical = new TableEditor("Chemical");
    disconnect(editorChemical);
    connect(editorChemical, SIGNAL(closed(QString)), this, SLOT(updateChemicalCombox(QString)));
    connect(editorChemical, SIGNAL(closed(QString)), this, SLOT(SetupSampleTableView()));
    editorChemical->show();
}

void DataBaseWindow::on_pushButton_editHost_clicked()
{
    editorHost = new TableEditor("Host");
    disconnect(editorHost);
    connect(editorHost, SIGNAL(closed(QString)), this, SLOT(SetupSampleTableView()));
    editorHost->show();
}

void DataBaseWindow::on_pushButton_editSolvent_clicked()
{
    editorSolvent = new TableEditor("Solvent");
    disconnect(editorSolvent);
    connect(editorSolvent, SIGNAL(closed(QString)), this, SLOT(SetupSampleTableView()));
    editorSolvent->show();
}

void DataBaseWindow::on_pushButton_selectSample_clicked()
{
    QModelIndex current = ui->sampleView->selectionModel()->currentIndex(); // the "current" item
    QString sampleName = sample->index(current.row(), 1).data().toString();

    QString filter = "Sample='" + sampleName + "'";
    qDebug() << filter;
    data->setFilter(filter);
}

void DataBaseWindow::on_pushButton_addSampleEntry_clicked()
{
    int row = sample->rowCount();
    sample->insertRow(row);
    ui->sampleView->scrollToBottom();

    //set default data
    QString sampleName = "Sample-" + QString::number(row+1);
    sample->setData(sample->index(row,1), sampleName);
    QDate date;
    sample->setData(sample->index(row, 6), date.currentDate().toString("yyyy-MM-dd"));
}

void DataBaseWindow::on_pushButton_deleteSampleEntry_clicked()
{
    QModelIndex current = ui->sampleView->selectionModel()->currentIndex(); // the "current" item
    sample->removeRow(current.row());

    QString msg;
    msg.sprintf("Deleted Row #%d.", current.row()+1);
    statusBar()->showMessage(msg);
}

void DataBaseWindow::on_pushButton_revertSample_clicked()
{
    sample->revertAll();
    sample->submitAll();

    statusBar()->showMessage("revert add/delete.");
}

void DataBaseWindow::on_pushButton_sumitSample_clicked()
{
    //sample->database().transaction();
    if (sample->submitAll()) {
    //    sample->database().commit();
        statusBar()->showMessage("Sample Database wriiten.");
    } else {
        sample->database().rollback();
        QMessageBox::warning(this, tr("Cached Table"),
                             tr("The database reported an error: %1").arg(sample->lastError().text()));
    }
}

void DataBaseWindow::on_pushButton_addDataEntry_clicked()
{
    int row = data->rowCount();
    data->insertRow(row);
    ui->dataView->scrollToBottom();

    //set default data
    QDate date;
    data->setData(data->index(row, 2), date.currentDate().toString("yyyy-MM-dd"));
}

void DataBaseWindow::on_pushButton_deleteDataEntry_clicked()
{
    QModelIndex current = ui->dataView->selectionModel()->currentIndex();
    data->removeRow(current.row());

    QString msg;
    msg.sprintf("Deleted Row #%d.", current.row()+1);
    statusBar()->showMessage(msg);
}

void DataBaseWindow::on_pushButton_revertData_clicked()
{
    data->revertAll();
    sample->submitAll();

    statusBar()->showMessage("revert add/delete.");
}

void DataBaseWindow::on_pushButton_submitData_clicked()
{
    //sample->database().transaction();
    if (data->submitAll()){
        statusBar()->showMessage("Data Database wriiten.");
    } else {
        data->database().rollback();
        QMessageBox::warning(this, tr("Cached Table"),
                             tr("The database reported an error: %1").arg(data->lastError().text()));
    }
}

void DataBaseWindow::on_pushButton_open_clicked()
{
    QModelIndex current = ui->dataView->selectionModel()->currentIndex(); // the "current" item
    QString dataName = data->index(current.row(), dataPathCol).data().toString();

    //check is it absolute path or relativePath
    QString dataNameFirst = dataName.split("/")[0];

    if( dataNameFirst == "D:"){
        ReturnFilePath(dataName);
    }else{
        ReturnFilePath(DATA_PATH + dataName);
    }

    this->hide();
}

void DataBaseWindow::on_checkBox_sortData_clicked()
{
    SetupDataTableView();
}

void DataBaseWindow::on_checkBox_showChemical_clicked()
{
    SetupDataTableView();
}

void DataBaseWindow::on_pushButton_editLaser_clicked()
{
    editorLaser = new TableEditor("Laser");
    editorLaser->show();
}

void DataBaseWindow::showSampleSpectrum(const QModelIndex &index)
{
    QString chemicalName = "'" +  sample->index(index.row(), 2).data().toString() + "'";
    QSqlQuery query;
    query.exec("SELECT PicPath From Chemical WHERE Chemical.NAME = " + chemicalName);
    query.last();
    QString chemicalPicPath = ChemicalPicture_PATH + query.value(0).toString();
    qDebug() << chemicalPicPath;
    QImage chemicalPic(chemicalPicPath);
    int imageSize = chemicalPic.height();
    if( imageSize > maxImageSize ) imageSize = maxImageSize;
    QImage scaledChemicalPic = chemicalPic.scaledToHeight(imageSize);
    ui->label_Picture->setPixmap(QPixmap::fromImage(scaledChemicalPic));

    QString spectrumPath = DATA_PATH + sample->index(index.row(), 9).data().toString();
    QImage image(spectrumPath);
    QImage scaledImage = image.scaledToHeight( maxImageSize );
    ui->label_spectrum->setPixmap(QPixmap::fromImage(scaledImage));
}

