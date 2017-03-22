#ifndef DATABASEWINDOW_H
#define DATABASEWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QVector>
#include <QMessageBox>
#include <QImage>
#include "constant.h"
#include "dateformatdelegate.h"
#include "openfiledelegate.h"
#include "tableeditor.h"

namespace Ui {
class DataBaseWindow;
}

class DataBaseWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DataBaseWindow(QWidget *parent = 0);
    ~DataBaseWindow();

    QStringList GetTableColEntries(QString tableName, int col);
    int GetTableColNumber(QString tableName);

    void ShowTable(QString tableName);

signals:
    QString ReturnFilePath(QString filePath);

private slots:
    void SetupSampleTableView();
    void updateChemicalCombox(QString tableName);

    void on_comboBox_chemical_currentTextChanged(const QString &arg1);

    void on_pushButton_editChemical_clicked();
    void on_pushButton_editHost_clicked();
    void on_pushButton_editSolvent_clicked();

    void on_pushButton_selectSample_clicked();
    void on_pushButton_addSampleEntry_clicked();
    void on_pushButton_deleteSampleEntry_clicked();
    void on_pushButton_revertSample_clicked();
    void on_pushButton_sumitSample_clicked();

    void on_pushButton_addDataEntry_clicked();
    void on_pushButton_deleteDataEntry_clicked();
    void on_pushButton_revertData_clicked();
    void on_pushButton_submitData_clicked();

    void on_pushButton_open_clicked();

    void on_checkBox_clicked(bool checked);

private:
    Ui::DataBaseWindow *ui;
    QSqlRelationalTableModel *sample;
    QSqlRelationalTableModel *data;

    QSqlDatabase db;

    TableEditor *editorChemical;
    TableEditor *editorHost;
    TableEditor *editorSolvent;
};

#endif // DATABASEWINDOW_H
