#include "prestazione.h"

#include <iostream>
Prestazione::Prestazione() {
  isgiocata = 0;
}

Prestazione::~Prestazione()
{}

void Prestazione::SetNull() {
    voto = 0;
    formazione = -1;
    sanzioni = 0;
    for(int i=0; i<7; ++i)
        data[i] = 0;
    isgiocata = 0;
}

void Prestazione::SetPartita(int b, int* c, float v) {
  voto = v;

  for(int i=0; i<7; i++)
    data[i] = *(c+i);

  formazione = -1;
  sanzioni = b;
}

QTextStream& operator<<(QTextStream& out, Prestazione x) {

  out << x.isgiocata << " " << x.voto << " ";
  out << x.formazione << " " << x.sanzioni << " ";
  for(int i=0; i<7; i++)
    out << x.data[i] << " ";
  out << '\n';
  return out;
}

QTextStream& operator>>(QTextStream& in, Prestazione& x) {
  in >> x.isgiocata >> x.voto;
  in >> x.formazione >> x.sanzioni;
  x.SetFormazione(-1); // todo need to be removed
  for(int i=0; i<7; i++)
    in >> x.data[i];

  return in;
}
