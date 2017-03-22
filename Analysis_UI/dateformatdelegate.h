#ifndef DATEFORMATDELEGATE_H
#define DATEFORMATDELEGATE_H

/****************
 * copy from tutorial, spreadsheetdelegate.h
 *
*****************/

#include <QStyledItemDelegate>
#include <QDate>
#include <QDateTimeEdit>
#include <QDebug>

class DateFormatDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DateFormatDelegate(QObject * parent = 0);
    //virtual QString displayText(const QVariant &value, const QLocale &locale) const;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

signals:

public slots:


};

#endif // DATEFORMATDELEGATE_H
