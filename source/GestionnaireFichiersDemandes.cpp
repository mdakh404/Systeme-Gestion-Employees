#include "GestionnaireFichiersDemandes.h"
#include "ExceptionsDemandes.h"
#include <cstdio>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <direct.h>
#else
#include <filesystem>
#endif

namespace {

// Decoupe une ligne en champs separes par ';'
std::vector<std::string> decouper(const std::string& ligne, char sep) {
    std::vector<std::string> champs;
    std::string champ;
    std::istringstream is(ligne);
    while (std::getline(is, champ, sep)) champs.push_back(champ);
    return champs;
}

// HIGH-004 : lit un entier strict, toute la chaine doit etre consommee.
// Rejette "1abc", "-1", " 1 ", "" et tout depassement.
bool entierStrict(const std::string& texte, int& sortie) {
    if (texte.empty()) return false;

    std::size_t debut = 0;
    // pas d'espace autour : le format impose des champs nus
    if (texte[0] < '0' || texte[0] > '9') return false;

    long long valeur = 0;
    for (std::size_t i = debut; i < texte.size(); ++i) {
        if (texte[i] < '0' || texte[i] > '9') return false;   // suffixe parasite
        valeur = valeur * 10 + (texte[i] - '0');
        if (valeur > 2147483647LL) return false;              // debordement int
    }

    sortie = static_cast<int>(valeur);
    return true;
}

} // namespace

GestionnaireFichiersDemandes::GestionnaireFichiersDemandes(const std::string& chemin)
    : chemin_(chemin) {}

std::vector<DemandeAdministrative> GestionnaireFichiersDemandes::lireDemandes() const {
    std::vector<DemandeAdministrative> res;
    std::ifstream in(chemin_);
    if (!in) return res;   // fichier absent : liste vide, sera cree a la premiere ecriture

    std::string ligne;
    while (std::getline(in, ligne)) {
        if (ligne.empty()) continue;

        std::vector<std::string> champs = decouper(ligne, ';');
        if (champs.size() != 6) continue;   // ligne illisible : ignoree

        // HIGH-004 : bornes metier strictes
        int id = 0;
        int idEmploye = 0;
        if (!entierStrict(champs[0], id) || id <= 0) continue;
        if (!entierStrict(champs[1], idEmploye) || idEmploye <= 0) continue;

        try {
            Date date = Date::fromString(champs[3]);
            if (!date.estValide()) continue;
        } catch (const std::exception&) {
            continue;
        }

        if (!DemandeAdministrative::statutValide(champs[4])) continue;
        if (champs[2].empty() || champs[5].empty()) continue;

        res.emplace_back(id, idEmploye, champs[2], champs[5],
                         Date::fromString(champs[3]), champs[4]);
    }
    return res;
}

void GestionnaireFichiersDemandes::ecrireDemandes(
    const std::vector<DemandeAdministrative>& demandes) const {

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

    // HIGH-005 : ecriture dans un fichier temporaire, puis remplacement
    // atomique. Un echec pendant l'ecriture laisse l'ancien fichier intact.
    const std::string temp = chemin_ + ".tmp";

    {
        std::ofstream out(temp, std::ios::trunc | std::ios::binary);
        if (!out) throw ExceptionDemande("Impossible d'ecrire le fichier : " + temp);

        for (const auto& d : demandes) {
            out << d.getId() << ';'
                << d.getIdEmploye() << ';'
                << d.getType() << ';'
                << d.getDateDepot().toString() << ';'
                << d.getStatut() << ';'
                << d.getDescription() << '\n';
        }

        out.flush();
        if (!out) {
            out.close();
            std::remove(temp.c_str());
            throw ExceptionDemande("Erreur d'ecriture dans le fichier : " + temp);
        }
    }

    // Remplacement atomique : sous Windows remove() doit passer avant rename()
    std::remove(chemin_.c_str());
    if (std::rename(temp.c_str(), chemin_.c_str()) != 0) {
        std::remove(temp.c_str());
        throw ExceptionDemande("Impossible de remplacer le fichier : " + chemin_);
    }
}
