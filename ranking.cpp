#include "ranking.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QMap>
#include <QDebug>
#include <QLabel>
#include <QTableWidget>
#include <QStringList>
#include <QFont>
#include <QFontMetrics>
#include <QHeaderView>

#include "team.h"

Ranking::Ranking(int widgetWidth, QSqlDatabase* db, QLabel* label, QTableWidget* tableRanking, QTableWidget* tableScores, QObject *parent) :
    QObject(parent),
    m_widgetWidth(widgetWidth),
    m_db(db),
    m_label(label),
    m_tableRanking(tableRanking),
    m_tableScores(tableScores),
    m_rankingComputed(false),
    m_round(-1),
    m_maxRounds(-1)
{}

void Ranking::init()
{
    m_tableRanking->setRowCount(10);
    m_tableRanking->setColumnCount(10);
    QStringList headers;
    headers << "" << "SQUADRA" << "P" << "G" << "V" << "N" << "P" << "GF" << "GS" << "FM";
    m_tableRanking->setHorizontalHeaderLabels(headers);
    m_tableRanking->verticalHeader()->setVisible(false);
    m_tableRanking->setShowGrid(false);

    computeColumnWidth();

    m_tableScores->setRowCount(3);
    m_tableScores->setColumnCount(3);
    m_tableScores->verticalHeader()->setVisible(false);
    m_tableScores->horizontalHeader()->setVisible(false);
    m_tableScores->setShowGrid(false);
}

void Ranking::changeRoundUp()
{
    if (m_round<m_maxRounds)
        m_round++;

    reportResults();
}

void Ranking::changeRoundDown()
{
    if (m_round > 1)
        m_round--;

    reportResults();
}

void Ranking::computeColumnWidth()
{
    int fontSize = m_tableRanking->fontInfo().pixelSize();
    QFontMetrics fm = m_tableRanking->fontMetrics();
    //int widgetWidth = m_tableRanking->viewport()->geometry().width();// ->width();

    int totalWidth = -1;
    int widthName = -1;
    int widthNumb = -1;
    int widthFM   = -1;

    int i = 0;
    bool isTextOK = false;
    while (!isTextOK)
    {
        widthName = fm.width("AAAAAAAA.");
        widthNumb = fm.width("999");
        widthFM   = fm.width("99.9");

        totalWidth = 25 + widthName + widthNumb*7 + widthFM;
        if (totalWidth > m_widgetWidth)
        {
            QFont font = m_tableRanking->font();
            fontSize--;
            font.setPixelSize(fontSize);
            m_tableRanking->setFont(font);
        }
        else
            isTextOK = true;
        i++;
        if (i == 8)
            break;
    }

    //qDebug() << m_widgetWidth << totalWidth << widthName << widthNumb << widthFM;
    int widthDiff = m_widgetWidth - totalWidth;
    for (int i=2; i<=8; i++)
        m_tableRanking->setColumnWidth(i, widthNumb + widthDiff/8);
    //qDebug() << widthDiff << widthDiff/8;
    m_tableRanking->setColumnWidth(9, widthFM + widthDiff/8);
}

