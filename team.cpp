#include "team.h"

Team::Team(QString name, QObject *parent) :
    QObject(parent),
    m_name(name)
{
    m_points = 0;
    m_won = 0;
    m_lost = 0;
    m_drawn= 0;
    m_scored = 0;
    m_got = 0;
    m_fantaPoints = 0.0;
    m_played = 0;
}
