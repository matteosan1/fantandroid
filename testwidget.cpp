#include "testwidget.h"
#include "../core/ComboBoxDelegate.h"

TestWidget::TestWidget(QWidget *parent) : QWidget(parent)
{
    m_ui.setupUi(this);

    m_model = new GiocatoriInsert();

    m_ui.tableRoster->verticalHeader()->hide();
    m_ui.tableRoster->setItemDelegateForColumn(2, new ComboBoxDelegate());
    m_ui.tableRoster->setModel(m_model);
    m_ui.tableRoster->show();

    //connect(m_ui.pushButtonNew, SIGNAL(pressed()), this, SLOT(insertRow()));
    //connect(m_ui.pushButtonDelete, SIGNAL(pressed()), this, SLOT(removeRow()));

    connect(m_ui.pushButtonOK, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_ui.pushButtonCancel, SIGNAL(rejected()), this, SLOT(reject()));
}

void TestWidget::insertRow() {
    m_ui.tableRoster->model()->insertRows(0, 1);
}

void TestWidget::removeRow() {

    int row = m_ui.tableRoster->currentIndex().row();
    m_ui.tableRoster->model()->removeRows(row, 1);
}
