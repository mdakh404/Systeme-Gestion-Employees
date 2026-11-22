#pragma once
#include <string>
#include "GestionnaireDemandes.h"
#include "GestionnaireFichiersDemandes.h"

// Couche 1 (presentation) : interactions utilisateur du module
// Demandes administratives. Ne contient aucune regle metier :
// elle appelle le gestionnaire et affiche le resultat.
class MenuDemandes {
    GestionnaireDemandes& gestion_;
    GestionnaireFichiersDemandes& fichiers_;

    void sauvegarder() const;
    void afficherListe(const std::vector<DemandeAdministrative>& demandes,
                       const std::string& titre) const;

    static std::string lireLigne(const std::string& prompt);

public:
    MenuDemandes(GestionnaireDemandes& gestion, GestionnaireFichiersDemandes& fichiers);

    // Espace employe
    void deposerDemande(int idEmploye);
    void consulterMesDemandes(int idEmploye) const;

    // Espace administrateur / responsable RH
    void consulterDemandes() const;
    void traiterDemande();
};
