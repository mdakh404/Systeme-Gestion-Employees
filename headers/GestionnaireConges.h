#ifndef GESTIONNAIRECONGES_H
#define GESTIONNAIRECONGES_H

#include <iostream>
#include <vector>
#include <string>
#include "Conge.h"
#include "GestionnaireFichiersConge.h"

using namespace std;

struct StatistiquesConges
{
    int total = 0;
    int enAttente = 0;
    int acceptees = 0;
    int refusees = 0;
    int joursPris = 0;
    double tauxAcceptation = 0.0;
};

class GestionnaireConges
{
    public:
        static const int QUOTA_ANNUEL = 22;

        GestionnaireConges();

        // Tache 3 : volet Employe
        void deposerDemande(int idEmploye,string debut,string fin,string type);
        vector<Conge> consulterHistorique(int idEmploye) const;

        // Tache 4 : volet RH & Stats
        vector<Conge> listerDemandesEnAttente() const;
        void traiterDemande(int idConge,bool accepter);
        int dureeJours(const Conge& c) const;
        int soldeRestant(int idEmploye, int annee) const;
        int joursDansAnnee(const Conge& c, int annee) const;
        static int anneeCourante();
        vector<Conge> tousLesConges() const;
        StatistiquesConges calculerStatistiques(int idEmploye=-1) const;
        void afficherStatistiques(int idEmploye=-1) const;

        // Tache 2 : persistance
        void chargerDepuisFichier();
        void sauvegarderDansFichier() const;
        virtual ~GestionnaireConges();

    protected:

    private:
        vector<Conge> conges;
        GestionnaireFichiersConge fichiers;
        bool dateValide(string date) const;
        bool chevauchement(int idEmploye,string debut,string fin) const;
        bool chevauchementAccepte(const Conge& cible) const;
        static bool estAcceptee(const string& statut);
        static bool estEnAttente(const string& statut);
        static bool estRefusee(const string& statut);
        static long joursDepuisEpoque(const string& date);
};

#endif // GESTIONNAIRECONGES_H
