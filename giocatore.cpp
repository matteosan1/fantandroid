#include "giocatore.h"

#include <iostream>

Giocatore::Giocatore()
{
    m_cognome = "";
    m_prezzo = 0;
    m_ruolo = Nullo;
    for(int i=0; i<50; ++i)
        prestazioni[i].SetNull();
}

Giocatore::Giocatore(Giocatore &g)
{
    m_cognome = g.cognome();
    m_nome = g.nome();
    m_ruolo = g.ruolo();
    m_prezzo = g.prezzo();
    m_squadra = g.squadra();
    for (int i=0; i<50; ++i)
        prestazioni[i] = g.partita(i);
}

float Giocatore::GetMedia(Punteggi* p) {
  
  int giocate = GetGiocate();
  float somma = 0.;
  
  for(int i=0; i<50; ++i)
    somma += GetVoto(i, p);
  
  if (giocate != 0)
    return somma/(float)giocate;
  else
    return 0;
}

int Giocatore::GetAmmonizioni() {
  int result = 0;
  
  for(int i=0; i<50; ++i)
    if (prestazioni[i].Sanzioni() == 1)
      result++;
  
  return result;
}

int Giocatore::GetEspulsioni() {
  int result = 0;
  
  for(int i=0; i<50; ++i)
    if (prestazioni[i].Sanzioni() == 2)
      result++;
  
  return result;
}

QString Giocatore::GetPresenze() {
  
  int result = 0, result1 = 0;
  
  for(int i=0; i<50; ++i) {
    if (prestazioni[i].Formazione() == 1)
      result++;
    
    if (prestazioni[i].Formazione() == 2)
      result1++;
  }
  
  QString s = QString("%1 (%2)").arg(result).arg(result1);
  
  return s;
}

int Giocatore::GetGiocate() {
  int giocate=0;
  
  for(int i=0; i<50; ++i)
    if (prestazioni[i].GetVoto() != 0)
      giocate++;
  
  return giocate;
}

int Giocatore::GetRigori() {
  
  int giocate=0;
  
  for(int i=0; i<50; ++i) {
    if (ruolo() == Portiere)
      giocate += prestazioni[i].GoalRigore();
    else
      giocate += prestazioni[i].RigoriParati();
  }
  
  return giocate;
}

int Giocatore::GetAutogoal() {
  int giocate=0;
  
  for(int i=0; i<50; ++i)
    giocate += prestazioni[i].Autogoal();
  
  return giocate;
}

int Giocatore::GetGoal() {
  
  int giocate=0;
  
  for(int i=0; i<50; ++i) {
    if (ruolo() == Portiere)
      giocate += prestazioni[i].GoalSubiti();
    else
      giocate += prestazioni[i].Goal();
  }
  
  return giocate;
}


void Giocatore::setGiocatore(QString a, QString b, RuoloEnum c, QString e, int f) {
  
  m_nome = a;
  m_cognome = b;
  m_ruolo = c;
  m_squadra = e;
  m_prezzo = f;
}

Giocatore::~Giocatore() {}

void Giocatore::setPartita(int giornata, int b, int *c, float v) {
  prestazioni[giornata].SetPartita(b, c, v);
  //prestazioni[giornata].SetHagiocato();
};

float Giocatore::GetVoto(int i, Punteggi* p) {
  
  float v = 0.0;
  v += prestazioni[i].GetVoto();                                    // voto
  v += (float)prestazioni[i].Autogoal() * (float)p->au;             // autogoal
  v += (float)prestazioni[i].Sanzioni() * (float)p->am * 0.5;       // ammonizioni etc.
  if (ruolo() == Portiere or ruolo() == PortiereFuoriRosa) {
    v += (float)prestazioni[i].RigoriParati() * (float)p->rp;     // rigori parati
    v += (float)prestazioni[i].GoalSubiti() * (float)p->gsu;      // goal subiti
    //v -= (float)(prestazioni[i].RigoriGetData(5) -
    //	   prestazioni[i].GetData(4));                          // rigori non parati
  }
  else {
    v += (float)prestazioni[i].GoalRigore() * (float)p->rse;      // rigori segnati
    v += (float)prestazioni[i].Goal() * (float)p->gse;            // goal
    // todo Nuova regola del +4 per i goal dei difensori
//    if (ruolo() < 2)
//      v += (float)prestazioni[i].Goal();

    v += (float)prestazioni[i].RigoriSbagliati() * (float)p->rsb; // rigori sbagliati
  }
  
  return v;
}

QTextStream& operator>>(QTextStream& in, Giocatore& x) {

    int ruolo;
    in >> x.m_nome >> x.m_cognome >> ruolo;
    in >> x.m_squadra >> x.m_prezzo;
    x.setRuolo(RuoloEnum(ruolo));

    x.m_nome = x.m_nome.replace("_", " ");
    x.m_cognome = x.m_cognome.replace("_", " ");
    x.m_squadra = x.m_squadra.replace("_", " ");

    for(int i=0; i<50; i++)
        in >> x.prestazioni[i];

    return in;
}

QTextStream& operator<<(QTextStream& out, Giocatore x) {
  
  x.m_nome = x.m_nome.replace(" ", "_");
  x.m_cognome = x.m_cognome.replace(" ", "_");
  x.m_squadra = x.m_squadra.replace(" ", "_");
  
  out << x.m_nome << " " << x.m_cognome << " " << int(x.m_ruolo) << '\n';
  out << x.m_squadra << " " << x.m_prezzo << '\n';
  for(int i=0; i<50; i++)
    out << x.prestazioni[i];
  
  return out;
}

Prestazione& Giocatore::partita(int giornata) {
  return prestazioni[giornata];
}

QString Giocatore::ruoloToString(RuoloEnum ruolo)
{
    switch(ruolo)
    {
    case Nullo:
        return QString("Nullo");
    case Portiere:
        return QString("P");
    case DifensoreTerzino:
        return QString("DT");
    case DifensoreCentrale:
        return QString("DC");
    case CentrocampistaCentrale:
        return QString("CC");
    case CentrocampistaEsterno:
        return QString("CE");
    case CentrocampistaTrequartista:
        return QString("CT");
    case AttaccanteCentrale:
        return QString("AC");
    case AttaccanteMovimento:
        return QString("AM");
    default:
        return QString("Fuori Rosa");
    }
}

RuoloEnum Giocatore::convertToRuoloEnum(const int value)
{
    switch (value)
    {
    case 10:
        return Portiere;
    case 21:
        return DifensoreTerzino;
    case 22:
        return DifensoreCentrale;
    case 32:
        return CentrocampistaEsterno;
    case 31:
        return CentrocampistaCentrale;
    case 33:
        return CentrocampistaTrequartista;
    case 41:
        return AttaccanteCentrale;
    case 42:
        return AttaccanteMovimento;
    default:
        return Nullo;
    }
}
