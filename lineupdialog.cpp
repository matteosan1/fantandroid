#include "lineupdialog.h"

#include <QListWidgetItem>

#include <QDebug>

#define EMPTY_PLAYER "--------"

LineupDialog::LineupDialog(QList<Giocatore*> giocatori,
                       Punteggi* punteggi,
                       QWidget *parent) : QDialog(parent),
    m_giocatori(giocatori),
    m_punteggi(punteggi) {

    m_ui.setupUi(this);

    m_giornata = -1;

    m_ui.comboBoxModulo->addItem(m_punteggi->m_schemiString[0]);
    for (int i=1; i<NUMERO_SCHEMI; i++)
    {
        //if (m_punteggi->m_schemi[i-1] == 1)
        m_ui.comboBoxModulo->addItem(m_punteggi->m_schemiString[i]);
    }

    connect(m_ui.comboBoxModulo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateModules(int)));
    connect(m_ui.pushButtonAggiungii, SIGNAL(clicked(bool)), this, SLOT(aggiungiPushed()));
    connect(m_ui.listPanchinari, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(removePanchinaro(QListWidgetItem*)));
    connect(m_ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    m_combos.append(m_ui.comboBoxPortiere);
    m_combos.append(m_ui.comboBoxDifensoreDx);
    m_combos.append(m_ui.comboBoxDifensoreSx);
    m_combos.append(m_ui.comboBoxDifensoreCx1);
    m_combos.append(m_ui.comboBoxDifensoreCx2);
    m_combos.append(m_ui.comboBoxDifensoreCx3);
    m_combos.append(m_ui.comboBoxCentrocampistaDx);
    m_combos.append(m_ui.comboBoxCentrocampistaSx);
    m_combos.append(m_ui.comboBoxCentrocampistaCx1);
    m_combos.append(m_ui.comboBoxCentrocampistaCx2);
    m_combos.append(m_ui.comboBoxCentrocampistaCx3);
    m_combos.append(m_ui.comboBoxAttaccanteDx);
    m_combos.append(m_ui.comboBoxAttaccanteSx);
    m_combos.append(m_ui.comboBoxAttaccanteCx);

    int index = 0;
    foreach (QComboBox* combo, m_combos)
    {
        combo->setObjectName(QString::number(index));
        connect(combo, SIGNAL(currentIndexChanged(QString)), this, SLOT(playerSelected(QString)));
        index++;
    }

    disableAllCombo();
}

void LineupDialog::playerSelected(QString player)
{
    if (player == EMPTY_PLAYER)
        return;

    QComboBox* sender = (QComboBox*)QObject::sender();
    if (sender != NULL)
    {
        int senderId = sender->objectName().toInt();

        foreach (Giocatore* g, m_giocatori)
        {
            if (g->nomeCompleto() == player)
            {
                int index = g->partita(0).Formazione();
                g->partita(0).SetFormazione(senderId);
                if (index != -1)
                {
                    m_combos.at(index)->setCurrentIndex(0);
                    break;
                }
            }
        }
    }

    int nGiocano = 0;
    foreach (Giocatore* g, m_giocatori)
    {
        int index = g->partita(0).Formazione();
        if (index != -1)
            nGiocano++;
    }

    if (nGiocano == 11)
    {
        // todo check nGiocano, capitano settato e panchina fatta
        m_ui.buttonBox->setEnabled(true);
        m_ui.comboBoxCapitano->clear();
        m_ui.comboBoxPanchinari->clear();
        foreach (Giocatore* g, m_giocatori)
        {
            if (g->partita(0).Formazione() != -1)
                m_ui.comboBoxCapitano->addItem(g->nomeCompleto());
            else
                m_ui.comboBoxPanchinari->addItem(g->nomeCompleto());
        }
    }
    else
    {
        m_ui.buttonBox->setEnabled(false);
        m_ui.comboBoxCapitano->clear();
        m_ui.comboBoxPanchinari->clear();
    }
}

void LineupDialog::updateModules(const int& chosenModule)
{
    int realModule = -1;
    int counter = 0;
    for (int i=1; i<NUMERO_SCHEMI; i++)
    {
        //if (m_punteggi->m_schemi[i-1] != 0)
        counter++;

        if (counter == chosenModule)
            realModule = i-1;
    }

    disableAllCombo();
    enableCombo(m_ui.comboBoxPortiere, Portiere);

    switch(realModule)
    {   
    case 0: // 5-4-1
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx3, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaSx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxCentrocampistaDx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxAttaccanteCx, AttaccanteCentrale);
        break;
    case 1: // 5-3-1-1
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx3, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxAttaccanteCx, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteMovimento);
        break;
    case 2: // 5-3-2
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx3, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteMovimento, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteMovimento, AttaccanteCentrale);
        break;
    case 3: // 4-5-1
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaSx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxCentrocampistaDx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxAttaccanteCx, AttaccanteCentrale);
        break;        
    case 4: // 4-4-2
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaSx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxCentrocampistaDx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteMovimento, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteMovimento, AttaccanteCentrale);
        break;
    case 5: // 4-3-2-1
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxAttaccanteDx, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxAttaccanteSx, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxAttaccanteCx, AttaccanteCentrale);
        break;
    case 6: // 4-3-1-2
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxAttaccanteCx, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteCentrale);
        break;
    case 7: // 4-3-3
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxAttaccanteCx, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteMovimento);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteMovimento);
        break;
    case 8: // 4-2-3-1
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxAttaccanteCx, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaDx, CentrocampistaEsterno, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxCentrocampistaSx, CentrocampistaEsterno, CentrocampistaTrequartista);
        break;
    case 9: // 4-2-1-3
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxAttaccanteCx, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteMovimento);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteMovimento);
        break;
    case 10: // 4-2-4
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreLaterale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteCx, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteMovimento, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteMovimento, CentrocampistaEsterno);
        break;
    case 11: // 3-5-2
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx3, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaSx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxCentrocampistaDx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteMovimento, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteMovimento, AttaccanteCentrale);
        break;
    case 12: // 3-4-1-2
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx3, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaSx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxCentrocampistaDx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxAttaccanteCx, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteMovimento, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteMovimento, AttaccanteCentrale);
        break;
    case 13: // 3-4-3
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx3, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaSx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxCentrocampistaDx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxAttaccanteCx, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteMovimento);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteMovimento);
        break;     
    }
}

