#include "GestionnaireFichiersDemandes.h"
#include "ExceptionsDemandes.h"
#include "DbPersistence.h"
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <direct.h>
#else
#include <filesystem>
#endif

namespace {

// Decoupe une ligne en champs separes par ';' (dernier champ = description)
std::vector<std::string> decouper(const std::string& ligne, char sep) {
    std::vector<std::string> champs;
    std::string champ;
    std::istringstream is(ligne);
    while (std::getline(is, champ, sep)) champs.push_back(champ);
    return champs;
}

} // namespace

GestionnaireFichiersDemandes::GestionnaireFichiersDemandes(const std::string& chemin)
    : chemin_(chemin) {}

std::vector<DemandeAdministrative> GestionnaireFichiersDemandes::lireDemandes() const {
    if (Database::instance().estConnectee()) return db::lireDemandes();   // mode MySQL
    std::vector<DemandeAdministrative> res;
    std::ifstream in(chemin_);
    if (!in) return res;   // fichier absent : liste vide, sera cree a la premiere ecriture

    std::string ligne;
    while (std::getline(in, ligne)) {
        if (ligne.empty()) continue;

        std::vector<std::string> champs = decouper(ligne, ';');
        if (champs.size() != 6) continue;   // ligne illisible : ignoree

        try {
            int id = std::stoi(champs[0]);
            int idEmploye = std::stoi(champs[1]);
            Date date = Date::fromString(champs[3]);

            if (!date.estValide() || !DemandeAdministrative::statutValide(champs[4])) continue;

            res.emplace_back(id, idEmploye, champs[2], champs[5], date, champs[4]);
        } catch (const std::exception&) {
            continue;   // enregistrement corrompu : ignore
        }
    }
    return res;
}

void GestionnaireFichiersDemandes::ecrireDemandes(
    const std::vector<DemandeAdministrative>& demandes) const {

    if (Database::instance().estConnectee()) {   // mode MySQL
        db::ecrireDemandes(demandes);
        return;
    }

#ifdef _WIN32
    // Creation du dossier data/ sous Windows
    std::string dossier = chemin_;
    std::size_t slash = dossier.find_last_of("/\\");
    if (slash != std::string::npos) {
        std::string cheminDossier = dossier.substr(0, slash);
        if (!cheminDossier.empty()) _mkdir(cheminDossier.c_str());
    }
#else
    std::filesystem::path dossier = std::filesystem::path(chemin_).parent_path();
    if (!dossier.empty()) std::filesystem::create_directories(dossier);
#endif

    std::ofstream out(chemin_, std::ios::trunc);
    if (!out) throw ExceptionDemande("Impossible d'ecrire le fichier : " + chemin_);

    for (const auto& d : demandes) {
        out << d.getId() << ';'
            << d.getIdEmploye() << ';'
            << d.getType() << ';'
            << d.getDateDepot().toString() << ';'
            << d.getStatut() << ';'
            << d.getDescription() << '\n';
    }

    if (!out) throw ExceptionDemande("Erreur d'ecriture dans le fichier : " + chemin_);
}
