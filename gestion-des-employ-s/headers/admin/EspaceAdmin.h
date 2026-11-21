#ifndef ESPACE_ADMIN_H
#define ESPACE_ADMIN_H

#include "../GestionnaireEmployes.h"

class EspaceAdmin {
private:
    GestionnaireEmployes& gestionnaire;

public:
    // Constructeur
    EspaceAdmin(GestionnaireEmployes& g);

    // Menu principal
    void menuAdmin();

    // Les fonctionnalités de l'administrateur
    void ajouterEmploye();
    void modifierEmploye();
    void supprimerEmploye();
    void rechercherEmploye();
    void consulterEmploye();
    void afficherEmployes();
};

#endif
