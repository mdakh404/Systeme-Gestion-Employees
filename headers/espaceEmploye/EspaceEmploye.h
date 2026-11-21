#pragma once
#include "GestionnaireEmployes.h"
#include "MenuDemandes.h"

class EspaceEmploye {
    GestionnaireEmployes& gestion_;
    MenuDemandes& demandes_;
    int idConnecte_ = 0;

    bool connecter();
    void deconnecter();

    const Employe& employeConnecte() const;

    void afficherProfil() const;
    void afficherInfosPersonnelles() const;
    void afficherInfosProfessionnelles() const;
    void afficherDepartement() const;
    void modifierCoordonnees();
    void menuConnecte();

    // Module demandes administratives
    void deposerDemande();
    void mesDemandes() const;

public:
    explicit EspaceEmploye(GestionnaireEmployes& gestion, MenuDemandes& demandes);

    void executer();
};
