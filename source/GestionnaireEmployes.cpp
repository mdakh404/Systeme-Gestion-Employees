#include "GestionnaireEmployes.h"
#include "Exceptions.h"
#include "Validation.h"
#include "Database.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <utility>
#include <fstream>
#include <iostream>
#include <filesystem>

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
    Employe& e = trouver(id);
    valider(d);
    verifierCinUnique(d.cin, id);

    e.modifierInformations(d);
}
 
void GestionnaireEmployes::supprimer(int id) {
    auto it = employes_.find(id);
    if (it == employes_.end()) throw EmployeIntrouvable(id);

    employes_.erase(it);
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

const Employe* GestionnaireEmployes::rechercherParCin(const std::string& cin) const {
    auto it = std::find_if(employes_.begin(), employes_.end(), [&](const auto& p) {
        return p.second.getCin() == cin;
    });
    return it == employes_.end() ? nullptr : &it->second;
}

const Employe* GestionnaireEmployes::rechercherParMatricule(const std::string& matricule) const {
    auto it = std::find_if(employes_.begin(), employes_.end(), [&](const auto& p) {
        return p.second.getMatricule() == matricule;
    });
    return it == employes_.end() ? nullptr : &it->second;
}

void GestionnaireEmployes::modifierStatut(int id, const std::string& statut) {
    Employe& e = trouver(id);
    if (!statutValide(statut))
        throw DonneesInvalides({"statut invalide (Actif, Inactif, En conge, Suspendu)"});

    if (e.getStatut() == statut) return;

    e.modifierStatut(statut);
}

void GestionnaireEmployes::modifierCoordonnees(int id, const std::string& email,
                                               const std::string& telephone) {
    Employe& e = trouver(id);

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

const std::vector<std::string>& GestionnaireEmployes::historique(int id) const {
    auto it = employes_.find(id);
    if (it == employes_.end()) throw EmployeIntrouvable(id);
    return it->second.getHistorique();
}

void GestionnaireEmployes::affecterDepartement(int idEmploye, int idDepartement) {
    Employe& e = trouver(idEmploye);
    if (idDepartement < 0)
        throw DonneesInvalides({"identifiant de departement invalide"});
    e.affecterDepartement(idDepartement);
}

std::vector<const Employe*> GestionnaireEmployes::employesDuDepartement(int idDepartement) const {
    std::vector<const Employe*> res;
    for (const auto& [id, e] : employes_)
        if (e.getIdDepartement() == idDepartement) res.push_back(&e);
    return res;
}

std::size_t GestionnaireEmployes::effectifDepartement(int idDepartement) const {
    return static_cast<std::size_t>(std::count_if(
        employes_.begin(), employes_.end(), [idDepartement](const auto& entree) {
            return entree.second.getIdDepartement() == idDepartement;
        }));
}

void GestionnaireEmployes::chargerDepuisFichier(const std::string& chemin) {
    if (Database::instance().estConnectee()) {   // mode MySQL
        chargerDepuisBD();
        return;
    }
    std::ifstream in(chemin);
    if (!in.is_open()) return;

    employes_.clear();
    prochainId_ = 1;

    std::string ligne;
    int rejetees = 0;
    while (std::getline(in, ligne)) {
        if (ligne.empty() || ligne[0] == '#') continue;
        if (!ligne.empty() && ligne.back() == '\r') ligne.pop_back();

        std::vector<std::string> champs;
        std::string champ;
        std::istringstream ss(ligne);
        while (std::getline(ss, champ, ';')) {
            champs.push_back(champ);
        }

        if (champs.size() < 11) { ++rejetees; continue; }

        try {
            int id = std::stoi(champs[0]);
            std::string matricule = champs[1];
            DonneesEmploye d;
            d.nom = champs[2];
            d.prenom = champs[3];
            d.cin = champs[4];
            d.email = champs[5];
            d.telephone = champs[6];
            d.dateNaissance = Date::fromString(champs[7]);
            d.dateEmbauche = Date::fromString(champs[8]);
            d.statut = champs[9];
            d.salaireBase = std::stod(champs[10]);

            Employe e(id, matricule, d);
            if (champs.size() >= 12) {
                e.affecterDepartement(std::stoi(champs[11]));
            }
            if (champs.size() >= 13) {
                e.affecterPoste(std::stoi(champs[12]));
            }

            employes_.emplace(id, std::move(e));
            if (id >= prochainId_) {
                prochainId_ = id + 1;
            }
        } catch (const std::exception&) {
            ++rejetees;
            continue;
        }
    }
    if (rejetees > 0) {
        in.close();
        std::error_code ec;
        std::filesystem::copy_file(chemin, chemin + ".bak", std::filesystem::copy_options::overwrite_existing, ec);
        std::cerr << "Attention : " << rejetees << " ligne(s) invalide(s) ignoree(s) dans " << chemin
                  << " (copie de securite : " << chemin << ".bak)\n";
    }
}

void GestionnaireEmployes::sauvegarderDansFichier(const std::string& chemin) const {
    if (Database::instance().estConnectee()) {   // mode MySQL
        sauvegarderDansBD();
        return;
    }
    namespace fs = std::filesystem;
    fs::path p(chemin);
    if (p.has_parent_path() && !fs::exists(p.parent_path())) {
        fs::create_directories(p.parent_path());
    }

    std::string cheminTmp = chemin + ".tmp";
    std::ofstream out(cheminTmp);
    if (!out.is_open()) {
        throw std::runtime_error("Impossible d'ouvrir le fichier temporaire en ecriture : " + cheminTmp);
    }

    out << "#id;matricule;nom;prenom;cin;email;telephone;dateNaissance;dateEmbauche;statut;salaireBase;idDepartement;idPoste\n";
    for (const auto& [id, e] : employes_) {
        out << e.getId() << ';'
            << e.getMatricule() << ';'
            << e.getNom() << ';'
            << e.getPrenom() << ';'
            << e.getCin() << ';'
            << e.getEmail() << ';'
            << e.getTelephone() << ';'
            << e.getDateNaissance().toString() << ';'
            << e.getDateEmbauche().toString() << ';'
            << e.getStatut() << ';'
            << std::fixed << std::setprecision(2) << e.getSalaire().getSalaireBase() << std::defaultfloat << ';'
            << e.getIdDepartement() << ';'
            << e.getIdPoste() << '\n';
    }

    out.close();
    if (!out) {
        throw std::runtime_error("Erreur lors de l'ecriture du fichier : " + cheminTmp);
    }

    fs::rename(cheminTmp, chemin);
}
