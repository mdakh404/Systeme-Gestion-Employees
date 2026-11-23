#pragma once
// ---------------------------------------------------------------------------
// GestionnaireDepartements — COPIE TEMPORAIRE
//
// Cette classe a ete ecrite par l'equipe back-end dans la branche
// `gestion_employee` du depot Systeme-Gestion-Employees, mais elle n'a pas
// encore ete fusionnee dans `main`. L'UI en a besoin (menu Departements,
// rapports), on la garde donc ici, a l'identique, avec en plus une
// persistance fichier (data/departements.txt).
//
// => Quand l'equipe back-end l'integre dans `main`, supprimer ces deux
//    fichiers et inclure "GestionnaireDepartements.h" du back-end.
// ---------------------------------------------------------------------------
#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "Departement.h"
#include "Exceptions.h"

class GestionnaireEmployes;

class DepartementIntrouvable : public ExceptionEmploye {
public:
    explicit DepartementIntrouvable(int id)
        : ExceptionEmploye("Departement introuvable (id = " + std::to_string(id) + ")") {}
};

class DepartementExistant : public ExceptionEmploye {
public:
    explicit DepartementExistant(const std::string& nom)
        : ExceptionEmploye("Un departement porte deja ce nom: " + nom) {}
};

class DepartementUtilise : public ExceptionEmploye {
public:
    explicit DepartementUtilise(int id)
        : ExceptionEmploye("Departement " + std::to_string(id) +
                           " non vide : reaffectez d'abord ses employes") {}
};

class GestionnaireDepartements {
    std::map<int, Departement> departements_;
    int prochainId_ = 1;

    void verifierNomUnique(const std::string& nom, int idIgnore = 0) const;

public:
    int ajouter(const std::string& nom, const std::string& description = "");
    void modifier(int id, const std::string& nom, const std::string& description = "");
    void supprimer(int id, const GestionnaireEmployes& employes);

    std::optional<Departement> rechercher(int id) const;
    Departement trouver(int id) const;
    std::vector<Departement> lister() const;
    bool contient(int id) const;
    std::size_t nombre() const { return departements_.size(); }

    // Persistance (format : id;nom;description)
    void chargerDepuisFichier(const std::string& chemin = "data/departements.txt");
    void sauvegarderDansFichier(const std::string& chemin = "data/departements.txt") const;
};
