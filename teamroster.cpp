#include "teamroster.h"
#include "../core/ComboBoxDelegate.h"

TeamRoster::TeamRoster(QWidget *parent) : QWidget(parent)
{
    //m_ui.setupUi(this);

    m_model = new GiocatoriInsert();

    m_ui.tableViewPlayers->verticalHeader()->hide();
    m_ui.tableViewPlayers->setItemDelegateForColumn(2, new ComboBoxDelegate());
    m_ui.tableViewPlayers->setModel(m_model);
    m_ui.tableViewPlayers->show();

    connect(m_ui.pushButtonNew, SIGNAL(pressed()), this, SLOT(insertRow()));
    connect(m_ui.pushButtonDelete, SIGNAL(pressed()), this, SLOT(removeRow()));

    //connect(m_ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    //connect(m_ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void TeamRoster::insertRow() {
    m_ui.tableViewPlayers->model()->insertRows(0, 1);
}

void TeamRoster::removeRow() {

    int row = m_ui.tableViewPlayers->currentIndex().row();
    m_ui.tableViewPlayers->model()->removeRows(row, 1);
}

