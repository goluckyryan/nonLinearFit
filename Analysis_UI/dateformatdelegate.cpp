#include "dateformatdelegate.h"

DateFormatDelegate::DateFormatDelegate(QObject *parent): QStyledItemDelegate(parent)
{

}

//QString DateFormatDelegate::displayText(const QVariant &value, const QLocale &locale) const
//{
//    //qDebug() << value;
//    QString formattedDate = locale.toString(value.toDate(), "yyyy-MM-dd");
//
//    return formattedDate;
//}

QWidget *DateFormatDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QDateTimeEdit *editor = new QDateTimeEdit(parent);
    editor->setDate(QDate::currentDate());
    editor->setDisplayFormat("yyyy-MM-dd");
    editor->setCalendarPopup(true);

    return editor;
}

void DateFormatDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QDateTimeEdit *dateEditor = qobject_cast<QDateTimeEdit *>(editor);
    if (dateEditor) {
        dateEditor->setDate(QDate::fromString(index.model()->data(index, Qt::EditRole).toString(),"yyyy-MM-dd"));
    }
}

void DateFormatDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QDateTimeEdit *dateEditor = qobject_cast<QDateTimeEdit *>(editor);
    if (dateEditor) model->setData(index, dateEditor->date().toString("yyyy-MM-dd"));
}
