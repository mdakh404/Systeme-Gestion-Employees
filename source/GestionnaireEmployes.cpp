#include "GestionnaireEmployes.h"
#include "Validation.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>

using namespace Validation;

GestionnaireEmployes::GestionnaireEmployes(IPersistanceEmployes& p) : persistance_(p) {}

void GestionnaireEmployes::charger() {
    employes_.clear();
    prochainId_ = 1;
    for (auto& e : persistance_.lireEmployes()) {
        int id = e.getId();
        prochainId_ = std::max(prochainId_, id + 1);
        employes_.emplace(id, std::move(e));
    }
}

void GestionnaireEmployes::sauvegarder() {
    persistance_.sauvegarderEmployes(lister());
}

std::string GestionnaireEmployes::fabriquerMatricule(int id) {
    std::ostringstream os;
    os << "EMP" << std::setfill('0') << std::setw(4) << id;
    return os.str();
}

void GestionnaireEmployes::valider(const DonneesEmploye& d) const {
    std::vector<std::string> err;

    if (!nomValide(d.nom)) err.push_back("nom invalide (2-50 lettres)");
    if (!nomValide(d.prenom)) err.push_back("prenom invalide (2-50 lettres)");
    if (!cinValide(d.cin)) err.push_back("CIN invalide (5-12 caracteres alphanumeriques)");
    if (!emailValide(d.email)) err.push_back("email invalide");
    if (!telephoneValide(d.telephone)) err.push_back("telephone invalide (8-15 chiffres, '+' optionnel)");

    static const std::vector<std::string> statuts = {"Actif", "Inactif", "En conge", "Suspendu"};
    if (std::find(statuts.begin(), statuts.end(), d.statut) == statuts.end())
        err.push_back("statut invalide (Actif, Inactif, En conge, Suspendu)");

    if (!std::isfinite(d.salaireBase) || d.salaireBase < 0)
        err.push_back("salaire de base invalide (>= 0)");

    bool naissanceOk = d.dateNaissance.estValide();
    bool embaucheOk = d.dateEmbauche.estValide();
    if (!naissanceOk) err.push_back("date de naissance invalide");
    if (!embaucheOk) err.push_back("date d'embauche invalide");
    // anneesEntre() renvoie 0 si l'embauche precede la naissance => refuse aussi
    if (naissanceOk && embaucheOk &&
        Date::anneesEntre(d.dateNaissance, d.dateEmbauche) < AGE_MIN_EMBAUCHE)
        err.push_back("age minimum a l'embauche: " + std::to_string(AGE_MIN_EMBAUCHE) + " ans");

    if (!err.empty()) throw DonneesInvalides(err);
}

void GestionnaireEmployes::verifierCinUnique(const std::string& cin, int idIgnore) const {
    auto it = std::find_if(employes_.begin(), employes_.end(), [&](const auto& p) {
        return p.first != idIgnore && p.second.getCin() == cin;
    });
    if (it != employes_.end()) throw EmployeExistant(cin);
}

int GestionnaireEmployes::ajouter(const DonneesEmploye& d) {
    valider(d);
    verifierCinUnique(d.cin);

    int id = prochainId_;
    Employe e(id, fabriquerMatricule(id), d);
    e.ajouterHistorique("Creation de l'employe");

    employes_.emplace(id, std::move(e));
    try {
        sauvegarder();
    } catch (...) {
        employes_.erase(id);          // annule l'ajout si la sauvegarde echoue
        throw;
    }
    ++prochainId_;
    return id;
}

void GestionnaireEmployes::modifier(int id, const DonneesEmploye& d) {
    Employe& e = trouver(id);
    valider(d);
    verifierCinUnique(d.cin, id);

    Employe copie = e;                // sauvegarde pour rollback
    e.modifierInformations(d);
    try {
        sauvegarder();
    } catch (...) {
        e = copie;
        throw;
    }
}

void GestionnaireEmployes::supprimer(int id) {
    auto it = employes_.find(id);
    if (it == employes_.end()) throw EmployeIntrouvable(id);

    Employe copie = it->second;
    employes_.erase(it);
    try {
        sauvegarder();
    } catch (...) {
        employes_.emplace(id, std::move(copie));
        throw;
    }
}

const Employe* GestionnaireEmployes::rechercher(int id) const {
    auto it = employes_.find(id);
    return it == employes_.end() ? nullptr : &it->second;
}

Employe& GestionnaireEmployes::trouver(int id) {
    auto it = employes_.find(id);
    if (it == employes_.end()) throw EmployeIntrouvable(id);
    return it->second;
}

static std::string enMinuscules(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

const Employe* GestionnaireEmployes::rechercherParMatricule(const std::string& matricule) const {
    auto it = std::find_if(employes_.begin(), employes_.end(), [&](const auto& p) {
        return p.second.getMatricule() == matricule;
    });
    return it == employes_.end() ? nullptr : &it->second;
}

const Employe* GestionnaireEmployes::rechercherParCin(const std::string& cin) const {
    auto it = std::find_if(employes_.begin(), employes_.end(), [&](const auto& p) {
        return p.second.getCin() == cin;
    });
    return it == employes_.end() ? nullptr : &it->second;
}

void GestionnaireEmployes::modifierCoordonnees(int id, const std::string& email,
                                              const std::string& telephone) {
    Employe& e = trouver(id);

    std::vector<std::string> err;
    if (!emailValide(email)) err.push_back("email invalide");
    if (!telephoneValide(telephone)) err.push_back("telephone invalide (8-15 chiffres, '+' optionnel)");
    if (!err.empty()) throw DonneesInvalides(err);

    Employe copie = e;
    e.setEmail(email);
    e.setTelephone(telephone);
    e.ajouterHistorique("Modification des coordonnees (email / telephone)");
    try {
        sauvegarder();
    } catch (...) {
        e = copie;
        throw;
    }
}

std::vector<const Employe*> GestionnaireEmployes::rechercherParNom(const std::string& motif) const {
    std::vector<const Employe*> res;
    const std::string m = enMinuscules(motif);
    for (const auto& [id, e] : employes_) {
        if (enMinuscules(e.getNom()).find(m) != std::string::npos ||
            enMinuscules(e.getPrenom()).find(m) != std::string::npos)
            res.push_back(&e);
    }
    return res;
}

std::vector<Employe> GestionnaireEmployes::lister() const {
    std::vector<Employe> res;
    res.reserve(employes_.size());
    for (const auto& [id, e] : employes_) res.push_back(e);   // map => deja trie par id
    return res;
}

const std::vector<std::string>& GestionnaireEmployes::historique(int id) const {
    auto it = employes_.find(id);
    if (it == employes_.end()) throw EmployeIntrouvable(id);
    return it->second.getHistorique();
}

void GestionnaireEmployes::affecterDepartement(int idEmploye, int idDepartement) {
    Employe& e = trouver(idEmploye);
    Employe copie = e;                // rollback si la sauvegarde echoue
    e.affecterDepartement(idDepartement);
    try {
        sauvegarder();
    } catch (...) {
        e = copie;
        throw;
    }
}

std::vector<const Employe*> GestionnaireEmployes::employesDuDepartement(int idDepartement) const {
    std::vector<const Employe*> res;
    for (const auto& [id, e] : employes_)
        if (e.getIdDepartement() == idDepartement) res.push_back(&e);
    return res;
}

std::size_t GestionnaireEmployes::effectifDepartement(int idDepartement) const {
    return employesDuDepartement(idDepartement).size();
}
