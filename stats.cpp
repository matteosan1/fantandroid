#include "stats.h"

#include <QSqlDatabase>
#include <QTableWidget>
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QtAlgorithms>

#include "giocatore.h"

Stats::Stats(QSqlDatabase* db,
             QTableWidget* tableTopScorer,
             QTableWidget* tableTopWeek,
             QTableWidget* tableFlopWeek,
             QTableWidget* tableTopOverall,
             QObject *parent) :
    QObject(parent),
    m_db(db),
    m_tableTopScorer(tableTopScorer),
    m_tableTopWeek(tableTopWeek),
    m_tableFlopWeek(tableFlopWeek),
    m_tableTopOverall(tableTopOverall),
    m_topScorerRanking(false),
    m_topFlop(false)
{}

void Stats::init()
{
    m_tableTopScorer->setRowCount(15);
    m_tableTopScorer->setColumnCount(2);
    QStringList headers;
    headers << "Giocatore" << "Goal";
    m_tableTopScorer->setHorizontalHeaderLabels(headers);
    m_tableTopScorer->verticalHeader()->setVisible(false);

    m_tableTopWeek->setRowCount(3);
    m_tableTopWeek->setColumnCount(2);
    headers.clear();
    headers << "Giocatore" << "Fantapunteggio";
    m_tableTopWeek->setHorizontalHeaderLabels(headers);
    m_tableTopWeek->verticalHeader()->setVisible(false);

    m_tableFlopWeek->setRowCount(3);
    m_tableFlopWeek->setColumnCount(2);
    headers.clear();
    headers << "Giocatore" << "Fantapunteggio";
    m_tableFlopWeek->setHorizontalHeaderLabels(headers);
    m_tableFlopWeek->verticalHeader()->setVisible(false);

    m_tableTopOverall->setRowCount(15);
    m_tableTopOverall->setColumnCount(3);
    headers.clear();
    headers << "Giocatore" << "Ruolo" << "Fantamedia";
    m_tableTopOverall->setHorizontalHeaderLabels(headers);
    m_tableTopOverall->verticalHeader()->setVisible(false);
}

void Stats::fillTopScorerRanking()
{
    if (!m_topScorerRanking)
    {
        m_topScorerRanking = true; // todo fix it if an update occurs

        // todo better format and add scored penalties

        QString qryStr = "SELECT playerStats.name,surname,sum(scored) AS sscored FROM playerStats,roster where playerStats.name=roster.surname||' '||substr(roster.name,1,1)||'.' GROUP BY playerStats.name ORDER BY sscored DESC;";
        QSqlQuery query(*m_db);
        if (!query.exec(qryStr))
            qDebug() << query.lastError().text();

        //m_tableTopScorer->resizeRowsToContents();
        m_tableTopScorer->setColumnWidth(0, 200);
        m_tableTopScorer->horizontalHeader()->setStretchLastSection(true);
        int i=0;
        while (query.next())
        {
            if (query.value(2).toInt() == 0)
                break;

            m_tableTopScorer->setItem(i, 0, new QTableWidgetItem(query.value(1).toString() + " " +
                                                                 query.value(0).toString().left(1) + "."));
            m_tableTopScorer->setItem(i, 1, new QTableWidgetItem(query.value(2).toString()));

            i++;
            if (i==15)
                break;
        }
    }
}