void LineupDialog::enableCombo(QComboBox* combo, RuoloEnum ruolo1, RuoloEnum ruolo2)
{
    combo->setEnabled(true);
    combo->setVisible(true);

    combo->clear();
    combo->addItem(EMPTY_PLAYER);

    foreach (Giocatore* g, m_giocatori)
    {
        if (g->ruolo() == ruolo1 or g->ruolo() == ruolo2)
        {
            combo->addItem(g->nomeCompleto());
        }
    }
}

void LineupDialog::disableAllCombo()
{
    m_ui.comboBoxPortiere->setEnabled(false);
    m_ui.comboBoxDifensoreSx->setEnabled(false);
    m_ui.comboBoxDifensoreDx->setEnabled(false);
    m_ui.comboBoxDifensoreCx1->setEnabled(false);
    m_ui.comboBoxDifensoreCx2->setEnabled(false);
    m_ui.comboBoxDifensoreCx3->setEnabled(false);
    m_ui.comboBoxCentrocampistaCx1->setEnabled(false);
    m_ui.comboBoxCentrocampistaCx2->setEnabled(false);
    m_ui.comboBoxCentrocampistaCx3->setEnabled(false);
    m_ui.comboBoxCentrocampistaSx->setEnabled(false);
    m_ui.comboBoxCentrocampistaDx->setEnabled(false);
    m_ui.comboBoxAttaccanteCx->setEnabled(false);
    m_ui.comboBoxAttaccanteDx->setEnabled(false);
    m_ui.comboBoxAttaccanteSx->setEnabled(false);

    m_ui.comboBoxPortiere->setVisible(false);
    m_ui.comboBoxDifensoreSx->setVisible(false);
    m_ui.comboBoxDifensoreDx->setVisible(false);
    m_ui.comboBoxDifensoreCx1->setVisible(false);
    m_ui.comboBoxDifensoreCx2->setVisible(false);
    m_ui.comboBoxDifensoreCx3->setVisible(false);
    m_ui.comboBoxCentrocampistaCx1->setVisible(false);
    m_ui.comboBoxCentrocampistaCx2->setVisible(false);
    m_ui.comboBoxCentrocampistaCx3->setVisible(false);
    m_ui.comboBoxCentrocampistaSx->setVisible(false);
    m_ui.comboBoxCentrocampistaDx->setVisible(false);
    m_ui.comboBoxAttaccanteCx->setVisible(false);
    m_ui.comboBoxAttaccanteDx->setVisible(false);
    m_ui.comboBoxAttaccanteSx->setVisible(false);
}

void LineupDialog::aggiungiPushed()
{
    QString player = m_ui.comboBoxPanchinari->currentText();

    bool alreadyIn = false;
    for (int i=0; i<m_ui.listPanchinari->count(); ++i)
    {
        if (player == m_ui.listPanchinari->item(i)->text())
        {
            alreadyIn = true;
            break;
        }
    }

    if (!alreadyIn)
    {
        int row = m_ui.listPanchinari->currentRow();
        if (row == -1)
            row = m_ui.listPanchinari->count() + 1;
        m_ui.listPanchinari->insertItem(row+1, player);
    }
}

void LineupDialog::removePanchinaro(QListWidgetItem* item)
{
    delete item;
}

QString LineupDialog::finalizeMessage()
{
    QString text;
    QString capitano = m_ui.comboBoxCapitano->currentText();

    text = text + "Modulo: " + m_ui.comboBoxModulo->currentText() + "\n";
    foreach(QComboBox* combo, m_combos)
    {
        if (combo->isEnabled())
        {
            QString player = combo->currentText();
            RuoloEnum ruolo;

            foreach (Giocatore* g, m_giocatori)
            {
                if (g->nomeCompleto() == player)
                {
                    ruolo = g->ruolo();
                    break;
                }
            }

            if (player == capitano)
                text = text + player + " [" + Giocatore::ruoloToString(ruolo) + "] (C)\n";
            else
                text = text + player + " [" + Giocatore::ruoloToString(ruolo) + "]\n";
        }
    }

    text = text + "\nPANCHINA\n";
    for (int i=0; i<m_ui.listPanchinari->count(); ++i)
    {
        text = text + m_ui.listPanchinari->item(i)->text() + "\n";
    }

    return text;
}
