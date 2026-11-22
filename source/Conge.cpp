#include "Conge.h"
using namespace std;

Conge::Conge()
    : id(0),
      idEmploye(0),
      dateDebut(""),
      dateFin(""),
      type(""),
      statut("En attente") {
}

Conge::Conge(int id,int idEmploye,string dateDebut,string dateFin,string type,string statut)
    : id(id),
      idEmploye(idEmploye),
      dateDebut(dateDebut),
      dateFin(dateFin),
      type(type),
      statut(statut) {
}


int Conge::Getid() const {
    return id;
}

int Conge::GetidEmploye() const {
    return idEmploye;
}

string Conge::GetdateDebut() const {
    return dateDebut;
}

string Conge::GetdateFin() const {
    return dateFin;
}

string Conge::Gettype() const {
    return type;
}

string Conge::Getstatut() const {
    return statut;
}

ostream& operator<<(ostream& os, const Conge& c) {
    os << "ID Conge       : " << c.id << endl;
    os << "ID Employe     : " << c.idEmploye << endl;
    os << "Date debut     : " << c.dateDebut << endl;
    os << "Date fin       : " << c.dateFin << endl;
    os << "Type           : " << c.type << endl;
    os << "Statut         : " << c.statut << endl;

    return os;
}

bool Conge::operator==(const Conge& autre){
    return id == autre.id;
}

Conge::~Conge()
{
    //dtor
}
