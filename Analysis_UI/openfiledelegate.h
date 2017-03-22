#ifndef OPENFILEDELEGATE_H
#define OPENFILEDELEGATE_H

#include <QStyledItemDelegate>
#include <QFileDialog>
#include <QDebug>
#include "constant.h"

class OpenFileDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    OpenFileDelegate(int falg = 0, QObject * parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    //QWidget *createEditor(QWidget *parent) const;

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    int flag;

};

#endif // OPENFILEDELEGATE_H
