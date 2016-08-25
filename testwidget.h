#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include "ui_test.h"
#include "../core/insertgiocatori.h"

#include <QWidget>

class TestWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TestWidget(QWidget *parent = 0);
    QList<Giocatore*> giocatori() {
        GiocatoriInsert* temp = dynamic_cast<GiocatoriInsert*>(m_model);
        return temp->GetGiocatori();
    }

public slots:
    void insertRow();
    void removeRow();

private:
    QAbstractItemModel* m_model;
    Ui::Test m_ui;
};

#endif // TESTWIDGET_H
