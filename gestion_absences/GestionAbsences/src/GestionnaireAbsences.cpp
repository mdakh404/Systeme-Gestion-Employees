#include "GestionnaireAbsences.h"
#include "Utf8.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <utility>

namespace {

constexpr int delaiEnregistrement = 30;
constexpr int delaiJustification = 7;
constexpr char espaces[] = " \t\n\r\f\v";

std::string dateLocale() {
    const auto heure = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const auto* locale = std::localtime(&heure);
    char aujourdhui[11];
    if (!locale || std::strftime(aujourdhui, sizeof aujourdhui, "%d/%m/%Y", locale) == 0) {
        throw std::runtime_error("Impossible de lire la date locale du syst\u00e8me.");
    }
    return aujourdhui;
}

} // namespace

GestionnaireAbsences::GestionnaireAbsences(const std::string& dateReference)
    : jourReference(Absence::numeroJour(dateReference)) {}

GestionnaireAbsences::GestionnaireAbsences(VerificationEmploye employeExiste)
    : employeExiste(std::move(employeExiste)) {}

GestionnaireAbsences::GestionnaireAbsences(const std::string& dateReference,
                                          VerificationEmploye employeExiste)
    : jourReference(Absence::numeroJour(dateReference)), employeExiste(std::move(employeExiste)) {}

void GestionnaireAbsences::verifierEmploye(int idEmploye) const {
    if (idEmploye <= 0) {
        throw std::invalid_argument("L'identifiant employé doit être strictement positif.");
    }
    if (!employeExiste) {
        throw std::runtime_error("Vérification des employés indisponible : aucun vérificateur configuré.");
    }
    bool existe;
    try {
        existe = employeExiste(idEmploye);
    } catch (const std::exception& erreur) {
        throw std::runtime_error(std::string("Vérification des employés indisponible : ") + erreur.what());
    } catch (...) {
        throw std::runtime_error("Vérification des employés indisponible : erreur de recherche.");
    }
    if (!existe) {
        throw std::invalid_argument("Employé introuvable.");
    }
}

std::string GestionnaireAbsences::normaliserMotif(const std::string& motif) {
    if (motif.find_first_of(";\r\n") != std::string::npos) {
        throw std::invalid_argument("Le motif ne peut contenir ni point-virgule ni saut de ligne.");
    }
    std::size_t debut = std::string::npos;
    std::size_t fin = 0;
    std::size_t longueur = 0;
    std::size_t blancs = 0;
    for (std::size_t position = 0; position < motif.size();) {
        const auto avant = position;
        char32_t point;
        if (!utf8::lirePoint(motif, position, point)) {
            throw std::invalid_argument("Le motif doit être encodé en UTF-8 valide.");
        }
        if (utf8::estBlanc(point)) {
            if (debut != std::string::npos) {
                ++blancs;
            }
            continue;
        }
        if (debut == std::string::npos) {
            debut = avant;
        }
        fin = position;
        longueur += blancs + 1;
        blancs = 0;
    }
    if (debut == std::string::npos) {
        throw std::invalid_argument("Le motif de l'absence est obligatoire.");
    }
    if (longueur < 3 || longueur > 200) {
        throw std::invalid_argument("Le motif doit contenir entre 3 et 200 caractères.");
    }
    return motif.substr(debut, fin - debut);
}

void GestionnaireAbsences::verifierDelais(const std::string& dateAbsence, bool justifiee) const {
    const int jour = Absence::numeroJour(dateAbsence);
    const int reference = jourReference ? *jourReference : Absence::numeroJour(dateLocale());
    const int age = reference - jour;
    if (age < 0) {
        throw std::invalid_argument("impossible d'enregistrer une absence future.");
    }
    if (age > delaiEnregistrement) {
        throw std::invalid_argument("d\u00e9lai maximal d'enregistrement d\u00e9pass\u00e9 ("
                                    + std::to_string(delaiEnregistrement) + " jours).");
    }
    if (age > delaiJustification && !justifiee) {
        throw std::invalid_argument("une absence de plus de " + std::to_string(delaiJustification)
                                    + " jours doit \u00eatre justifi\u00e9e.");
    }
}

bool GestionnaireAbsences::contientDoublon(int idEmploye, const std::string& dateAbsence,
                                            int idIgnore) const {
    return std::any_of(absences.begin(), absences.end(),
        [idEmploye, &dateAbsence, idIgnore](const Absence& absence) {
            return !absence.estAnnulee() && absence.getId() != idIgnore
                   && absence.getIdEmploye() == idEmploye && absence.getDateAbsence() == dateAbsence;
        });
}

