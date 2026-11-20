#pragma once
#include "GestionnaireEmployes.h"

class EspaceEmploye {
    GestionnaireEmployes& gestion_;
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

public:
    explicit EspaceEmploye(GestionnaireEmployes& gestion);

    void executer();
};
