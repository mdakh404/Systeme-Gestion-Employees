#pragma once
#include <stdexcept>
#include <vector>

// NOTE: version minimale. La classe Salaire complete (prime d'anciennete,
// calcul, historique) appartient a l'equipe "Salaires". Employe la POSSEDE
// (composition) : elle vit et meurt avec lui.
class Salaire {
    double salaireBase_ = 0.0;
    double primeAnciennete_ = 0.0;
    std::vector<double> historique_;
public:
    Salaire() = default;
    explicit Salaire(double base) { setSalaireBase(base); }

    double getSalaireBase() const { return salaireBase_; }
    double getPrimeAnciennete() const { return primeAnciennete_; }
    const std::vector<double>& getHistorique() const { return historique_; }

    void setSalaireBase(double b) {
        if (b < 0) throw std::invalid_argument("salaire de base negatif");
        if (salaireBase_ != b) historique_.push_back(salaireBase_);
        salaireBase_ = b;
    }
};
