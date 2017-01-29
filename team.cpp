#include "team.h"

Team::Team(QString name, QByteArray image, QObject *parent) :
    QObject(parent),
    m_name(name),
    m_image(image)
{
    m_points = 0;
    m_won = 0;
    m_lost = 0;
    m_drawn= 0;
    m_scored = 0;
    m_conceded = 0;
    m_fantaPoints = 0.0;
    m_played = 0;
}

QString Team::reducedName(QString name)
{
    if (name.length() > 10)
    {
        QStringList splittedName = name.split(" ");

        if (splittedName[0].length() > 10)
            return splittedName[0].left(9) + ".";
        else
            return splittedName[0].trimmed();
    }
    else
        return name;
}

QIcon Team::setTeamIcon(int size, QByteArray image)
{
    QPixmap pixmap;
    if (pixmap.loadFromData(image))
    {
        pixmap = pixmap.scaled(size, size, Qt::KeepAspectRatio, Qt::FastTransformation);
    }

    QIcon icon(pixmap);

    return icon;
}
