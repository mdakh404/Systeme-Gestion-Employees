#include "ProgrammeRecompense.h"

#include "DateIso.h"
#include "ValidationTexte.h"

#include <iostream>
#include <stdexcept>
#include <utility>

std::string toString(TypeProgramme type) {
    switch (type) {
        case TypeProgramme::Performance:    return "performance";
        case TypeProgramme::Fidelite:       return "fidelite";
        case TypeProgramme::Reconnaissance: return "reconnaissance";
    }
    return "performance";
}

TypeProgramme typeDepuisTexte(const std::string& texte) {
    if (texte == "performance")    return TypeProgramme::Performance;
    if (texte == "fidelite")       return TypeProgramme::Fidelite;
    if (texte == "reconnaissance") return TypeProgramme::Reconnaissance;
    throw std::invalid_argument("Type de programme inconnu : " + texte);
}

ProgrammeRecompense::ProgrammeRecompense(int id,
                                         std::string nom,
                                         std::string description,
                                         TypeProgramme type,
                                         int pointsMaxParAttribution,
                                         std::string dateDebut,
                                         std::string dateFin,
                                         bool actif)
    : id_(id),
      nom_(std::move(nom)),
      description_(std::move(description)),
      type_(type),
      pointsMaxParAttribution_(pointsMaxParAttribution),
      dateDebut_(std::move(dateDebut)),
      dateFin_(std::move(dateFin)),
      actif_(actif) {
    if (id_ <= 0) {
        throw std::invalid_argument("Identifiant de programme invalide.");
    }
    rewards::verifierTexteObligatoire(nom_, "nom du programme");
    rewards::verifierTexteOptionnel(description_, "description du programme");
    if (pointsMaxParAttribution_ <= 0) {
        throw std::invalid_argument("Le maximum de points doit etre strictement positif.");
    }
    validerPeriode();
}

void ProgrammeRecompense::validerPeriode() const {
    if (!rewards::dateIsoValide(dateDebut_)) {
        throw std::invalid_argument("Date de debut invalide (format attendu : AAAA-MM-JJ).");
    }
    if (dateFin_.empty()) return;
    if (!rewards::dateIsoValide(dateFin_)) {
        throw std::invalid_argument("Date de fin invalide (format attendu : AAAA-MM-JJ).");
    }
    if (rewards::comparerDates(dateDebut_, dateFin_) > 0) {
        throw std::invalid_argument("La date de fin precede la date de debut.");
    }
}

void ProgrammeRecompense::setNom(const std::string& nom) {
    rewards::verifierTexteObligatoire(nom, "nom du programme");
    nom_ = nom;
}

void ProgrammeRecompense::setDescription(const std::string& description) {
    rewards::verifierTexteOptionnel(description, "description du programme");
    description_ = description;
}

void ProgrammeRecompense::setType(TypeProgramme type) {
    type_ = type;
}

void ProgrammeRecompense::setPointsMaxParAttribution(int points) {
    if (points <= 0) {
        throw std::invalid_argument("Le maximum de points doit etre strictement positif.");
    }
    pointsMaxParAttribution_ = points;
}

void ProgrammeRecompense::setPeriode(const std::string& dateDebut, const std::string& dateFin) {
    const std::string ancienneDebut = dateDebut_;
    const std::string ancienneFin = dateFin_;
    dateDebut_ = dateDebut;
    dateFin_ = dateFin;
    try {
        validerPeriode();
    } catch (...) {
        dateDebut_ = ancienneDebut;
        dateFin_ = ancienneFin;
        throw;
    }
}

bool ProgrammeRecompense::estDisponible(const std::string& date) const {
    if (!actif_) return false;
    if (!rewards::dateIsoValide(date)) {
        throw std::invalid_argument("Date de reference invalide (format attendu : AAAA-MM-JJ).");
    }
    if (rewards::comparerDates(date, dateDebut_) < 0) return false;
    if (!dateFin_.empty() && rewards::comparerDates(date, dateFin_) > 0) return false;
    return true;
}

void ProgrammeRecompense::afficher() const {
    std::cout << "  #" << id_ << " - " << nom_
              << " [" << toString(type_) << "]"
              << (actif_ ? "" : " (desactive)") << "\n"
              << "      " << (description_.empty() ? "(sans description)" : description_) << "\n"
              << "      Points max / attribution : " << pointsMaxParAttribution_ << "\n"
              << "      Periode : " << dateDebut_ << " -> "
              << (dateFin_.empty() ? "illimitee" : dateFin_) << "\n";
}
