#include "GestionnaireEmployes.h"
#include "Exceptions.h"
#include "Validation.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <utility>

namespace {

bool statutValide(const std::string& statut) {
    static const std::vector<std::string> statuts = {"Actif", "Inactif", "En conge", "Suspendu"};
    return std::find(statuts.begin(), statuts.end(), statut) != statuts.end();
}

std::string enMinuscules(std::string texte) {
    std::transform(texte.begin(), texte.end(), texte.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return texte;
}

} // namespace

GestionnaireEmployes::GestionnaireEmployes(
    const GestionnaireDepartements& gestionnaireDepartements)
    : gestionnaireDepartements_(gestionnaireDepartements) {}

std::string GestionnaireEmployes::fabriquerMatricule(int id) {
    std::ostringstream os;
    os << "EMP" << std::setfill('0') << std::setw(4) << id;
    return os.str();
}

void GestionnaireEmployes::valider(const DonneesEmploye& d) const {
    std::vector<std::string> err;

    if (!Validation::nomValide(d.nom)) err.push_back("nom invalide (2-50 lettres)");
    if (!Validation::nomValide(d.prenom)) err.push_back("prenom invalide (2-50 lettres)");
    if (!Validation::cinValide(d.cin)) err.push_back("CIN invalide (5-12 caracteres alphanumeriques)");
    if (!Validation::emailValide(d.email)) err.push_back("email invalide");
    if (!Validation::telephoneValide(d.telephone))
        err.push_back("telephone invalide (8-15 chiffres, '+' optionnel)");

    if (!statutValide(d.statut))
        err.push_back("statut invalide (Actif, Inactif, En conge, Suspendu)");

    if (!std::isfinite(d.salaireBase) || d.salaireBase < 0)
        err.push_back("salaire de base invalide (>= 0)");

    bool naissanceOk = d.dateNaissance.estValide();
    bool embaucheOk = d.dateEmbauche.estValide();
    if (!naissanceOk) err.push_back("date de naissance invalide");
    if (!embaucheOk) err.push_back("date d'embauche invalide");
    const Date aujourdHui = Date::aujourdhui();
    if (naissanceOk && aujourdHui < d.dateNaissance)
        err.push_back("date de naissance dans le futur");
    if (embaucheOk && aujourdHui < d.dateEmbauche)
        err.push_back("date d'embauche dans le futur");
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
    ++prochainId_;
    return id;
}

void GestionnaireEmployes::modifier(int id, const DonneesEmploye& d) {
    Employe& e = trouverInterne(id);
    valider(d);
    verifierCinUnique(d.cin, id);

    e.modifierInformations(d);
}
 
void GestionnaireEmployes::supprimer(int id) {
    auto it = employes_.find(id);
    if (it == employes_.end()) throw EmployeIntrouvable(id);

    employes_.erase(it);
}

std::optional<Employe> GestionnaireEmployes::rechercher(int id) const {
    auto it = employes_.find(id);
    if (it == employes_.end()) return std::nullopt;
    return it->second;
}

Employe GestionnaireEmployes::trouver(int id) const {
    auto it = employes_.find(id);
    if (it == employes_.end()) throw EmployeIntrouvable(id);
    return it->second;
}

Employe& GestionnaireEmployes::trouverInterne(int id) {
    auto it = employes_.find(id);
    if (it == employes_.end()) throw EmployeIntrouvable(id);
    return it->second;
}

std::vector<Employe> GestionnaireEmployes::rechercherParNom(const std::string& motif) const {
    std::vector<Employe> res;
    const std::string m = enMinuscules(motif);
    for (const auto& [id, e] : employes_) {
        if (enMinuscules(e.getNom()).find(m) != std::string::npos ||
            enMinuscules(e.getPrenom()).find(m) != std::string::npos)
            res.push_back(e);
    }
    return res;
}

std::optional<Employe> GestionnaireEmployes::rechercherParCin(const std::string& cin) const {
    auto it = std::find_if(employes_.begin(), employes_.end(), [&](const auto& p) {
        return p.second.getCin() == cin;
    });
    if (it == employes_.end()) return std::nullopt;
    return it->second;
}

std::optional<Employe> GestionnaireEmployes::rechercherParMatricule(
    const std::string& matricule) const {
    auto it = std::find_if(employes_.begin(), employes_.end(), [&](const auto& p) {
        return p.second.getMatricule() == matricule;
    });
    if (it == employes_.end()) return std::nullopt;
    return it->second;
}

void GestionnaireEmployes::modifierStatut(int id, const std::string& statut) {
    Employe& e = trouverInterne(id);
    if (!statutValide(statut))
        throw DonneesInvalides({"statut invalide (Actif, Inactif, En conge, Suspendu)"});

    if (e.getStatut() == statut) return;

    e.modifierStatut(statut);
}

void GestionnaireEmployes::modifierCoordonnees(int id, const std::string& email,
                                               const std::string& telephone) {
    Employe& e = trouverInterne(id);

    std::vector<std::string> erreurs;
    if (!Validation::emailValide(email)) erreurs.push_back("email invalide");
    if (!Validation::telephoneValide(telephone))
        erreurs.push_back("telephone invalide (8-15 chiffres, '+' optionnel)");
    if (!erreurs.empty()) throw DonneesInvalides(erreurs);

    if (e.getEmail() == email && e.getTelephone() == telephone) return;

    e.setEmail(email);
    e.setTelephone(telephone);
    e.ajouterHistorique("Modification des coordonnees (email / telephone)");
}

std::vector<Employe> GestionnaireEmployes::lister() const {
    std::vector<Employe> res;
    res.reserve(employes_.size());
    for (const auto& [id, e] : employes_) res.push_back(e);   // map => deja trie par id
    return res;
}

std::vector<std::string> GestionnaireEmployes::historique(int id) const {
    auto it = employes_.find(id);
    if (it == employes_.end()) throw EmployeIntrouvable(id);
    return it->second.getHistorique();
}

void GestionnaireEmployes::affecterDepartement(int idEmploye, int idDepartement) {
    Employe& e = trouverInterne(idEmploye);
    if (idDepartement < 0)
        throw DonneesInvalides({"identifiant de departement invalide"});
    if (idDepartement != 0 && !gestionnaireDepartements_.contient(idDepartement))
        throw DepartementIntrouvable(idDepartement);
    e.affecterDepartement(idDepartement);
}

std::vector<Employe> GestionnaireEmployes::employesDuDepartement(int idDepartement) const {
    std::vector<Employe> res;
    for (const auto& [id, e] : employes_)
        if (e.getIdDepartement() == idDepartement) res.push_back(e);
    return res;
}

std::size_t GestionnaireEmployes::effectifDepartement(int idDepartement) const {
    return static_cast<std::size_t>(std::count_if(
        employes_.begin(), employes_.end(), [idDepartement](const auto& entree) {
            return entree.second.getIdDepartement() == idDepartement;
        }));
}
