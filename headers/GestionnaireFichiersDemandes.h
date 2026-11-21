#pragma once
#include <string>
#include <vector>
#include "DemandeAdministrative.h"

// Couche 4 (acces aux donnees) : persistance des demandes administratives.
// Format : un enregistrement par ligne, champs separes par ';'
//   id;idEmploye;type;dateDepot;statut;description
class GestionnaireFichiersDemandes {
    std::string chemin_;

public:
    explicit GestionnaireFichiersDemandes(const std::string& chemin = "data/demandes.txt");

    // Retourne une liste vide si le fichier n'existe pas encore
    std::vector<DemandeAdministrative> lireDemandes() const;
    // Ecrit la totalite des demandes (le dossier data/ est cree si absent)
    void ecrireDemandes(const std::vector<DemandeAdministrative>& demandes) const;

    const std::string& getChemin() const { return chemin_; }
};
