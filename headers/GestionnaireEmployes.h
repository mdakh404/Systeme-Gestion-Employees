#pragma once
#include <cstddef>
#include <map>
#include <string>
#include <vector>
#include "Employe.h"

// Logique metier des employes, conservee en memoire.
class GestionnaireEmployes {
    std::map<int, Employe> employes_;
    int prochainId_ = 1;

    static constexpr int AGE_MIN_EMBAUCHE = 18;   // regle metier modifiable

    void valider(const DonneesEmploye& d) const;
    void verifierCinUnique(const std::string& cin, int idIgnore = 0) const;
    static std::string fabriquerMatricule(int id);

public:
    int ajouter(const DonneesEmploye& d);
    void modifier(int id, const DonneesEmploye& d);
    void supprimer(int id);

    const Employe* rechercher(int id) const;
    Employe& trouver(int id);
    std::vector<const Employe*> rechercherParNom(const std::string& motif) const;
    const Employe* rechercherParCin(const std::string& cin) const;
    const Employe* rechercherParMatricule(const std::string& matricule) const;
    void modifierStatut(int id, const std::string& statut);
    void modifierCoordonnees(int id, const std::string& email, const std::string& telephone);

    std::vector<Employe> lister() const;
    const std::vector<std::string>& historique(int id) const;
    std::size_t nombre() const { return employes_.size(); }

    // L'identifiant 0 signifie que l'employe n'est pas affecte.
    void affecterDepartement(int idEmploye, int idDepartement);
    std::vector<const Employe*> employesDuDepartement(int idDepartement) const;
    std::size_t effectifDepartement(int idDepartement) const;
};
