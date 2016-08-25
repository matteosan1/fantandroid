#ifndef LINEUP_H
#define LINEUP_H

#include "ui_lineup.h"
#include "../core/giocatore.h"
#include "../core/punteggi.h"

#include <QDialog>
#include <QStringList>
#include <QString>
#include <QList>

class QComboBox;
class QListWidgetItem;

class LineupDialog: public QDialog {
    Q_OBJECT

public:
    LineupDialog(QList<Giocatore*> g, Punteggi* punteggi = 0, QWidget *parent = 0);
    int getGiornata() const { return m_giornata; }
    QString finalizeMessage();

private slots:
    void updateModules(const int& chosenModule);
    void playerSelected(QString player);
    void aggiungiPushed();
    void removePanchinaro(QListWidgetItem*);

private:
    void enableCombo(QComboBox* combo, RuoloEnum ruolo1, RuoloEnum ruolo2 = Nullo);
    void disableAllCombo();

    int               m_giornata;
    Ui::lineup        m_ui;
    QList<Giocatore*> m_giocatori;
    Punteggi*         m_punteggi;
    //int m_partite;

    QList<QComboBox*> m_combos;
};

#endif
