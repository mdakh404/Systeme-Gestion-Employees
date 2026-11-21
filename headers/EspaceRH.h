#pragma once
#include "GestionnaireEmployes.h"
#include "MenuDemandes.h"

// Espace Responsable RH - couche 1 (presentation).
// Seule fonctionnalite actuellement disponible : traiter les demandes
// administratives. Les autres fonctionnalites RH (employes, departements,
// postes...) seront ajoutees par un autre developpeur.
class EspaceRH {
    GestionnaireEmployes& gestion_;   // conserve pour les futurs modules RH
    MenuDemandes& demandes_;

    void menuConnecte();

public:
    explicit EspaceRH(GestionnaireEmployes& gestion, MenuDemandes& demandes);

    void executer();
};
