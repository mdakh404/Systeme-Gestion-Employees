#ifndef ASSURANCE_MALADIE_SERVICE_H
#define ASSURANCE_MALADIE_SERVICE_H

#include "Service.h"

#include <cstdint>
#include <functional>
#include <iosfwd>
#include <map>
#include <utility>
#include <vector>

class AssuranceMaladieService : public Service {
public:
    using Service::Service;
    using Montant = std::int64_t;  // En centimes : 100 centimes = 1 MAD.

    enum class Statut { EnAttente, Acceptee, Refusee };

    struct Demande {
        int id;
        int idEmploye;
        int anneeSoins;
        Montant frais;
        Montant remboursement;
        Statut statut;
    };

    void affilier(int idEmploye, int annee, int tauxPourcent,
                  Montant plafondAnnuel);
    void deposer(int idDossier, int idEmploye, int anneeSoins, Montant frais);
    Montant traiter(int idDossier, bool accepter);

    Montant plafondRestant(int idEmploye, int annee) const;
    std::vector<Demande> historique(int idEmploye) const;
    std::vector<Demande> demandesEnAttente() const;

    // --- Interface Service (obligatoire) ---
    void display() const override;
    void execute() override;                       // menu console

    // Facultatif : fonction fournie par le module Employes qui dit si un id existe.
    // Sans elle, le service n'a aucune dependance envers le module Employes.
    using VerificateurEmploye = std::function<bool(int)>;
    void setVerificateurEmploye(VerificateurEmploye v) { verificateur_ = std::move(v); }

    void sauvegarder(std::ostream& sortie) const;
    void charger(std::istream& entree);

private:
    struct Couverture {
        int taux;
        Montant plafond;
    };
    using CleCouverture = std::pair<int, int>;
    struct Donnees {
        std::map<CleCouverture, Couverture> couvertures;
        std::map<int, Demande> demandes;
        std::vector<int> ordreDecisions;
    };
    Donnees donnees;
    VerificateurEmploye verificateur_;
};

#endif