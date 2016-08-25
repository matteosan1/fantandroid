#ifndef __PARTITA__
#define __PARTITA__

#include <fstream>

using namespace std;

class Partita {
 public:
  Partita();
  virtual ~Partita();

  friend istream& operator>>(istream& p, Partita& x);
  friend ostream& operator<<(ostream& p, Partita x);

  void SetPartita(float a, int b) { totale = a; schema = b; };
  void SetTotale(float a) { totale = a; };

  int GetSchema() { return schema; };
  float GetTotale() { return totale; };

 private:
  float totale;
  int schema;
  // 0) 5-4-1 1) 4-5-1 2) 5-3-2 3) 4-4-2 4) 3-5-2 5) 4-3-3 6) 3-4-3
};
#endif
