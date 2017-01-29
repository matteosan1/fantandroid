#ifndef TEAM_H
#define TEAM_H

#include <QObject>
#include <QByteArray>
#include <QIcon>

class Team : public QObject
{
    Q_OBJECT
public:
    explicit Team(QString name, QByteArray image, QObject *parent = 0);

    static QIcon setTeamIcon(int size, QByteArray image);
    int diff() { return m_scored - m_conceded; }
    static QString reducedName(QString name);

    QString    m_name;
    int        m_points;
    int        m_won;
    int        m_lost;
    int        m_drawn;
    int        m_scored;
    int        m_conceded;
    float      m_fantaPoints;
    int        m_played;
    QByteArray m_image;
};

#endif // TEAM_H
