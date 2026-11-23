#pragma once
#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "Employe.h"

// Logique metier des employes, conservee en memoire.
class GestionnaireEmployes {
    std::map<int, Employe> employes_;
    int prochainId_ = 1;
    // Identifiants connus de la base (charges ou enregistres) : sert a desactiver, sans suppression
    // physique, les employes retires de la memoire.
    mutable std::set<int> idsBD_;

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

    // Persistance sur fichier
    void chargerDepuisFichier(const std::string& chemin = "data/employes.txt");
    void sauvegarderDansFichier(const std::string& chemin = "data/employes.txt") const;

    // Persistance MySQL (utilisee automatiquement par les deux fonctions ci-dessus quand la
    // connexion est active). Implementation : source/GestionnaireEmployesBD.cpp
    void chargerDepuisBD();
    void sauvegarderDansBD() const;
};