void Stats::fillTopFlop()
{
    if (!m_topFlop)
    {
        m_topFlop = true;
        QString qryStr = "SELECT MAX(round) from playerStats;";
        QSqlQuery query(*m_db);
        if (!query.exec(qryStr))
            qDebug() << query.lastError().text();

        int round = -1;
        if (query.first())
            round = query.value(0).toInt();

        int index = 0;
        m_tableTopWeek->setColumnWidth(0, 200);
        qryStr = "SELECT roster.surname||' '||substr(roster.name,1,1)||'.' AS fn, finalVote AS v FROM roster, playerStats WHERE fn=playerStats.name AND round=" + QString::number(round) + " ORDER BY v DESC, fn ASC;";
        query.exec(qryStr);

        while (query.next())
        {
            m_tableTopWeek->setItem(index, 0, new QTableWidgetItem(query.value(0).toString()));
            //m_tableTopWeek->setRowHeight(index, 40);
            m_tableTopWeek->setItem(index, 1, new QTableWidgetItem(query.value(1).toString()));

            index++;
            if (index == 3)
                break;
        }
        m_tableTopWeek->horizontalHeader()->setStretchLastSection(true);

        index = 0;
        m_tableFlopWeek->setColumnWidth(0, 200);
        qryStr = "SELECT roster.surname||' '||substr(roster.name,1,1)||'.' AS fn, finalVote AS v FROM roster, playerStats WHERE fn=playerStats.name AND vote <> 0 AND round=" + QString::number(round) + " ORDER BY v ASC, fn ASC;";
        if (!query.exec(qryStr))
            qDebug() << query.lastError().text();

        while (query.next())
        {
            //m_tableFlopWeek->setRowHeight(index, 20);
            m_tableFlopWeek->setItem(index, 0, new QTableWidgetItem(query.value(0).toString()));
            m_tableFlopWeek->setItem(index, 1, new QTableWidgetItem(query.value(1).toString()));

            index++;
            if (index == 3)
                break;
        }
        m_tableFlopWeek->horizontalHeader()->setStretchLastSection(true);

        qryStr = "SELECT roster.name, roster.surname, roster.surname||' '||substr(roster.name,1,1)||'.' AS fn, AVG(finalVote) as average, roster.role AS v FROM roster, playerStats WHERE fn=playerStats.name GROUP BY fn;";
        if (!query.exec(qryStr))
            qDebug() << query.lastError().text();

        QList<Giocatore*> players;
        while (query.next())
        {
            Giocatore* g = new Giocatore();
            g->setCognome(query.value(1).toString());
            g->setNome(query.value(0).toString());
            g->setRuolo(query.value(4).toInt());
            g->setMedia(query.value(3).toFloat());
            players.append(g);
        }

        m_tableTopOverall->setColumnWidth(0, 200);
        qSort(players.begin(), players.end(), Giocatore::PlayerComparator);
        index = 0;
        foreach (Giocatore* p, players)
        {
            RuoloEnum role = p->ruolo();
            if (role == Portiere and index == 0)
            {
                //m_tableTopOverall->setRowHeight(index, 20);
                m_tableTopOverall->setItem(index, 0, new QTableWidgetItem(p->nomeCompleto()));
                m_tableTopOverall->setItem(index, 1, new QTableWidgetItem(Giocatore::ruoloToString(role)));
                m_tableTopOverall->setItem(index, 2, new QTableWidgetItem(QString::number(p->media(), 'f', 2)));

                index++;
            }
            if (role == DifensoreTerzino and index > 0 and index <= 2)
            {
                //m_tableTopOverall->setRowHeight(index, 20);
                m_tableTopOverall->setItem(index, 0, new QTableWidgetItem(p->nomeCompleto()));
                m_tableTopOverall->setItem(index, 1, new QTableWidgetItem(Giocatore::ruoloToString(role)));
                m_tableTopOverall->setItem(index, 2, new QTableWidgetItem(QString::number(p->media(), 'f', 2)));

                index++;
            }
            if (role == DifensoreCentrale and index > 2 and index <= 4)
            {
                //m_tableTopOverall->setRowHeight(index, 20);
                m_tableTopOverall->setItem(index, 0, new QTableWidgetItem(p->nomeCompleto()));
                m_tableTopOverall->setItem(index, 1, new QTableWidgetItem(Giocatore::ruoloToString(role)));
                m_tableTopOverall->setItem(index, 2, new QTableWidgetItem(QString::number(p->media(), 'f', 2)));

                index++;
            }
            if (role == CentrocampistaCentrale and index > 4 and index <= 6)
            {
                //m_tableTopOverall->setRowHeight(index, 20);
                m_tableTopOverall->setItem(index, 0, new QTableWidgetItem(p->nomeCompleto()));
                m_tableTopOverall->setItem(index, 1, new QTableWidgetItem(Giocatore::ruoloToString(role)));
                m_tableTopOverall->setItem(index, 2, new QTableWidgetItem(QString::number(p->media(), 'f', 2)));

                index++;
            }
            if (role == CentrocampistaEsterno and index > 6 and index <= 8)
            {
                //m_tableTopOverall->setRowHeight(index, 20);
                m_tableTopOverall->setItem(index, 0, new QTableWidgetItem(p->nomeCompleto()));
                m_tableTopOverall->setItem(index, 1, new QTableWidgetItem(Giocatore::ruoloToString(role)));
                m_tableTopOverall->setItem(index, 2, new QTableWidgetItem(QString::number(p->media(), 'f', 2)));

                index++;
            }
            if (role == CentrocampistaTrequartista and index > 8 and index <= 10)
            {
                //m_tableTopOverall->setRowHeight(index, 20);
                m_tableTopOverall->setItem(index, 0, new QTableWidgetItem(p->nomeCompleto()));
                m_tableTopOverall->setItem(index, 1, new QTableWidgetItem(Giocatore::ruoloToString(role)));
                m_tableTopOverall->setItem(index, 2, new QTableWidgetItem(QString::number(p->media(), 'f', 2)));

                index++;
            }
            if (role == AttaccanteCentrale and index > 10 and index <= 12)
            {
                //m_tableTopOverall->setRowHeight(index, 20);
                m_tableTopOverall->setItem(index, 0, new QTableWidgetItem(p->nomeCompleto()));
                m_tableTopOverall->setItem(index, 1, new QTableWidgetItem(Giocatore::ruoloToString(role)));
                m_tableTopOverall->setItem(index, 2, new QTableWidgetItem(QString::number(p->media(), 'f', 2)));

                index++;
            }
            if (role == AttaccanteMovimento and index > 12 and index <= 14)
            {
                //m_tableTopOverall->setRowHeight(index, 20);
                m_tableTopOverall->setItem(index, 0, new QTableWidgetItem(p->nomeCompleto()));
                m_tableTopOverall->setItem(index, 1, new QTableWidgetItem(Giocatore::ruoloToString(role)));
                m_tableTopOverall->setItem(index, 2, new QTableWidgetItem(QString::number(p->media(), 'f', 2)));

                index++;
            }
        }
        m_tableTopOverall->horizontalHeader()->setStretchLastSection(true);
    }
}
