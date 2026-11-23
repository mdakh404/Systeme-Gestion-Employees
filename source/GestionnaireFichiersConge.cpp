 #include "GestionnaireFichiersConge.h"
#include "DbPersistence.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;

GestionnaireFichiersConge::GestionnaireFichiersConge(const std::string& chemin, char sep)
    : cheminFichier(chemin), separateur(sep) {
    assurerExistenceDossierEtFichier();
}

void GestionnaireFichiersConge::assurerExistenceDossierEtFichier() const {
    fs::path p(cheminFichier);
    if (p.has_parent_path() && !fs::exists(p.parent_path())) {
        fs::create_directories(p.parent_path());
    }
    if (!fs::exists(p)) {
        std::ofstream fichierCreation(cheminFichier);
        if (!fichierCreation.is_open()) {
            throw std::runtime_error("Impossible de creer le fichier : " + cheminFichier);
        }
        fichierCreation.close();
    }
}

std::string GestionnaireFichiersConge::serialiserConge(const Conge& c) const {
    std::ostringstream oss;
    oss << c.Getid() << separateur
        << c.GetidEmploye() << separateur
        << c.GetdateDebut() << separateur
        << c.GetdateFin() << separateur
        << c.Gettype() << separateur
        << c.Getstatut();
    return oss.str();
}

Conge GestionnaireFichiersConge::deserialiserLigne(const std::string& ligne) const {
    std::stringstream ss(ligne);
    std::string idStr, idEmpStr, dateDebut, dateFin, type, statut;

    if (!std::getline(ss, idStr, separateur) ||
        !std::getline(ss, idEmpStr, separateur) ||
        !std::getline(ss, dateDebut, separateur) ||
        !std::getline(ss, dateFin, separateur) ||
        !std::getline(ss, type, separateur) ||
        !std::getline(ss, statut, separateur)) {
        throw std::runtime_error("Ligne corrompue dans le fichier des conges : " + ligne);
    }

    int id = 0, idEmploye = 0;
    try {
        id = std::stoi(idStr);
        idEmploye = std::stoi(idEmpStr);
    }
    catch (const std::exception&) {
        throw std::runtime_error("Ligne corrompue dans le fichier des conges : " + ligne);
    }
    return Conge(id, idEmploye, dateDebut, dateFin, type, statut);
}

std::vector<Conge> GestionnaireFichiersConge::chargerTousLesConges() const {
    if (Database::instance().estConnectee()) return db::chargerConges();   // mode MySQL
    assurerExistenceDossierEtFichier();
    std::ifstream fichier(cheminFichier);
    if (!fichier.is_open()) {
        throw std::runtime_error("Erreur d'ouverture en lecture : " + cheminFichier);
    }

    std::vector<Conge> liste;
    std::string ligne;
    while (std::getline(fichier, ligne)) {
        if (!ligne.empty() && ligne.back() == '\r') ligne.pop_back();
        if (ligne.empty()) continue; // Ignorer les lignes vides
        liste.push_back(deserialiserLigne(ligne));
    }
    fichier.close();
    return liste;
}

void GestionnaireFichiersConge::sauvegarderTousLesConges(const std::vector<Conge>& conges) const {
    if (Database::instance().estConnectee()) {   // mode MySQL
        db::sauvegarderConges(conges);
        return;
    }
    assurerExistenceDossierEtFichier();
    std::ofstream fichier(cheminFichier, std::ios::trunc);
    if (!fichier.is_open()) {
        throw std::runtime_error("Erreur d'ouverture en ecriture : " + cheminFichier);
    }

    for (const auto& c : conges) {
        fichier << serialiserConge(c) << "\n";
    }
    fichier.close();
}

void GestionnaireFichiersConge::ajouterConge(const Conge& nouveauConge) const {
    if (Database::instance().estConnectee()) {   // mode MySQL
        db::ajouterConge(nouveauConge);
        return;
    }
    assurerExistenceDossierEtFichier();
    std::ofstream fichier(cheminFichier, std::ios::app);
    if (!fichier.is_open()) {
        throw std::runtime_error("Erreur d'ouverture en ajout : " + cheminFichier);
    }
    fichier << serialiserConge(nouveauConge) << "\n";
    fichier.close();
}

bool GestionnaireFichiersConge::mettreAJourStatut(int idConge, const std::string& nouveauStatut) const {
    if (Database::instance().estConnectee()) return db::mettreAJourStatutConge(idConge, nouveauStatut);
    std::vector<Conge> tous = chargerTousLesConges();
    bool trouve = false;

    for (auto& c : tous) {
        if (c.Getid() == idConge) {
            c.Setstatut(nouveauStatut);
            trouve = true;
            break;
        }
    }

    if (trouve) {
        sauvegarderTousLesConges(tous);
    }
    return trouve;
}

std::vector<Conge> GestionnaireFichiersConge::rechercherParEmploye(int idEmploye) const {
    std::vector<Conge> tous = chargerTousLesConges();
    std::vector<Conge> resultat;

    for (const auto& c : tous) {
        if (c.GetidEmploye() == idEmploye) {
            resultat.push_back(c);
        }
    }
    return resultat;
}
