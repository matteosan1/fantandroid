#include "stats.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QtAlgorithms>
#include <QTextEdit>

#include "giocatore.h"

Stats::Stats(QSqlDatabase* db,
             QTextEdit* tableTopScorer,
             QTextEdit* tableTopWeek,
             QTextEdit* tableFlopWeek,
             QTextEdit* tableTopOverall,
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
{}

void Stats::fillTopScorerRanking()
{
    if (!m_topScorerRanking)
    {
        m_topScorerRanking = true; // todo fix it if an update occurs

        // TODO add scored penalties

        // FIXME
        QString qryStr = "SELECT playerStats.name,surname,sum(scored) AS sscored FROM playerStats,roster where playerStats.name=roster.surname GROUP BY playerStats.name ORDER BY sscored DESC;";
        QSqlQuery query(*m_db);
        if (!query.exec(qryStr))
            qDebug() << query.lastError().text();

        QString text;
        text = "<table style=\"width:100%\">";
        text += "<tr><th width=250 align=\"left\">Giocatore</th><th>Goal Segnati</th></tr>";

        int i=0;
        while (query.next())
        {
            if (query.value(2).toInt() == 0)
                break;

            text += "<tr><td>" + query.value(1).toString() + "</td><td align=\"center\">" + query.value(2).toString() + "</td></tr>";

            i++;
            if (i==15)
                break;
        }
        text += "</table>";
        m_tableTopScorer->append(text);
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
        qryStr = "SELECT roster.surname AS fn, finalVote AS v FROM roster, playerStats WHERE fn=playerStats.name AND round=" + QString::number(round) + " ORDER BY v DESC, fn ASC;";
        query.exec(qryStr);

        QString text;
        text = "<table style=\"width:100%\">";
        text += "<tr><th width=200 align=\"left\">Giocatore</th><th>Fantapunti</th></tr>";

        while (query.next())
        {
            text += "<tr><td>" + query.value(0).toString() + "</td><td align=\"center\">" + query.value(1).toString() + "</td></tr>";
            index++;
            if (index == 5)
                break;
        }
        text += "</table>";
        m_tableTopWeek->append(text);

        index = 0;
        text = "<table style=\"width:100%\">";
        text += "<tr><th width=200 align=\"left\">Giocatore</th><th>Fantapunti</th></tr>";

        qryStr = "SELECT roster.surname AS fn, finalVote AS v FROM roster, playerStats WHERE fn=playerStats.name AND vote <> 0 AND round=" + QString::number(round) + " ORDER BY v ASC, fn ASC;";
        if (!query.exec(qryStr))
            qDebug() << query.lastError().text();

        while (query.next())
        {
            text += "<tr><td>" + query.value(0).toString() + "</td><td align=\"center\">" + query.value(1).toString() + "</td></tr>";
            index++;
            if (index == 5)
                break;
        }
        text += "</table>";
        m_tableFlopWeek->append(text);

        qryStr = "SELECT roster.name, roster.surname, roster.surname AS fn, AVG(finalVote) as average, roster.role AS v FROM roster, playerStats WHERE fn=playerStats.name GROUP BY fn;";
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

        qSort(players.begin(), players.end(), Giocatore::PlayerComparator);
        index = 0;
        text = "<table style=\"width:100%\">";
        text += "<tr><th width=200 align=\"left\">Giocatore</th><th width=50>Ruolo</th><th>Fantamedia</th></tr>";

        foreach (Giocatore* p, players)
        {
            RuoloEnum role = p->ruolo();
            if (role == Portiere and index == 0)
            {
                text += "<tr><td>" + p->nomeCompleto() + "</td>";
                text += "<td align=\"center\">" + Giocatore::ruoloToString(role) + "</td>";
                text += "<td align=\"center\">" + QString::number(p->media(), 'f', 2) + "</td></tr>";
                index++;
            }

            if (role == DifensoreTerzino and index > 0 and index <= 2)
            {
                text += "<tr><td>" + p->nomeCompleto() + "</td>";
                text += "<td align=\"center\">" + Giocatore::ruoloToString(role) + "</td>";
                text += "<td align=\"center\">" + QString::number(p->media(), 'f', 2) + "</td></tr>";
                index++;
            }

            if (role == DifensoreCentrale and index > 2 and index <= 4)
            {
                text += "<tr><td>" + p->nomeCompleto() + "</td>";
                text += "<td align=\"center\">" + Giocatore::ruoloToString(role) + "</td>";
                text += "<td align=\"center\">" + QString::number(p->media(), 'f', 2) + "</td></tr>";
                index++;
            }

            if (role == CentrocampistaCentrale and index > 4 and index <= 6)
            {
                text += "<tr><td>" + p->nomeCompleto() + "</td>";
                text += "<td align=\"center\">" + Giocatore::ruoloToString(role) + "</td>";
                text += "<td align=\"center\">" + QString::number(p->media(), 'f', 2) + "</td></tr>";
                index++;
            }

            if (role == CentrocampistaEsterno and index > 6 and index <= 8)
            {
                text += "<tr><td>" + p->nomeCompleto() + "</td>";
                text += "<td align=\"center\">" + Giocatore::ruoloToString(role) + "</td>";
                text += "<td align=\"center\">" + QString::number(p->media(), 'f', 2) + "</td></tr>";
                index++;
            }

            if (role == CentrocampistaTrequartista and index > 8 and index <= 10)
            {
                text += "<tr><td>" + p->nomeCompleto() + "</td>";
                text += "<td align=\"center\">" + Giocatore::ruoloToString(role) + "</td>";
                text += "<td align=\"center\">" + QString::number(p->media(), 'f', 2) + "</td></tr>";
                index++;
            }

            if (role == AttaccanteCentrale and index > 10 and index <= 12)
            {
                text += "<tr><td>" + p->nomeCompleto() + "</td>";
                text += "<td align=\"center\">" + Giocatore::ruoloToString(role) + "</td>";
                text += "<td align=\"center\">" + QString::number(p->media(), 'f', 2) + "</td></tr>";
                index++;
            }

            if (role == AttaccanteMovimento and index > 12 and index <= 14)
            {
                text += "<tr><td>" + p->nomeCompleto() + "</td>";
                text += "<td align=\"center\">" + Giocatore::ruoloToString(role) + "</td>";
                text += "<td align=\"center\">" + QString::number(p->media(), 'f', 2) + "</td></tr>";
                index++;
            }
        }
        text += "</table>";
        m_tableTopOverall->append(text);
    }
}
