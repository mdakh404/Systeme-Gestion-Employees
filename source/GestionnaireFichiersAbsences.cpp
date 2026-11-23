#include "GestionnaireFichiersAbsences.h"
#include "DbPersistence.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace {

namespace fs = std::filesystem;
constexpr char prefixeCompteur[] = "#dernierId;";

bool memesDonnees(const Absence& gauche, const Absence& droite) {
    return gauche.getIdEmploye() == droite.getIdEmploye()
           && gauche.getDateAbsence() == droite.getDateAbsence()
           && gauche.getMotif() == droite.getMotif()
           && gauche.estJustifiee() == droite.estJustifiee()
           && gauche.estAnnulee() == droite.estAnnulee();
}

int lireIdentifiant(const std::string& champ) {
    if (champ.empty() || champ.find_first_not_of("0123456789") != std::string::npos) {
        throw std::invalid_argument("Identifiant entier positif attendu.");
    }
    const int id = std::stoi(champ);
    if (id <= 0) {
        throw std::invalid_argument("Identifiant entier positif attendu.");
    }
    return id;
}

fs::path cheminRepere(const std::string& chemin) {
    return fs::path(chemin).concat(".compteur");
}

int lireRepere(const fs::path& chemin) {
    const auto statut = fs::symlink_status(chemin);
    if (statut.type() == fs::file_type::not_found) {
        return 0;
    }
    std::ifstream entree;
    if (fs::is_regular_file(statut)) {
        entree.open(chemin, std::ios::binary);
    }
    std::string ligne;
    if (!std::getline(entree, ligne) || entree.peek() != std::char_traits<char>::eof() || entree.bad()) {
        throw std::runtime_error("Repère d'identifiants illisible : " + chemin.string());
    }
    try {
        return lireIdentifiant(ligne);
    } catch (const std::logic_error&) {
        throw std::runtime_error("Repère d'identifiants invalide : " + chemin.string());
    }
}

void ecrireRepere(const fs::path& destination, const fs::path& temporaire, int identifiant) {
    std::ofstream sortie(temporaire, std::ios::binary | std::ios::trunc);
    sortie << identifiant << '\n';
    sortie.close();
    if (!sortie) {
        throw std::runtime_error("Impossible d'écrire le repère d'identifiants.");
    }
    fs::rename(temporaire, destination);
}

Absence lireAbsence(const std::string& ligne) {
    std::istringstream champs(ligne);
    std::string id, employe, date, motif, justification, annulation = "0";
    if (!std::getline(champs, id, ';') || !std::getline(champs, employe, ';')
        || !std::getline(champs, date, ';') || !std::getline(champs, motif, ';')
        || !std::getline(champs, justification, ';')
        || (!champs.eof() && (!std::getline(champs, annulation, ';') || !champs.eof()))) {
        throw std::invalid_argument("Cinq ou six champs separes par des points-virgules sont attendus.");
    }
    if (justification != "0" && justification != "1") {
        throw std::invalid_argument("La justification doit valoir 0 ou 1.");
    }
    if (motif.find_first_of("\r\n") != std::string::npos) {
        throw std::invalid_argument("Saut de ligne dans le motif.");
    }
    if (annulation != "0" && annulation != "1") {
        throw std::invalid_argument("Le statut d'annulation doit valoir 0 ou 1.");
    }
    return Absence(lireIdentifiant(id), lireIdentifiant(employe), date, motif,
                   justification == "1", annulation == "1");
}

class FichierTemporaire {
public:
    explicit FichierTemporaire(const fs::path& destination)
        : dossier(fs::path(destination).concat(".sauvegarde")),
          fichier(dossier / "absences.tmp") {
        if (!fs::create_directory(dossier)) {
            throw std::runtime_error("Sauvegarde deja en cours ou dossier temporaire present : "
                                     + dossier.string());
        }
    }

    ~FichierTemporaire() {
        std::error_code erreur;
        fs::remove(fichier, erreur);
        fs::remove(cheminCompteur(), erreur);
        fs::remove(dossier, erreur);
    }

    FichierTemporaire(const FichierTemporaire&) = delete;
    FichierTemporaire& operator=(const FichierTemporaire&) = delete;

    const fs::path& chemin() const { return fichier; }
    const fs::path& cheminCompteur() const { return fichierCompteur; }

private:
    fs::path dossier;
    fs::path fichier;
    fs::path fichierCompteur = dossier / "compteur.tmp";
};

} // namespace

