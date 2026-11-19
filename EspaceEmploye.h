#pragma once
#include "GestionnaireEmployes.h"
#include "IPersistanceDepartements.h"

class EspaceEmploye {
    GestionnaireEmployes& gestion_;
    IPersistanceDepartements& persistanceDept_;
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
    EspaceEmploye(GestionnaireEmployes& gestion, IPersistanceDepartements& persistanceDept);

    void executer();
};