void Ranking::computeRanking()
{
    QSqlQuery query(*m_db);
    QString qryStr;

    if (!m_rankingComputed)
    {
        QMap<QString, Team*> teams;
        // update flag when update has been downloaded

        qryStr = "SELECT name,image from teamName;";
        if (!query.exec(qryStr))
            qDebug() << query.lastError().text();
        while (query.next())
        {
            QString name = query.value(0).toString();
            QByteArray image = query.value(1).toByteArray();
            teams.insert(name, new Team(name, image));
        }

        qryStr = "SELECT * from fixture;";
        if (!query.exec(qryStr))
            qDebug() << query.lastError().text();
        while (query.next())
        {
            if (query.value("played").toInt() == 1)
            {
                Team* team1 = teams[query.value("team1").toString()];
                Team* team2 = teams[query.value("team2").toString()];
                int score1 = query.value("goal1").toInt();
                int score2 = query.value("goal2").toInt();
                float points1 = query.value("points1").toFloat();
                float points2 = query.value("points2").toFloat();

                team1->m_played++;
                team2->m_played++;

                if (score1 > score2)
                {
                    team1->m_points += 2;
                    team1->m_won += 1;
                    team1->m_scored += score1;
                    team1->m_conceded += score2;
                    team1->m_fantaPoints += points1;

                    //team2.m_points += 2;
                    team2->m_lost += 1;
                    team2->m_scored += score2;
                    team2->m_conceded += score1;
                    team2->m_fantaPoints += points2;
                }
                else if (score1 < score2)
                {
                    //team1.m_points += 2;
                    team1->m_lost += 1;
                    team1->m_scored += score1;
                    team1->m_conceded += score2;
                    team1->m_fantaPoints += points1;

                    team2->m_points += 2;
                    team2->m_won += 1;
                    team2->m_scored += score2;
                    team2->m_conceded += score1;
                    team2->m_fantaPoints += points2;
                }
                else
                {
                    team1->m_points += 1;
                    team1->m_drawn += 1;
                    team1->m_scored += score1;
                    team1->m_conceded += score2;
                    team1->m_fantaPoints += points1;

                    team2->m_points += 1;
                    team2->m_drawn += 1;
                    team2->m_scored += score2;
                    team2->m_conceded += score1;
                    team2->m_fantaPoints += points2;
                }
            }
        }

        QList<QString> sortedTeams;
        foreach(QString key, teams.keys())
            sortedTeams.append(key);

        for (int i=0; i<sortedTeams.size()-1; i++)
        {
            for (int j=i+1; j<sortedTeams.size(); j++)
            {
                if ((teams[sortedTeams[i]]->m_points < teams[sortedTeams[j]]->m_points) or
                        ((teams[sortedTeams[i]]->m_points == teams[sortedTeams[j]]->m_points) and
                         (teams[sortedTeams[i]]->m_scored < teams[sortedTeams[j]]->m_scored)) or
                        ((teams[sortedTeams[i]]->m_points == teams[sortedTeams[j]]->m_points) and
                         (teams[sortedTeams[i]]->m_scored == teams[sortedTeams[j]]->m_scored) and
                         (teams[sortedTeams[i]]->diff() < teams[sortedTeams[j]]->diff())))
                {
                    QString temp = sortedTeams[i];
                    sortedTeams[i] = sortedTeams[j];
                    sortedTeams[j] = temp;
                }
            }
        }

        for (int i=0; i<sortedTeams.size(); i++)
        {
            QString key = sortedTeams[i];
            m_tableRanking->setItem(i, 0, new QTableWidgetItem(Team::setTeamIcon(50, teams[key]->m_image), ""));
            m_tableRanking->setColumnWidth(0, 25);
            m_tableRanking->setItem(i, 1, new QTableWidgetItem(Team::reducedName(key)));
            m_tableRanking->setItem(i, 2, new QTableWidgetItem(QString::number(teams[key]->m_points)));
            m_tableRanking->setItem(i, 3, new QTableWidgetItem(QString::number(teams[key]->m_played)));
            m_tableRanking->setItem(i, 4, new QTableWidgetItem(QString::number(teams[key]->m_won)));
            m_tableRanking->setItem(i, 5, new QTableWidgetItem(QString::number(teams[key]->m_drawn)));
            m_tableRanking->setItem(i, 6, new QTableWidgetItem(QString::number(teams[key]->m_lost)));
            m_tableRanking->setItem(i, 7, new QTableWidgetItem(QString::number(teams[key]->m_scored)));
            m_tableRanking->setItem(i, 8, new QTableWidgetItem(QString::number(teams[key]->m_conceded)));
            m_tableRanking->setItem(i, 9, new QTableWidgetItem(QString::number(teams[key]->m_fantaPoints/teams[key]->m_played, 'f', 2)));
        }
    }

    m_tableRanking->setVisible(false);
    //QRect vporig = m_ui.tableWidgetRanking->viewport()->geometry();
    //QRect vpnew = vporig;
    //vpnew.setWidth(std::numeric_limits<int>::max());
    //m_ui.tableWidgetRanking->viewport()->setGeometry(vpnew);
     m_tableRanking->resizeRowsToContents();
    //m_ui.tableWidgetRanking->viewport()->setGeometry(vporig);
    //m_ui.tableWidgetRanking->horizontalHeader()->setStretchLastSection(true);
    //m_ui.tableWidgetRanking->horizontalHeader()->setSectionResizeMode(QHeaderView::AdjustToContents);
    //m_ui.tableWidgetRanking->resizeColumnToContents(0);

    computeColumnWidth();

    m_tableRanking->setVisible(true);
    m_tableRanking->updateGeometry();
    m_tableRanking->repaint();
}

void Ranking::reportResults()
{
    QSqlQuery query(*m_db);
    QString qryStr;

    if (m_round == -1)
    {
        qryStr = "SELECT max(round) FROM fixture WHERE played = 1;";
        if (!query.exec(qryStr))
            qDebug() << query.lastError().text();

        if (query.first())
        {
            m_round = query.value(0).toInt();
            qDebug() << m_round;
        }
        else
            m_round++;

        qryStr = "SELECT max(round) FROM fixture;";
        query.exec(qryStr);
        if (query.first())
        {
            m_maxRounds = query.value(0).toInt();
        }
    }

    qryStr = "SELECT * FROM fixture where round = " + QString::number(m_round);
    if (!query.exec(qryStr))
        qDebug() << query.lastError().text();

    QString round = "Risultati giornata " + QString::number(m_round);
    m_label->setText(round);

    m_tableScores->setRowCount(4);//query.size());
    int i = 0;

    while(query.next())
    {
        QString team1 = Team::reducedName(query.value("team1").toString());
        QString team2 = Team::reducedName(query.value("team2").toString());
        QString result = query.value("goal1").toString() + " - " + query.value("goal2").toString();

        m_tableScores->setItem(i, 0, new QTableWidgetItem(team1));
        m_tableScores->setItem(i, 1, new QTableWidgetItem(team2));
        m_tableScores->setItem(i, 2, new QTableWidgetItem(result));

        i++;
    }
}
