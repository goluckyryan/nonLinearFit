#include "databasewindow.h"
#include "ui_databasewindow.h"

DataBaseWindow::DataBaseWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DataBaseWindow)
{
    ui->setupUi(this);

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(DB_PATH);
    qDebug() << db.databaseName();
    qDebug() << "database open? " << db.open();
    QStringList tableList = db.tables();
    qDebug() << tableList;

    //===================== set up the sample-table
    sample = new QSqlRelationalTableModel(ui->sampleView);
    SetupSampleTableView();


    //====================== set up the data-table
    data = new QSqlRelationalTableModel(this);
    data->setTable("Data");
    data->setEditStrategy(QSqlTableModel::OnManualSubmit);
    data->select();
    ui->dataView->setModel(data);
    ui->dataView->resizeColumnsToContents();

    int sampleIdx = data->fieldIndex("Sample");
    data->setRelation(sampleIdx, QSqlRelation("Sample", "NAME", "NAME"));
    ui->dataView->setItemDelegate(new QSqlRelationalDelegate(ui->sampleView));
    ui->dataView->setItemDelegateForColumn(2, new DateFormatDelegate());
    ui->dataView->setItemDelegateForColumn(5, new OpenFileDelegate());
    ui->dataView->horizontalHeader()->model()->setHeaderData(1, Qt::Horizontal, "Sample");

    ui->dataView->setColumnWidth(1, 100);
    ui->dataView->setColumnWidth(2, 100);

    //====================== Other things
    editorChemical = NULL;
    editorHost = NULL;
    editorSolvent = NULL;

    //ShowTable("Chemical");
    //ShowTable("Sample");
    ShowTable("Data");

    updateChemicalCombox("Chemical");
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

    QStringList fieldNameList;
    for(int j = 0; j< col; j++){
        fieldNameList << query.record().fieldName(j);
    }

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
    //ui->sampleView->setColumnHidden(sample->fieldIndex("ID"), true);
    ui->sampleView->setSelectionMode(QAbstractItemView::SingleSelection);
    //for some unknown reasons, the column header names are needed to rename;
    ui->sampleView->horizontalHeader()->model()->setHeaderData(2, Qt::Horizontal, "Chemical");
    ui->sampleView->horizontalHeader()->model()->setHeaderData(3, Qt::Horizontal, "Host");
    ui->sampleView->horizontalHeader()->model()->setHeaderData(4, Qt::Horizontal, "Solvent");

    ui->sampleView->setColumnWidth(1, 100);
    ui->sampleView->setColumnWidth(2, 100);
    ui->sampleView->setColumnWidth(3, 100);
    ui->sampleView->setColumnWidth(4, 100);
    ui->sampleView->setColumnWidth(6, 100);

    //connect(ui->pushButton_sumbitSample, SIGNAL(clicked()), this, SLOT(submit()));
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
            //QImage scaledImage = image.scaledToHeight(100);
            ui->label_Picture->setPixmap(QPixmap::fromImage(image));
            break;
        }
    }

    //select sample
    QString filter = "Chemical='" + arg1 + "'";
    qDebug() << filter  ;
    sample->setFilter(filter);
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
    data->setData(sample->index(row, 2), date.currentDate().toString("yyyy-MM-dd"));
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
    QString dataName = data->index(current.row(), 5).data().toString();

    //check is it absolute path or relativePath
    QString dataNameFirst = dataName.split("/")[0];

    if( dataNameFirst == "D:"){
        ReturnFilePath(dataName);
    }else{
        ReturnFilePath(DATA_PATH + dataName);
    }

    this->hide();
}