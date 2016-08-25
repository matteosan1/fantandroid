#ifndef TEAMROSTER_H
#define TEAMROSTER_H

#include "ui_mainwindow.h"
#include "../core/insertgiocatori.h"

class QAbstractItemModel;

class TeamRoster : public QWidget
{
    Q_OBJECT

public:
    explicit TeamRoster(QWidget *parent = 0);
    QList<Giocatore*> giocatori() {
        GiocatoriInsert* temp = dynamic_cast<GiocatoriInsert*>(m_model);
        return temp->GetGiocatori();
    }

public slots:
    void insertRow();
    void removeRow();

private:
    Ui::MainWindowTemp  m_ui;
    QAbstractItemModel* m_model;
};

#endif // TEAMROSTER_H