GestionnaireFichiersAbsences::GestionnaireFichiersAbsences(const std::string& chemin)
    : chemin(chemin) {}

GestionnaireAbsences GestionnaireFichiersAbsences::chargerAbsences() const {
    return charger(nullptr);
}

GestionnaireAbsences GestionnaireFichiersAbsences::chargerAbsences(
    GestionnaireAbsences::VerificationEmploye employeExiste) const {
    auto gestionnaire = charger(nullptr);
    gestionnaire.employeExiste = std::move(employeExiste);
    return gestionnaire;
}
GestionnaireAbsences GestionnaireFichiersAbsences::charger(
    std::vector<std::string>* lignesOriginales) const {
    GestionnaireAbsences gestionnaire;
    if (Database::instance().estConnectee()) {   // mode MySQL
        auto lues = db::chargerAbsences();
        for (auto& absence : lues.liste) {
            try {
                gestionnaire.restaurerAbsence(absence);
            } catch (const std::exception& ex) {
                std::cerr << "Absence " << absence.getId() << " ignoree : " << ex.what() << "\n";
            }
        }
        gestionnaire.dernierIdAttribue = std::max(gestionnaire.dernierIdAttribue, lues.dernierId);
        return gestionnaire;
    }
    const int repere = lireRepere(cheminRepere(chemin));
    const auto statut = fs::symlink_status(chemin);
    if (statut.type() == fs::file_type::not_found) {
        if (repere != 0) {
            throw std::runtime_error("Fichier d'absences manquant malgré un repère d'identifiants : " + chemin);
        }
        return gestionnaire;
    }
    if (!fs::is_regular_file(statut)) {
        throw std::runtime_error("Fichier d'absences non ordinaire : " + chemin);
    }

    std::ifstream entree(chemin, std::ios::binary);
    if (!entree) {
        throw std::runtime_error("Impossible d'ouvrir le fichier en lecture : " + chemin);
    }

    std::string ligne;
    std::size_t numero = 0;
    int compteur = 0;
    while (std::getline(entree, ligne)) {
        ++numero;
        const bool estEntete = numero == 1
            && ligne.compare(0, sizeof prefixeCompteur - 1, prefixeCompteur) == 0;
        if (lignesOriginales && !estEntete) {
            lignesOriginales->push_back(entree.eof() ? ligne : ligne + '\n');
        }
        if (!ligne.empty() && ligne.back() == '\r') {
            ligne.pop_back();
        }
        try {
            if (estEntete) {
                compteur = lireIdentifiant(ligne.substr(sizeof prefixeCompteur - 1));
            } else {
                gestionnaire.restaurerAbsence(lireAbsence(ligne));
            }
        } catch (const std::logic_error& erreur) {
            throw std::runtime_error(chemin + ", ligne " + std::to_string(numero) + " : "
                                     + erreur.what());
        }
    }
    if (entree.bad() || !entree.eof()) {
        throw std::runtime_error("Erreur de lecture du fichier : " + chemin);
    }
    if (compteur != 0) {
        if (compteur < gestionnaire.dernierIdAttribue) {
            throw std::runtime_error(chemin + ", ligne 1 : compteur inférieur aux identifiants enregistrés.");
        }
        gestionnaire.dernierIdAttribue = compteur;
    }
    if (gestionnaire.dernierIdAttribue < repere) {
        throw std::runtime_error("Restauration incohérente ou réinitialisation du fichier d'absences : " + chemin);
    }
    return gestionnaire;
}

void GestionnaireFichiersAbsences::sauvegarderAbsences(
    const GestionnaireAbsences& gestionnaire) const {
    sauvegarder(gestionnaire, nullptr);
}

void GestionnaireFichiersAbsences::sauvegarderAbsences(
    const GestionnaireAbsences& gestionnaire, const GestionnaireAbsences& etatInitial) const {
    sauvegarder(gestionnaire, &etatInitial);
}

void GestionnaireFichiersAbsences::sauvegarderSuppression(
    const GestionnaireAbsences& gestionnaire, const GestionnaireAbsences& etatInitial,
    int idSupprime) const {
    if (idSupprime <= 0 || !etatInitial.rechercherAbsence(idSupprime)
        || gestionnaire.rechercherAbsence(idSupprime)
        || gestionnaire.consulterAbsences().size() + 1 != etatInitial.consulterAbsences().size()
        || gestionnaire.dernierIdAttribue != etatInitial.dernierIdAttribue) {
        throw std::invalid_argument("La sauvegarde doit correspondre à la suppression d'une seule absence.");
    }
    sauvegarder(gestionnaire, &etatInitial, idSupprime);
}

