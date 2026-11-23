#include "GestionnaireDepartements.h"
#include "GestionnaireEmployes.h"
#include "Validation.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

void GestionnaireDepartements::verifierNomUnique(const std::string& nom, int idIgnore) const {
    const auto existe = std::find_if(
        departements_.begin(), departements_.end(),
        [&](const auto& entree) {
            return entree.first != idIgnore && entree.second.getNom() == nom;
        });
    if (existe != departements_.end()) throw DepartementExistant(nom);
}

int GestionnaireDepartements::ajouter(const std::string& nom, const std::string& description) {
    verifierNomUnique(nom);
    const int id = prochainId_;
    departements_.emplace(id, Departement(id, nom, description)); // valide nom/description
    ++prochainId_;
    return id;
}

void GestionnaireDepartements::modifier(int id, const std::string& nom, const std::string& description) {
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
    for (const auto& entree : departements_) resultats.push_back(entree.second);
    return resultats;
}

bool GestionnaireDepartements::contient(int id) const {
    return departements_.find(id) != departements_.end();
}

void GestionnaireDepartements::chargerDepuisFichier(const std::string& chemin) {
    std::ifstream in(chemin);
    if (!in.is_open()) return; // premier lancement : aucun departement

    departements_.clear();
    prochainId_ = 1;
    std::string ligne;
    while (std::getline(in, ligne)) {
        if (!ligne.empty() && ligne.back() == '\r') ligne.pop_back();
        if (ligne.empty() || ligne[0] == '#') continue;
        std::istringstream ss(ligne);
        std::string id, nom, description;
        std::getline(ss, id, ';');
        std::getline(ss, nom, ';');
        std::getline(ss, description);
        try {
            const int n = std::stoi(id);
            departements_.emplace(n, Departement(n, nom, description));
            prochainId_ = std::max(prochainId_, n + 1);
        } catch (const std::exception&) {
            // ligne corrompue : ignoree
        }
    }
}

void GestionnaireDepartements::sauvegarderDansFichier(const std::string& chemin) const {
    const auto dossier = std::filesystem::path(chemin).parent_path();
    if (!dossier.empty()) std::filesystem::create_directories(dossier);

    const std::string temporaire = chemin + ".tmp";
    {
        std::ofstream out(temporaire, std::ios::trunc);
        if (!out) throw std::runtime_error("Impossible d'ecrire " + chemin);
        out << "#id;nom;description\n";
        for (const auto& entree : departements_) {
            const Departement& d = entree.second;
            out << d.getId() << ';' << d.getNom() << ';' << d.getDescription() << '\n';
        }
    }
    std::filesystem::rename(temporaire, chemin); // remplacement atomique
}
