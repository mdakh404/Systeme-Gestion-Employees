#pragma once
#include "GestionnaireEmployes.h"
#include "GestionnaireDepartements.h"

class EspaceEmploye {
    GestionnaireEmployes& gestion_;
    const GestionnaireDepartements& gestionnaireDepartements_;
    int idConnecte_ = 0;

    bool connecter();
    void deconnecter();

    Employe employeConnecte() const;

    void afficherProfil() const;
    void afficherInfosPersonnelles() const;
    void afficherInfosProfessionnelles() const;
    void afficherDepartement() const;
    void modifierCoordonnees();
    void menuConnecte();

public:
    EspaceEmploye(GestionnaireEmployes& gestion,
                  const GestionnaireDepartements& gestionnaireDepartements);

    void executer();
};
