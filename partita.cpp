#include "partita.h"

Partita::Partita()
{}

Partita::~Partita()
{}

ostream& operator<<(ostream& out, Partita x)
{
	out << x.schema << " " << x.totale << '\n';
    return out;
}

istream& operator>>(istream& in, Partita& x)
{
    in >> x.schema >> x.totale;

    return in;
}    

