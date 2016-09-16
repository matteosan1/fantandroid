#ifndef TEAM_H
#define TEAM_H

#include <QObject>

class Team : public QObject
{
    Q_OBJECT
public:
    explicit Team(QString name, QObject *parent = 0);

    QString m_name;
    int     m_points;
    int     m_won;
    int     m_lost;
    int     m_drawn;
    int     m_scored;
    int     m_got; // ?????
    float   m_fantaPoints;
    int     m_played;
};

#endif // TEAM_H
