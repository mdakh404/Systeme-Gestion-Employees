#ifndef PRIMESERVICE_H
#define PRIMESERVICE_H

#include <string>
#include "Service.h"
#include "Employe.h"
#include "GestionnaireEmployes.h"

class PrimeService : public Service {
public:
    struct Bonification {
        int idEmploye = 0;
        std::string matricule;
        double salaireBase = 0.0;
        int anneesAnciennete = 0;
        double taux = 0.0;
        double montantPrime = 0.0;
    };

    explicit PrimeService(
        int id,
        GestionnaireEmployes& gestion,
        const std::string& name = "Bonification des primes",
        const std::string& description = "Calcul et suivi des primes d'anciennete",
        double tauxParAnnee = 0.01,
        double tauxMaximum = 0.20,
        int ancienneteMinimum = 1);

    double calculerTaux(int anneesAnciennete) const;
    double calculerMontant(const Employe& employe) const;
    Bonification bonifier(const Employe& employe);

    double getTauxParAnnee() const { return tauxParAnnee_; }
    double getTauxMaximum() const { return tauxMaximum_; }
    int getAncienneteMinimum() const { return ancienneteMinimum_; }

    void setTauxParAnnee(double tauxParAnnee);
    void setTauxMaximum(double tauxMaximum);
    void setAncienneteMinimum(int ancienneteMinimum);

    void display() const override;
    void execute() override;

private:
    GestionnaireEmployes& gestion_;
    double tauxParAnnee_;
    double tauxMaximum_;
    int ancienneteMinimum_;

    static double arrondirCentimes(double montant);
    static void validerTaux(double taux);
};

#endif
