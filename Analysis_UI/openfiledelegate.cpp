#include "openfiledelegate.h"

OpenFileDelegate::OpenFileDelegate(int falg, QObject *parent) : QStyledItemDelegate(parent)
{
    this->flag = falg;
}

QWidget *OpenFileDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFileDialog * editor = new QFileDialog(parent);

    if(flag == 0) editor->setDirectory(DATA_PATH);
    editor->setReadOnly(true);
    QStringList filters;
    filters << "Data File (*txt *dat *csv *.*)";
    editor->setNameFilters(filters);


    return editor;
}

void OpenFileDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QFileDialog *fileDialog = qobject_cast<QFileDialog *> (editor);
    QStringList filePathList = fileDialog->selectedFiles();
    QString filePath = filePathList[0];

    if( flag == 0){
        int length = filePath.length();
        int lengthDATA = DATA_PATH.length();
        //qDebug() << length << "," << filePath ;
        //qDebug() << lengthDATA << "," << DATA_PATH ;
        //qDebug() << filePath.right(length-lengthDATA);

        if( filePath.left(lengthDATA) == DATA_PATH ){
            QString fileName = filePath.right(length-lengthDATA);
            model->setData(index, fileName);
            return;
        }else{
            model->setData(index, filePath);
            return;
        }

    }

    if( flag == 1){
        QStringList fileName = filePath.split("/");
        int length = fileName.size();
        //qDebug() << filePath;
        //qDebug() << fileName;
        model->setData(index, fileName[length-1]);
        return;
    }
}

void OpenFileDelegate::updateEditorGeometry(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //qDebug() << parent->geometry();
    //qDebug() << parent->parentWidget()->parentWidget()->parentWidget()->parentWidget()->parentWidget()->pos();
    if( flag == 0){
        QRect rect = parent->parentWidget()->parentWidget()->parentWidget()->parentWidget()->parentWidget()->geometry();
        parent->setGeometry(rect.x()+rect.width()/4., rect.y()+rect.height()/4., rect.width()/2, rect.height()*3./4.);
    }else{
        parent->setGeometry(500, 500, 800, 500);
    }

}
