#ifndef MENUCONGE_H
#define MENUCONGE_H

#include <iostream>
#include <string>
#include <vector>
#include "Conge.h"
#include "GestionnaireConges.h"
#include "GestionnaireFichiersConge.h"

using namespace std;

class MenuConge
{
    public:
        
        MenuConge(GestionnaireConges& gestionnaire, istream& entree = cin);
        virtual ~MenuConge();

        // Espace Employe : leve invalid_argument si idEmploye <= 0
        void lancerEmploye(int idEmploye);
        // Espace Responsable RH
        void lancerRH();

        // en indicateur d'affichage : [ACCEPTEE], [REFUSEE] ou [EN ATTENTE]
        static string indicateurStatut(const string& statut);

    protected:

    private:
        GestionnaireConges& gc;
       
        GestionnaireFichiersConge fichiers;
        istream& entree;

        bool lireTexte(const string& invite, string& texte);
        bool lireEntier(const string& invite, int minimum, int maximum, int& valeur);
        bool lireDate(const string& invite, string& date);
        static bool formatDateOk(const string& date);

        void afficherTableau(const vector<Conge>& liste) const;
        void afficherMenuEmploye() const;
        void afficherMenuRH() const;

        //  Espace Employe
        void deposerDemande(int idEmploye);
        void consulterHistorique(int idEmploye);

        // Espace RH 
        void traiterDemandes();
        void consulterTousLesConges();
        void bilanEmploye();
};

#endif 