bool GestionnaireAbsences::enregistrerAbsence(int idEmploye, const std::string& dateAbsence,
                                              const std::string& motif, bool justifiee) {
    verifierEmploye(idEmploye);
    verifierDelais(dateAbsence, justifiee);
    const auto motifNormalise = normaliserMotif(motif);
    if (contientDoublon(idEmploye, dateAbsence)) {
        return false;
    }

    if (dernierIdAttribue == std::numeric_limits<int>::max()) {
        throw std::overflow_error("Tous les identifiants d'absence ont ete utilises.");
    }

    absences.emplace_back(dernierIdAttribue + 1, idEmploye, dateAbsence, motifNormalise, justifiee);
    ++dernierIdAttribue;
    return true;
}

Absence& GestionnaireAbsences::trouverActive(int idAbsence) {
    const auto resultat = std::find_if(absences.begin(), absences.end(),
        [idAbsence](const Absence& absence) { return absence.getId() == idAbsence; });
    if (resultat == absences.end()) {
        throw std::invalid_argument("Absence introuvable.");
    }
    if (resultat->estAnnulee()) {
        throw std::invalid_argument("Cette absence est d\u00e9j\u00e0 annul\u00e9e.");
    }
    return *resultat;
}

bool GestionnaireAbsences::modifierAbsence(int idAbsence, const std::string& nouvelleDate,
                                          const std::string& nouveauMotif, bool nouvelleJustification) {
    auto& absence = trouverActive(idAbsence);
    verifierDelais(nouvelleDate, nouvelleJustification);
    const auto motifNormalise = normaliserMotif(nouveauMotif);
    if (contientDoublon(absence.getIdEmploye(), nouvelleDate, idAbsence)) {
        return false;
    }
    absence = Absence(idAbsence, absence.getIdEmploye(), nouvelleDate, motifNormalise, nouvelleJustification);
    return true;
}

void GestionnaireAbsences::annulerAbsence(int idAbsence) {
    trouverActive(idAbsence).annuler();
}

bool GestionnaireAbsences::supprimerAbsence(int idAbsence) {
    const auto resultat = std::find_if(absences.begin(), absences.end(),
        [idAbsence](const Absence& absence) { return absence.getId() == idAbsence; });
    if (resultat == absences.end()) {
        return false;
    }
    absences.erase(resultat);
    return true;
}

void GestionnaireAbsences::restaurerAbsence(Absence absence) {
    const int id = absence.getId();
    if (absence.getIdEmploye() <= 0 || absence.getMotif().find_first_not_of(espaces) == std::string::npos) {
        throw std::invalid_argument("Identifiant employe ou motif historique invalide.");
    }
    if (id <= 0 || rechercherAbsence(id)) {
        throw std::invalid_argument("Identifiant d'absence invalide ou duplique.");
    }
    if (!absence.estAnnulee() && contientDoublon(absence.getIdEmploye(), absence.getDateAbsence())) {
        throw std::invalid_argument("Absence dupliquee pour cet employe et cette date.");
    }

    absences.push_back(std::move(absence));
    dernierIdAttribue = std::max(dernierIdAttribue, id);
}

const std::vector<Absence>& GestionnaireAbsences::consulterAbsences() const {
    return absences;
}

std::vector<Absence> GestionnaireAbsences::consulterAbsencesActives() const {
    std::vector<Absence> actives;
    std::copy_if(absences.begin(), absences.end(), std::back_inserter(actives),
        [](const Absence& absence) { return !absence.estAnnulee(); });
    return actives;
}

std::vector<Absence> GestionnaireAbsences::consulterHistorique(int idEmploye) const {
    std::vector<Absence> historique;
    std::copy_if(absences.begin(), absences.end(), std::back_inserter(historique),
        [idEmploye](const Absence& absence) {
            return absence.getIdEmploye() == idEmploye;
        });
    return historique;
}

std::optional<Absence> GestionnaireAbsences::rechercherAbsence(int idAbsence) const {
    const auto resultat = std::find_if(absences.begin(), absences.end(),
        [idAbsence](const Absence& absence) {
            return absence.getId() == idAbsence;
        });
    if (resultat == absences.end()) {
        return std::nullopt;
    }
    return *resultat;
}
