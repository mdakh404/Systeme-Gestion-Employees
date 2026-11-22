#include "GestionnaireDepartements.h"
#include "Exceptions.h"
#include "GestionnaireEmployes.h"
#include <algorithm>

void GestionnaireDepartements::verifierNomUnique(const std::string& nom, int idIgnore) const {
    const auto existe = std::find_if(
        departements_.begin(), departements_.end(),
        [&](const auto& entree) {
            return entree.first != idIgnore && entree.second.getNom() == nom;
        });
    if (existe != departements_.end()) throw DepartementExistant(nom);
}

int GestionnaireDepartements::ajouter(const std::string& nom,
                                      const std::string& description) {
    verifierNomUnique(nom);
    const int id = prochainId_;
    departements_.emplace(id, Departement(id, nom, description));
    ++prochainId_;
    return id;
}

void GestionnaireDepartements::modifier(int id, const std::string& nom,
                                        const std::string& description) {
    auto it = departements_.find(id);
    if (it == departements_.end()) throw DepartementIntrouvable(id);

    verifierNomUnique(nom, id);
    it->second = Departement(id, nom, description);
}

void GestionnaireDepartements::supprimer(int id, const GestionnaireEmployes& employes) {
    auto it = departements_.find(id);
    if (it == departements_.end()) throw DepartementIntrouvable(id);
    if (employes.effectifDepartement(id) != 0) throw DepartementUtilise(id);
    departements_.erase(it);
}

std::optional<Departement> GestionnaireDepartements::rechercher(int id) const {
    const auto it = departements_.find(id);
    if (it == departements_.end()) return std::nullopt;
    return it->second;
}

Departement GestionnaireDepartements::trouver(int id) const {
    const auto it = departements_.find(id);
    if (it == departements_.end()) throw DepartementIntrouvable(id);
    return it->second;
}

std::vector<Departement> GestionnaireDepartements::lister() const {
    std::vector<Departement> resultats;
    resultats.reserve(departements_.size());
    for (const auto& [id, departement] : departements_) resultats.push_back(departement);
    return resultats;
}

bool GestionnaireDepartements::contient(int id) const {
    return departements_.find(id) != departements_.end();
}
