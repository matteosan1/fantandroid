#ifndef __PRESTAZIONE__
#define __PRESTAZIONE__

#include <QTextStream>

using namespace std;

class Prestazione {
 public:
  Prestazione();
  virtual ~Prestazione();

  friend QTextStream& operator>>(QTextStream& p, Prestazione& x);
  friend QTextStream& operator<<(QTextStream& p, Prestazione x);

  void SetPartita(int b, int* c, float v);
  //void SetData(int i, int j) { data[i] = j; }
  void SetVoto(float v) { voto = v; }
  void SetFormazione (int a) { formazione = a; }
  void SetHaGiocato() { isgiocata = 1; }
  void SetNonHaGiocato() { isgiocata = 0; }

  bool isAmmonizione() { return (sanzioni == 1); }
  bool isEspulsione() { return (sanzioni == 2); }
  bool isGoal() { return ((data[0] != 0) || (data[1] != 0) || (data[2] != 0)); }
  int Goal() { return data[0]; }
  int GoalSubiti() { return data[2]; }
  int GoalRigore() { return data[1]; }
  int RigoriSbagliati() { return data[4]; }
  int Autogoal() { return data[5]; }
  int RigoriParati() { return data[3]; }
  int Assist() { return data[6]; }
  //void GetPartita(int a, float b, int* c);
  //int GetData(int i) { return data[i]; }
  int HaGiocato() { return isgiocata; }
  //int GetGiornata() { return giornata; }
  float GetVoto() { return voto; }
  int Sanzioni() { return sanzioni; }
  int Formazione() { return formazione; }

  void SetNull();

 private:
  int isgiocata;
  float voto;
  int data[7];
  int formazione;
  int sanzioni;

  // sanzioni 0 niente, 1 ammonizione, 2 espulsione
};
#endif
