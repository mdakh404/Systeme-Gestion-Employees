#pragma once
#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include "Departement.h"

class GestionnaireEmployes;

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
};
