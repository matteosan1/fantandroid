#ifndef PUNTEGGI_H
#define PUNTEGGI_H

#include <QList>
#include <QString>

class QSqlDatabase;

class Punteggi {

public:
    Punteggi();
    virtual ~Punteggi();

    QList<QString> schemi() { return m_schemiString; }
    void loadPunteggi(QSqlDatabase* db);

    float goalSubito() const { return gsu; }
    float rigoreParato() const { return rp; }
    float rigoreSbagliato() const { return rsb; }
    float goalSegnato() const { return gse; }
    float assist() const { return as; }
    float ammonizione() const { return am; }
    float espulsione() const { return es; }
    float rigoreSegnato() const { return rse; }
    float autogoal() const { return au; }

private:
    int n_portieri, n_difensori, n_centrocampisti, n_attaccanti;
    int auto_portiere, sv_portiere, sei_politico;
    int reti[10];

    QList<QString> m_schemiString;
    float gsu, rp, rsb, gse, as, am, es, rse, au;
};
#endif
