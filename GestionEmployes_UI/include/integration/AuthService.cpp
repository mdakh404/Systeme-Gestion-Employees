#include "AuthService.h"
#include "GestionnaireEmployes.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

AuthService::AuthService(const GestionnaireEmployes& employes, std::string cheminComptes)
    : employes_(employes), cheminComptes_(std::move(cheminComptes)) {}

bool AuthService::initialiserComptesParDefaut() const {
    if (std::filesystem::exists(cheminComptes_)) return false;
    const auto dossier = std::filesystem::path(cheminComptes_).parent_path();
    if (!dossier.empty()) std::filesystem::create_directories(dossier);

    std::ofstream out(cheminComptes_);
    if (!out) throw std::runtime_error("Impossible de creer " + cheminComptes_);
    out << "#login;motDePasse;role (administrateur | responsable_rh)\n"
        << "admin;admin;administrateur\n"
        << "rh;rh;responsable_rh\n";
    return true;
}

std::optional<SessionAuth> AuthService::authentifierCompte(const std::string& login,
                                                           const std::string& motDePasse,
                                                           const std::string& roleAttendu) const {
    std::ifstream in(cheminComptes_);
    if (!in.is_open()) return std::nullopt;

    std::string ligne;
    while (std::getline(in, ligne)) {
        if (!ligne.empty() && ligne.back() == '\r') ligne.pop_back();
        if (ligne.empty() || ligne[0] == '#') continue;
        std::istringstream ss(ligne);
        std::string l, m, r;
        std::getline(ss, l, ';');
        std::getline(ss, m, ';');
        std::getline(ss, r);
        if (l == login && m == motDePasse && r == roleAttendu)
            return SessionAuth{0, login, roleAttendu};
    }
    return std::nullopt;
}

std::optional<SessionAuth> AuthService::authentifierEmploye(const std::string& matriculeOuCin) const {
    const Employe* e = employes_.rechercherParMatricule(matriculeOuCin);
    if (!e) e = employes_.rechercherParCin(matriculeOuCin);
    if (!e) return std::nullopt;
    if (e->getStatut() == "Inactif")
        throw std::runtime_error("Compte employe desactive (statut Inactif).");
    return SessionAuth{e->getId(), e->getMatricule(), "employe"};
}
