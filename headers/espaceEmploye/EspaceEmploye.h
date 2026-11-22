#pragma once
#include "GestionnaireEmployes.h"
#include "MenuDemandes.h"
#include "MenuConge.h"

class EspaceEmploye {
    GestionnaireEmployes& gestion_;
    MenuDemandes& demandes_;
    MenuConge& conges_;
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

    // Module conges
    void mesConges();

public:
    explicit EspaceEmploye(GestionnaireEmployes& gestion, MenuDemandes& demandes, MenuConge& conges);

    void executer();
};
