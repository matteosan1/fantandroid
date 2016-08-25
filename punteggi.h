#ifndef PUNTEGGI_H
#define PUNTEGGI_H

#include <QTextStream>
#include <QList>
#include <QStringList>

#define NUMERO_SCHEMI 15

class Punteggi {

    public:
        Punteggi();
        virtual ~Punteggi();

        friend QTextStream& operator<<(QTextStream& p, Punteggi x);
        friend QTextStream& operator>>(QTextStream& p, Punteggi& x);

        int n_portieri, n_difensori, n_centrocampisti, n_attaccanti;
        int auto_portiere, sv_portiere, sei_politico;
        int reti[10];
        QList<int> m_schemi;
        QStringList m_schemiString;
        int gsu, rp, rsb, gse, as, am, es, rse, au;
};
#endif
