#pragma once
#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include "Employe.h"
#include "GestionnaireDepartements.h"

// Logique metier des employes, conservee en memoire.
class GestionnaireEmployes {
    std::map<int, Employe> employes_;
    const GestionnaireDepartements& gestionnaireDepartements_;
    int prochainId_ = 1;

    static constexpr int AGE_MIN_EMBAUCHE = 18;   // regle metier modifiable

    void valider(const DonneesEmploye& d) const;
    void verifierCinUnique(const std::string& cin, int idIgnore = 0) const;
    Employe& trouverInterne(int id);
    static std::string fabriquerMatricule(int id);

public:
    explicit GestionnaireEmployes(const GestionnaireDepartements& gestionnaireDepartements);

    int ajouter(const DonneesEmploye& d);
    void modifier(int id, const DonneesEmploye& d);
    void supprimer(int id);

    std::optional<Employe> rechercher(int id) const;
    Employe trouver(int id) const;
    std::vector<Employe> rechercherParNom(const std::string& motif) const;
    std::optional<Employe> rechercherParCin(const std::string& cin) const;
    std::optional<Employe> rechercherParMatricule(const std::string& matricule) const;
    void modifierStatut(int id, const std::string& statut);
    void modifierCoordonnees(int id, const std::string& email, const std::string& telephone);

    std::vector<Employe> lister() const;
    std::vector<std::string> historique(int id) const;
    std::size_t nombre() const { return employes_.size(); }

    // L'identifiant 0 signifie que l'employe n'est pas affecte.
    void affecterDepartement(int idEmploye, int idDepartement);
    std::vector<Employe> employesDuDepartement(int idDepartement) const;
    std::size_t effectifDepartement(int idDepartement) const;
};
