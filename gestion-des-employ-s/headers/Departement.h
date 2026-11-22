#pragma once
#include <string>

// Departement : regroupe plusieurs employes (agregation, 1 -> 0..*).
// Le lien est porte par Employe (idDepartement) : un departement ne "possede"
// pas ses employes, il existe independamment d'eux.
class Departement {
    int id_;
    std::string nom_;
    std::string description_;

public:
    // throws DonneesInvalides si le nom ou la description est invalide
    Departement(int id, const std::string& nom, const std::string& description = "");

    void afficherInfos() const;

    int getId() const { return id_; }
    const std::string& getNom() const { return nom_; }
    const std::string& getDescription() const { return description_; }

    void setNom(const std::string& nom);                   // throws DonneesInvalides
    void setDescription(const std::string& description);   // throws DonneesInvalides

    bool operator==(const Departement& o) const { return id_ == o.id_; }
    bool operator!=(const Departement& o) const { return !(*this == o); }

    static constexpr std::size_t DESCRIPTION_MAX = 200;
};
