#include "Departement.h"
#include "Exceptions.h"
#include "Validation.h"
#include <iostream>
#include <vector>

static void verifierNom(const std::string& nom) {
    if (!Validation::nomDepartementValide(nom))
        throw DonneesInvalides({"nom de departement invalide (2-50 caracteres : lettres, chiffres, espace, - ' &)"});
}

static void verifierDescription(const std::string& d) {
    if (d.size() > Departement::DESCRIPTION_MAX || !Validation::sansSeparateur(d))
        throw DonneesInvalides({"description invalide (max 200 caracteres, sans ';' ni '|')"});
}

Departement::Departement(int id, const std::string& nom, const std::string& description)
    : id_(id) {
    verifierNom(nom);
    verifierDescription(description);
    nom_ = nom;
    description_ = description;
}

void Departement::setNom(const std::string& nom) {
    verifierNom(nom);
    nom_ = nom;
}

void Departement::setDescription(const std::string& description) {
    verifierDescription(description);
    description_ = description;
}

void Departement::afficherInfos() const {
    std::cout << "[" << id_ << "] " << nom_;
    if (!description_.empty()) std::cout << " - " << description_;
    std::cout << "\n";
}
