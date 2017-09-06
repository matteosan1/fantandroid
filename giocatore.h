#ifndef __GIOCATORE__
#define __GIOCATORE__

#include "prestazione.h"
#include "punteggi.h"

#include <QString>
#include <QTextStream>

#include <vector>

using namespace std;

enum RuoloEnum
{
    Nullo = -1,
    Portiere = 10,
    DifensoreTerzino = 21,
    DifensoreCentrale = 22,
    CentrocampistaCentrale = 31,
    CentrocampistaEsterno = 32,
    CentrocampistaTrequartista = 33,
    AttaccanteCentrale = 41,
    AttaccanteMovimento = 42,
    PortiereFuoriRosa = 110,
    DifensoreTerzinoFuoriRosa = 121,
    DifensoreCentraleFuoriRosa = 122,
    CentrocampistaCentraleFuoriRosa = 131,
    CentrocampistaEsternoFuoriRosa = 132,
    CentrocampistaTrequartistaFuoriRosa = 133,
    AttaccanteCentraleFuoriRosa = 141,
    AttaccanteMovimentoFuoriRosa = 142
};

class Giocatore {

public:

    static bool PlayerComparator(Giocatore* p1, Giocatore* p2)
    {
        if (p1->ruolo() < p2->ruolo())
            return true;
        if (p1->ruolo() == p2->ruolo() and p1->media() > p2->media())
            return true;

        return false;
    }

    Giocatore();
    Giocatore(Giocatore& g);

    virtual ~Giocatore();

    static QString ruoloToString(RuoloEnum ruolo);
    static RuoloEnum convertToRuoloEnum(const int value);

    friend QTextStream& operator<<(QTextStream& p, Giocatore x);
    friend QTextStream& operator>>(QTextStream& p, Giocatore& x);

    QString nome() { return m_nome.toUpper(); }
    QString cognome() { return m_cognome.toUpper(); }
    QString nomeCompleto() {
        if (m_nome == " ")
            return m_cognome.toUpper();
        else
            //return cognome()+QString(" ")+nome().left(1)+QString(".");
            return cognome();
    };

    float GetMedia(Punteggi* p);
    float media() { return m_media; }
    int GetAmmonizioni();
    int GetEspulsioni();
    QString GetPresenze();
    int GetGiocate();
    int GetRigori();
    int GetAutogoal();
    int GetGoal();
    float GetVoto(int i, Punteggi* p);

    RuoloEnum ruolo() const { return m_ruolo; }
    QString squadra() { return m_squadra; }
    int prezzo() { return m_prezzo; }
    Prestazione& partita(int);
    //int HaGiocato(int g) { return GetPartita(g).HaGiocato(); };

    void setGiocatore(QString a, QString b, RuoloEnum ruolo, QString e, int f);
    void setNome(QString a) { m_nome = a; }
    void setCognome(QString a) { m_cognome = a; }
    void setRuolo(const RuoloEnum ruolo) { m_ruolo = ruolo; }
    void setRuolo(const int ruolo) { m_ruolo = convertToRuoloEnum(ruolo); }
    void setPartita(int giornata, int b, int *c, float v);
    void setSquadra(QString a) { m_squadra = a; }
    void setPrezzo(int a) { m_prezzo = a; }
    void setMedia(const float& m) { m_media = m; }

private:
    QString     m_nome;
    QString     m_cognome;
    RuoloEnum   m_ruolo;
    int         m_prezzo;
    QString     m_squadra;
    Prestazione prestazioni[50];
    float       m_media;
};
#endif