void GestionnaireFichiersAbsences::sauvegarder(
    const GestionnaireAbsences& gestionnaire, const GestionnaireAbsences* etatInitial,
    int idSupprime) const {
    if (Database::instance().estConnectee()) {   // mode MySQL : transaction tout ou rien
        db::sauvegarderAbsences(gestionnaire.consulterAbsences(), idSupprime);
        return;
    }
    const auto& comparaison = etatInitial ? *etatInitial : gestionnaire;
    const auto& absences = gestionnaire.consulterAbsences();
    for (const auto& absence : absences) {
        if (absence.getMotif().find_first_of(";\r\n") != std::string::npos) {
            throw std::invalid_argument("Le motif ne peut contenir ni point-virgule ni saut de ligne.");
        }
    }

    const FichierTemporaire temporaire(chemin);
    const auto repere = cheminRepere(chemin);
    const int borne = lireRepere(repere);
    std::vector<std::string> lignesOriginales;
    const auto existantes = charger(idSupprime != 0 ? &lignesOriginales : nullptr);
    if (gestionnaire.dernierIdAttribue < existantes.dernierIdAttribue
        || (etatInitial && (comparaison.dernierIdAttribue != existantes.dernierIdAttribue
            || comparaison.consulterAbsences().size() != existantes.consulterAbsences().size()))) {
        throw std::runtime_error("Sauvegarde refusée : le fichier a changé depuis son chargement.");
    }
    for (const auto& existence : existantes.consulterAbsences()) {
        const auto absence = comparaison.rechercherAbsence(existence.getId());
        if (!absence || !memesDonnees(*absence, existence)) {
            throw std::runtime_error("Sauvegarde refusee : une absence sur disque manque ou differe.");
        }
        if (existence.getId() != idSupprime) {
            const auto candidate = gestionnaire.rechercherAbsence(existence.getId());
            if (!candidate || candidate->getIdEmploye() != existence.getIdEmploye()
                || (idSupprime != 0 && !memesDonnees(*candidate, existence))) {
                throw std::runtime_error("Sauvegarde refusée : une autre absence a été retirée ou modifiée.");
            }
        }
    }

    int plusGrandIdPresent = 0;
    for (const auto& absence : absences) {
        const int id = absence.getId();
        if (id <= existantes.dernierIdAttribue && !existantes.rechercherAbsence(id)) {
            throw std::runtime_error("Sauvegarde refusée : l'identifiant a déjà été attribué.");
        }
        plusGrandIdPresent = std::max(plusGrandIdPresent, id);
    }

    std::ofstream sortie(temporaire.chemin(), std::ios::binary | std::ios::trunc);
    if (!sortie) {
        throw std::runtime_error("Impossible d'ouvrir le fichier temporaire pour : " + chemin);
    }
    if (gestionnaire.dernierIdAttribue > plusGrandIdPresent) {
        sortie << prefixeCompteur << gestionnaire.dernierIdAttribue << '\n';
    }
    if (idSupprime != 0) {
        const auto& originales = existantes.consulterAbsences();
        for (std::size_t i = 0; i < originales.size(); ++i) {
            if (originales[i].getId() != idSupprime) {
                sortie << lignesOriginales[i];
            }
        }
    } else {
        for (const auto& absence : absences) {
            sortie << absence.getId() << ';' << absence.getIdEmploye() << ';'
                   << absence.getDateAbsence() << ';' << absence.getMotif() << ';'
                   << (absence.estJustifiee() ? '1' : '0') << ';'
                   << (absence.estAnnulee() ? '1' : '0') << '\n';
        }
    }
    sortie.close();
    if (!sortie) {
        throw std::runtime_error("Erreur d'ecriture du fichier temporaire pour : " + chemin);
    }
    // Le repère ne dépasse jamais un compteur déjà publié dans le fichier principal.
    if (existantes.dernierIdAttribue > borne) {
        ecrireRepere(repere, temporaire.cheminCompteur(), existantes.dernierIdAttribue);
    }
    fs::rename(temporaire.chemin(), chemin);
    if (gestionnaire.dernierIdAttribue > existantes.dernierIdAttribue) {
        try {
            ecrireRepere(repere, temporaire.cheminCompteur(), gestionnaire.dernierIdAttribue);
        } catch (const std::exception&) {
            // La transaction est déjà publiée. Une prochaine sauvegarde réparera le repère.
        }
    }
}
