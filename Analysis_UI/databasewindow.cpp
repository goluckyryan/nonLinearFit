#include "databasewindow.h"
#include "ui_databasewindow.h"

DataBaseWindow::DataBaseWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DataBaseWindow)
{
    ui->setupUi(this);
    this->resize(1300, 1000);

    maxImageSize = 250;

    if( QFile::exists(DB_PATH) ){
        msg = "database exist : " + DB_PATH;
        isDBOpened = true;
    }else{
        msg = "No database : " + DB_PATH;
        isDBOpened = false;
        //TODO created a db
        return;
    }

    db = QSqlDatabase::addDatabase("QSQLITE");
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
            SLOT(showSamplePicture(QModelIndex))
            );

    //====================== set up the data-table    
    data = new QSqlRelationalTableModel(ui->dataView);
    SetupDataTableView();

    connect(ui->dataView->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this,
            SLOT(showDataPicture(QModelIndex))
            );

    //====================== Other things
    //ShowTable("Chemical");
    //ShowTable("Sample");
    //ShowTable("Data");

    updateChemicalCombox("Chemical");

    ui->checkBox_sortData->setChecked(true);
    int dateIdx = data->fieldIndex("Date");
    ui->dataView->sortByColumn(dateIdx, Qt::DescendingOrder);

    enableChemicalFilter = true;

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
    //sample->setEditStrategy(QSqlTableModel::OnManualSubmit);

    int nameIdx = sample->fieldIndex("NAME");
    int chemicalIdx = sample->fieldIndex("ChemicalID");
    int solventIdx = sample->fieldIndex("SolventID");
    int dateIdx = sample->fieldIndex("Date");
    //int specPathIdx = sample->fieldIndex("SpectrumPath");
    //int picPathIdx = sample->fieldIndex("PicPath");

    //set relation, so that can choose directly on the table
    sample->setRelation(chemicalIdx, QSqlRelation("Chemical", "ID", "NAME"));
    sample->setRelation(solventIdx, QSqlRelation("Solvent", "ID", "NAME"));

    //for some unknown reasons, the column header names are needed to rename;
    sample->setHeaderData(chemicalIdx, Qt::Horizontal, "Pol. Agent");
    sample->setHeaderData(solventIdx, Qt::Horizontal, "Solvent");
    sample->select();

    ui->sampleView->setModel(sample);
    ui->sampleView->resizeColumnsToContents();
    ui->sampleView->setEditTriggers( QAbstractItemView::NoEditTriggers );
    ui->sampleView->setSelectionBehavior( QAbstractItemView::SelectRows );
    ui->sampleView->setSelectionMode(QAbstractItemView::SingleSelection);

    //ui->sampleView->setItemDelegate(new QSqlRelationalDelegate(ui->sampleView));
    //ui->sampleView->setItemDelegateForColumn(dateIdx, new DateFormatDelegate() );
    //ui->sampleView->setItemDelegateForColumn(picPathIdx, new OpenFileDelegate() );
    //ui->sampleView->setItemDelegateForColumn(specPathIdx, new OpenFileDelegate() );
    //ui->sampleView->setColumnHidden(sample->fieldIndex("ID"), true);

    ui->sampleView->setColumnWidth(nameIdx, 150);
    ui->sampleView->setColumnWidth(chemicalIdx, 100);
    ui->sampleView->setColumnWidth(solventIdx, 100);
    ui->sampleView->setColumnWidth(dateIdx, 100);

    //connect(ui->pushButton_sumbitSample, SIGNAL(clicked()), this, SLOT(submit()));
}

