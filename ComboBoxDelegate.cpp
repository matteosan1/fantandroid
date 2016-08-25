#include "ComboBoxDelegate.h"

#include <QtGui>
#include <QStringList>

ComboBoxDelegate::ComboBoxDelegate(QObject *parent)
    : QItemDelegate(parent)
{}

QWidget* ComboBoxDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem&,
                                        const QModelIndex& index) const {
    Q_UNUSED(index);
    // todo spostare in giocatore
    QStringList list;
    list << "P"
         << "DT"
         << "DC"
         << "CE"
         << "CC"
         << "CT"
         << "AC"
         << "AM";

    QComboBox *editor = new QComboBox(parent);
    editor->insertItems(0, list);

    return editor;
}

void ComboBoxDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const {

    int value = index.model()->data(index, Qt::DisplayRole).toInt();
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    comboBox->setCurrentIndex(value);
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const {

    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int value = comboBox->currentIndex();
    model->setData(index, value);
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor,
                                            const QStyleOptionViewItem &option, const QModelIndex &/* index */) const {
    editor->setGeometry(option.rect);
}
