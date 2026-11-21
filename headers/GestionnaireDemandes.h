#pragma once
#include <cstddef>
#include <map>
#include <string>
#include <vector>
#include "DemandeAdministrative.h"

// Couche 2 (logique metier) : gestion des demandes administratives.
// Conservee en memoire, la persistance est assuree par GestionnaireFichiersDemandes.
class GestionnaireDemandes {
    std::map<int, DemandeAdministrative> demandes_;
    int prochainId_ = 1;

    void valider(const DonneesDemande& d) const;

public:
    // Depot : la demande est creee avec le statut "En attente"
    int creerDemande(const DonneesDemande& d);

    // Decision du responsable RH : "Acceptee" ou "Refusee"
    void traiterDemande(int id, const std::string& decision);

    const DemandeAdministrative* rechercher(int id) const;
    std::vector<DemandeAdministrative> lister() const;
    std::vector<DemandeAdministrative> listerParEmploye(int idEmploye) const;
    std::vector<DemandeAdministrative> listerEnAttente() const;

    std::size_t nombre() const { return demandes_.size(); }

    // Reconstitution de l'etat a partir du fichier (appelee au demarrage)
    void charger(const std::vector<DemandeAdministrative>& donnees);
};