void DataBaseWindow::SetupDataTableView()
{
    data->clear();
    data->setTable("Data");
    //data->setEditStrategy(QSqlTableModel::OnManualSubmit);

    int sampleIdx = data->fieldIndex("SampleID");
    int laserIdx = data->fieldIndex("LaserID");
    int dateIdx = data->fieldIndex("Date");
    int repeatIdx = data->fieldIndex("repetition");
    int acummIdx = data->fieldIndex("Average");
    int pointIdx = data->fieldIndex("DataPoint");
    int tempIdx = data->fieldIndex("Temperature");
    int timeRangeIdx = data->fieldIndex("TimeRange");

    data->setHeaderData(laserIdx, Qt::Horizontal, "Laser");
    data->setHeaderData(repeatIdx, Qt::Horizontal, "Trig.\nRate[Hz]");
    data->setHeaderData(acummIdx, Qt::Horizontal, "Acumm.");
    data->setHeaderData(pointIdx, Qt::Horizontal, "Data\nPoint");
    data->setHeaderData(tempIdx, Qt::Horizontal, "Temp.\n[K]");
    data->setHeaderData(timeRangeIdx, Qt::Horizontal, "Time\nRange[us]");
    data->setHeaderData(sampleIdx, Qt::Horizontal, "Sample");

    data->setRelation(laserIdx, QSqlRelation("Laser", "ID", "Name"));
    data->setRelation(sampleIdx, QSqlRelation("Sample", "ID", "NAME"));
    //if(ui->checkBox_showChemical->isChecked() == false){
    //}else{
        //data->setRelation(sampleIdx, QSqlRelation("Sample", "ID", "ChemicalID"));
        //data->setHeaderData(sampleIdx, Qt::Horizontal, "Pol. Agent");
    //}

    data->select();

    ui->dataView->setModel(data);
    ui->dataView->resizeColumnsToContents();
    ui->dataView->setEditTriggers( QAbstractItemView::NoEditTriggers );
    ui->dataView->setSelectionBehavior( QAbstractItemView::SelectRows );

    //ui->dataView->setItemDelegate(new QSqlRelationalDelegate(ui->sampleView));
    //ui->dataView->setItemDelegateForColumn(dateIdx, new DateFormatDelegate());
    //ui->dataView->setItemDelegateForColumn(dataPathCol, new OpenFileDelegate());

    ui->dataView->setColumnWidth(sampleIdx, 150);
    ui->dataView->setColumnWidth(dateIdx, 100);
    ui->dataView->setColumnWidth(laserIdx, 120);

    if( ui->checkBox_sortData->isChecked()) {
        ui->dataView->sortByColumn(dateIdx, Qt::DescendingOrder);
    }else{
        ui->dataView->sortByColumn(dateIdx, Qt::AscendingOrder);
    }

}

void DataBaseWindow::updateChemicalCombox(QString tableName)
{
    QStringList chemicalList = GetTableColEntries(tableName, sample->fieldIndex("NAME"));
    ui->comboBox_chemical->clear();
    ui->comboBox_chemical->addItem("All");
    ui->comboBox_chemical->addItems(chemicalList);
    qDebug() << chemicalList;
}

void DataBaseWindow::on_comboBox_chemical_currentTextChanged(const QString &arg1)
{
    if(arg1 == "All") {
        sample->setFilter("");
        data->setFilter("");
        ui->lineEdit_ChemicalFormula->setText("-----");
        ui->label_ChemPicture->clear();
        ui->label_ChemPicture->setText("Chemical Structure");
        return;
    }

    QSqlTableModel chemical;
    chemical.setTable("Chemical");

    QStringList idList = GetTableColEntries("Chemical", chemical.fieldIndex("ID"));
    QStringList nameList = GetTableColEntries("Chemical", chemical.fieldIndex("NAME"));
    QStringList formulaList = GetTableColEntries("Chemical", chemical.fieldIndex("FORMULA"));
    QStringList picNameList = GetTableColEntries("Chemical", chemical.fieldIndex("PicPath"));
    QStringList commentList = GetTableColEntries("Chemical", chemical.fieldIndex("Comment"));

    chemical.clear();

    int correctID = -1;

    for(int i = 0; i < nameList.size(); i ++ ){
        if( nameList[i] == arg1) {
            QString idStr = idList[i];
            correctID = idStr.toInt();
            ui->lineEdit_ChemicalFormula->setText(formulaList[i]);
            ui->lineEdit_ChemicalComment->setText(commentList[i]);
            if( !enableChemicalFilter ) break;
            QString picPath = CHEMICAL_PIC_PATH + picNameList[i];
            if( QFile::exists(picPath)){
                QImage image(picPath);
                int imageSize = image.height();
                if( imageSize > maxImageSize) imageSize = maxImageSize;
                QImage scaledImage = image.scaledToHeight(imageSize);
                ui->label_ChemPicture->setPixmap(QPixmap::fromImage(scaledImage));
            }else{
                ui->label_ChemPicture->setText("no image found.\nPlease check the file location.");
            }
            break;
        }
    }

    if( !enableChemicalFilter ) return;
    //select sample
    QString filter = "ChemicalID=" + QString::number(correctID) + "";
    qDebug() << filter  ;
    sample->setFilter(filter);
    ui->label_SamplePic->clear();
    ui->label_SamplePic->setText("Sample Picture.");

}

