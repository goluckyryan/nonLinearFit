#ifndef DATABASEWINDOW_H
#define DATABASEWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QVector>
#include <QMessageBox>
#include <QImage>
#include "constant.h"
#include "dateformatdelegate.h"
//#include "openfiledelegate.h"

namespace Ui {
class DataBaseWindow;
}

class DataBaseWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DataBaseWindow(QWidget *parent = 0);
    ~DataBaseWindow();

    QString GetMsg() {return msg;}

    QStringList GetTableColEntries(QString tableName, int col);
    int GetTableColNumber(QString tableName);
    QStringList GetTableColName(QString tableName);

    void ShowTable(QString tableName);
    bool IsDBOpened() {return isDBOpened;}

signals:
    void ReturnFilePath(QString filePath);
    void SendMsg(QString msg);

private slots:
    void SetupSampleTableView();
    void SetupDataTableView();

    void updateChemicalCombox(QString tableName);

    void on_comboBox_chemical_currentTextChanged(const QString &arg1);

    void on_pushButton_selectSample_clicked();
    void on_pushButton_open_clicked();

    void on_checkBox_sortData_clicked();
    void on_checkBox_showChemical_clicked();

    void showSamplePicture(const QModelIndex &index);
    void showDataPicture(const QModelIndex &index);

private:
    Ui::DataBaseWindow *ui;
    QSqlRelationalTableModel *sample;
    QSqlRelationalTableModel *data;

    QSqlDatabase db;

    int dataPathCol;
    int maxImageSize;

    QString msg;
    bool isDBOpened;
    bool enableChemicalFilter;
};

#endif // DATABASEWINDOW_H
