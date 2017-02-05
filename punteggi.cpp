#include "punteggi.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <QDebug>

Punteggi::Punteggi() {

    n_portieri = 0;
    n_difensori = 0;
    n_centrocampisti = 0;
    n_attaccanti = 0;
    auto_portiere = 0;
    sv_portiere = 0;
    sei_politico = 0;
    for(int i=0; i<10; ++i)
        reti[i] = 0;

    gsu = 0;
    rp = 0;
    rsb = 0;
    gse = 0;
    as = 0;
    am = 0;
    es = 0;
    rse = 0;
    au = 0;
}

Punteggi::~Punteggi()
{}

void Punteggi::loadPunteggi(QSqlDatabase* db)
{
//    m_schemiString << "-" << "5-4-1" << "5-3-1-1" << "5-3-2" << "4-5-1"
//                   << "4-4-2" << "4-3-2-1" << "4-3-1-2" << "4-3-3"
//                   << "4-2-3-1" << "4-2-1-3" << "4-2-4"
//                   << "3-5-2" << "3-4-1-2" << "3-4-3";


    QString qryStr = "select * from modules;";
    QSqlQuery query(*db);

    if (!query.exec(qryStr)) {
        qDebug() << query.lastError();
    }
    else {
        while(query.next())
        {
            QString module = query.value(0).toString();
            m_schemiString.append(module);
        }
    }
}

