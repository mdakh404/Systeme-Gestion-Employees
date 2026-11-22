#pragma once
#include <string>
#include <vector>
#include "Date.h"

// Donnees necessaires au depot d'une demande administrative.
struct DonneesDemande {
    int idEmploye = 0;
    std::string type;
    std::string description;
    Date dateDepot;
};

// Couche 3 (domaine) : represente une demande administrative.
// Cycle de vie du statut : "En attente" -> "Acceptee" | "Refusee"
class DemandeAdministrative {
    int id_ = 0;
    int idEmploye_ = 0;
    std::string type_;
    std::string description_;
    Date dateDepot_;
    std::string statut_;

public:
    static constexpr const char* STATUT_EN_ATTENTE = "En attente";
    static constexpr const char* STATUT_ACCEPTEE = "Acceptee";
    static constexpr const char* STATUT_REFUSEE = "Refusee";

    DemandeAdministrative() : statut_(STATUT_EN_ATTENTE) {}
    DemandeAdministrative(int id, const DonneesDemande& d);
    DemandeAdministrative(int id, int idEmploye, const std::string& type,
                          const std::string& description, const Date& dateDepot,
                          const std::string& statut);

    void afficherInfos() const;

    // Une demande traitee ne peut plus changer de statut (throws ExceptionDemande)
    void definirStatut(const std::string& statut);
    void restaurerStatut(const std::string& statut);   // rollback interne
    bool estEnAttente() const { return statut_ == STATUT_EN_ATTENTE; }

    // Getters
    int getId() const { return id_; }
    int getIdEmploye() const { return idEmploye_; }
    const std::string& getType() const { return type_; }
    const std::string& getDescription() const { return description_; }
    const Date& getDateDepot() const { return dateDepot_; }
    const std::string& getStatut() const { return statut_; }

    static bool statutValide(const std::string& statut);
    static std::vector<std::string> typesProposes();

    bool operator==(const DemandeAdministrative& o) const { return id_ == o.id_; }
};
