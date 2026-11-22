#include "Recompense.h"

#include "ValidationTexte.h"

#include <iostream>
#include <stdexcept>
#include <utility>

namespace {
constexpr int STOCK_MAXIMUM = 1000000;
}

Recompense::Recompense(int id,
                       std::string nom,
                       std::string description,
                       int coutPoints,
                       int stock)
    : id_(id),
      nom_(std::move(nom)),
      description_(std::move(description)),
      coutPoints_(coutPoints),
      stock_(stock) {
    if (id_ <= 0) {
        throw std::invalid_argument("Identifiant de recompense invalide.");
    }
    rewards::verifierTexteObligatoire(nom_, "nom de la recompense");
    rewards::verifierTexteOptionnel(description_, "description de la recompense");
    if (coutPoints_ <= 0) {
        throw std::invalid_argument("Le cout en points doit etre strictement positif.");
    }
    if (stock_ < 0 || stock_ > STOCK_MAXIMUM) {
        throw std::invalid_argument("Stock invalide.");
    }
}

void Recompense::setNom(const std::string& nom) {
    rewards::verifierTexteObligatoire(nom, "nom de la recompense");
    nom_ = nom;
}

void Recompense::setDescription(const std::string& description) {
    rewards::verifierTexteOptionnel(description, "description de la recompense");
    description_ = description;
}

void Recompense::setCoutPoints(int coutPoints) {
    if (coutPoints <= 0) {
        throw std::invalid_argument("Le cout en points doit etre strictement positif.");
    }
    coutPoints_ = coutPoints;
}

void Recompense::setStock(int stock) {
    if (stock < 0 || stock > STOCK_MAXIMUM) {
        throw std::invalid_argument("Stock invalide.");
    }
    stock_ = stock;
}

void Recompense::retirerDuStock() {
    if (stock_ <= 0) {
        throw std::invalid_argument("Recompense en rupture de stock.");
    }
    --stock_;
}

void Recompense::reapprovisionner(int quantite) {
    if (quantite <= 0) {
        throw std::invalid_argument("La quantite de reapprovisionnement doit etre positive.");
    }
    if (stock_ + quantite > STOCK_MAXIMUM) {
        throw std::invalid_argument("Le stock depasserait le maximum autorise.");
    }
    stock_ += quantite;
}

void Recompense::afficher() const {
    std::cout << "  #" << id_ << " - " << nom_
              << " (" << coutPoints_ << " points)"
              << (estDisponible() ? "" : " [rupture de stock]") << "\n"
              << "      " << (description_.empty() ? "(sans description)" : description_) << "\n"
              << "      Stock : " << stock_ << "\n";
}
