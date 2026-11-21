#pragma once
#include <stdexcept>
#include <string>
#include <vector>

// Hierarchie d'exceptions du module Demandes administratives
class ExceptionDemande : public std::runtime_error {
public:
    explicit ExceptionDemande(const std::string& msg) : std::runtime_error(msg) {}
};

class DemandeIntrouvable : public ExceptionDemande {
public:
    explicit DemandeIntrouvable(int id)
        : ExceptionDemande("Demande introuvable (id = " + std::to_string(id) + ")") {}
};

class DemandeDejaTraitee : public ExceptionDemande {
public:
    explicit DemandeDejaTraitee(int id, const std::string& statut)
        : ExceptionDemande("Demande " + std::to_string(id) +
                           " deja traitee (statut: " + statut + ")") {}
};

class DonneesDemandeInvalides : public ExceptionDemande {
    std::vector<std::string> erreurs_;
    static std::string joindre(const std::vector<std::string>& e) {
        std::string s = "Donnees invalides: ";
        for (size_t i = 0; i < e.size(); ++i) s += (i ? "; " : "") + e[i];
        return s;
    }
public:
    explicit DonneesDemandeInvalides(const std::vector<std::string>& e)
        : ExceptionDemande(joindre(e)), erreurs_(e) {}
    const std::vector<std::string>& erreurs() const { return erreurs_; }
};
