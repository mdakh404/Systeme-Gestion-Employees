#include "Absence.h"

#include <array>
#include <iostream>
#include <stdexcept>

int Absence::numeroJour(const std::string& date) {
    const char* message = "La date doit etre une date valide au format JJ/MM/AAAA.";
    if (date.size() != 10 || date[2] != '/' || date[5] != '/') {
        throw std::invalid_argument(message);
    }

    for (std::size_t i = 0; i < date.size(); ++i) {
        if (i != 2 && i != 5 && (date[i] < '0' || date[i] > '9')) {
            throw std::invalid_argument(message);
        }
    }

    const int jour = std::stoi(date.substr(0, 2));
    const int mois = std::stoi(date.substr(3, 2));
    const int annee = std::stoi(date.substr(6, 4));
    if (annee == 0 || mois < 1 || mois > 12 || jour < 1) {
        throw std::invalid_argument(message);
    }

    constexpr std::array<int, 12> joursParMois{
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    const bool bissextile = annee % 4 == 0 && (annee % 100 != 0 || annee % 400 == 0);
    const int maximum = joursParMois[mois - 1] + (mois == 2 && bissextile);
    if (jour > maximum) {
        throw std::invalid_argument(message);
    }

    const int anneesCompletes = annee - 1;
    int total = 365 * anneesCompletes + anneesCompletes / 4
                - anneesCompletes / 100 + anneesCompletes / 400 + jour;
    for (int i = 0; i < mois - 1; ++i) {
        total += joursParMois[i];
    }
    return total + (bissextile && mois > 2);
}

Absence::Absence(int id, int idEmploye, const std::string& dateAbsence,
                 const std::string& motif, bool justifiee, bool annulee)
    : id(id), idEmploye(idEmploye), dateAbsence(dateAbsence), motif(motif),
      justifiee(justifiee), annulee(annulee) {
    numeroJour(dateAbsence);
}

int Absence::getId() const {
    return id;
}

int Absence::getIdEmploye() const {
    return idEmploye;
}

const std::string& Absence::getDateAbsence() const {
    return dateAbsence;
}

const std::string& Absence::getMotif() const {
    return motif;
}

bool Absence::estJustifiee() const {
    return justifiee;
}

void Absence::setDateAbsence(const std::string& dateAbsence) {
    numeroJour(dateAbsence);
    this->dateAbsence = dateAbsence;
}

void Absence::setMotif(const std::string& motif) {
    this->motif = motif;
}

void Absence::setJustifiee(bool justifiee) {
    this->justifiee = justifiee;
}

bool Absence::estAnnulee() const {
    return annulee;
}

void Absence::annuler() {
    annulee = true;
}

void Absence::afficher() const {
    std::cout << "ID absence : " << id << '\n'
              << "ID employe : " << idEmploye << '\n'
              << "Date : " << dateAbsence << '\n'
              << "Motif : " << motif << '\n'
              << "Justifiee : " << (justifiee ? "Oui" : "Non") << '\n'
              << "Statut : " << (annulee ? "Annul\u00e9e" : "Active") << '\n';
}
