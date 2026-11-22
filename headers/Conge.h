#ifndef CONGE_H
#define CONGE_H

#include <string>
#include <iostream>
using namespace std;

class Conge
{
    public:
        Conge();
        Conge(int id,int idEmploye,string dateDebut,string dateFin,string type,string statut="En attente");
        virtual ~Conge();

        int Getid() const;
        int GetidEmploye() const;
        string GetdateDebut() const;
        string GetdateFin() const;
        string Gettype() const;
        string Getstatut() const;
        void SetdateDebut(string val) { dateDebut = val; }
        void SetdateFin(string val) { dateFin = val; }
        void Settype(string val) { type = val; }
        void Setstatut(string val) { statut = val; }
        friend ostream& operator<<(ostream& os,const Conge& c);
        bool operator==(const Conge& autre);

    protected:

    private:
        int id;
        int idEmploye;
        string dateDebut;
        string dateFin;
        string type;
        string statut;
};

#endif // CONGE_H