void DataBaseWindow::on_pushButton_selectSample_clicked()
{
    QModelIndex current = ui->sampleView->selectionModel()->currentIndex(); // the "current" item
    QString sampleID = sample->index(current.row(), sample->fieldIndex("ID")).data().toString();

    QString filter = "SampleID='" + sampleID + "'";
    qDebug() << filter;
    data->setFilter(filter);
}

void DataBaseWindow::on_pushButton_unSelectSample_clicked()
{
    data->setFilter("");
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

void DataBaseWindow::showSamplePicture(const QModelIndex &index)
{
    // find the chemical picture
    QSqlTableModel temp;
    temp.setTable("Sample");
    // after QSqlRealtion, the ChemicalID changed to Chemical Name
    QString chemicalName = sample->index(index.row(), temp.fieldIndex("ChemicalID")).data().toString();
    QSqlQuery query;
    query.exec("SELECT PicPath From Chemical WHERE Chemical.Name = '" + chemicalName + "'");
    query.last();
    QString chemicalPicPath = CHEMICAL_PIC_PATH + query.value(0).toString();
    if( !QFile::exists(chemicalPicPath) || chemicalPicPath.right(1) == "/"){
        ui->label_ChemPicture->setText("no image found.\nPlease check the file location.");
    }else{
        QImage chemicalPic(chemicalPicPath);
        int imageSize = chemicalPic.height();
        if( imageSize > maxImageSize ) imageSize = maxImageSize;
        QImage scaledChemicalPic = chemicalPic.scaledToHeight(imageSize);
        ui->label_ChemPicture->setPixmap(QPixmap::fromImage(scaledChemicalPic));
    }

    // set the chemical combox
    enableChemicalFilter = false;
    for(int i = 1; i < ui->comboBox_chemical->count(); i++ ){
        if( ui->comboBox_chemical->itemText(i) == chemicalName ){
            ui->comboBox_chemical->setCurrentIndex(i);
            break;
        }
    }
    enableChemicalFilter = true;

    // set the sample picture
    int pathIdx = sample->fieldIndex("PicPath");
    QString samplePicPath = SAMPLE_PIC_PATH + sample->index(index.row(), pathIdx).data().toString();
    if( !QFile::exists(samplePicPath) || samplePicPath.right(1) == "/"){
        ui->label_SamplePic->setText("no image found.\nPlease check the file location.");
    }else{
        QImage samplePic(samplePicPath);
        QImage scaledSamplePic = samplePic.scaledToHeight( maxImageSize );
        ui->label_SamplePic->setPixmap(QPixmap::fromImage(scaledSamplePic));
    }

    // set the sample spectrum
    pathIdx = sample->fieldIndex("SpectrumPath");
    QString spectrumPicPath = SAMPLE_PIC_PATH + sample->index(index.row(), pathIdx).data().toString();
    if( !QFile::exists(spectrumPicPath) || spectrumPicPath.right(1) == "/"){
        ui->label_SampleSpectrum->setText("no image found.\nPlease check the file location.");
    }else{
        QImage spectrumPic(spectrumPicPath);
        QImage scaledSpectrumPic = spectrumPic.scaledToHeight( maxImageSize );
        ui->label_SampleSpectrum->setPixmap(QPixmap::fromImage(scaledSpectrumPic));
    }
}

void DataBaseWindow::showDataPicture(const QModelIndex &index)
{
    QSqlQuery query;

    // find the sample picture
    int nameIdx = data->fieldIndex("NAME");
    QString sampleName = data->index(index.row(), nameIdx).data().toString();
    query.exec("SELECT PicPath From Sample WHERE Sample.NAME = '" + sampleName + "'");
    query.last();
    QString samplePicPath = SAMPLE_PIC_PATH + query.value(0).toString();
    if( !QFile::exists(samplePicPath) || samplePicPath.right(1) == "/"){
        ui->label_SamplePic->setText("no image found.\nPlease check the file location.");
    }else{
        QImage samplePic(samplePicPath);
        int imageSize = samplePic.height();
        if( imageSize > maxImageSize ) imageSize = maxImageSize;
        QImage scaledSamplePic = samplePic.scaledToHeight(imageSize);
        ui->label_SamplePic->setPixmap(QPixmap::fromImage(scaledSamplePic));
    }

    // find sample spectrum;
    query.exec("SELECT SpectrumPath From Sample WHERE Sample.NAME = '" + sampleName + "'");
    query.last();
    QString spectrumPicPath = SAMPLE_PIC_PATH + query.value(0).toString();
    if( !QFile::exists(spectrumPicPath) || spectrumPicPath.right(1) == "/"){
        ui->label_SampleSpectrum->setText("no image found.\nPlease check the file location.");
    }else{
        QImage spectrumPic(spectrumPicPath);
        int imageSize = spectrumPic.height();
        if( imageSize > maxImageSize ) imageSize = maxImageSize;
        QImage scaledSpectrumPic = spectrumPic.scaledToHeight(imageSize);
        ui->label_SampleSpectrum->setPixmap(QPixmap::fromImage(scaledSpectrumPic));
    }

    //find chemcial picture;
    query.exec("SELECT ChemicalID From Sample WHERE Sample.NAME = '" + sampleName + "'");
    query.last();
    QString chemicalID = query.value(0).toString();
    query.exec("SELECT PicPath From Chemical WHERE Chemical.ID = '" + chemicalID + "'");
    query.last();
    QString chemicalPicPath = CHEMICAL_PIC_PATH + query.value(0).toString();
    if( !QFile::exists(chemicalPicPath) || chemicalPicPath.right(1) == "/"){
        ui->label_ChemPicture->setText("no image found.\nPlease check the file location.");
    }else{
        QImage chemicalPic(chemicalPicPath);
        int imageSize = chemicalPic.height();
        if( imageSize > maxImageSize ) imageSize = maxImageSize;
        QImage scaledChemicalPic = chemicalPic.scaledToHeight(imageSize);
        ui->label_ChemPicture->setPixmap(QPixmap::fromImage(scaledChemicalPic));
    }

    // set the chemical combox
    enableChemicalFilter = false;
    //when there is a skipped ID in table "Chemical", error occur;
    ui->comboBox_chemical->setCurrentIndex(chemicalID.toInt());
    enableChemicalFilter = true;

}


void DataBaseWindow::on_pushButton_openFolder_clicked()
{
    QModelIndex current = ui->dataView->selectionModel()->currentIndex(); // the "current" item
    QString dataPath = data->index(current.row(), dataPathCol).data().toString();

    //check is it absolute path or relativePath
    QStringList dataNameList = dataPath.split("/");
    int size = dataNameList.size();

    QString folderPath = DATA_PATH + "/";
    for( int i = 0; i < size - 1; i++){
        folderPath += dataNameList[i];
        folderPath += "/";
    }

    QDesktopServices::openUrl( folderPath );
}
