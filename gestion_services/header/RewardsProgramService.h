#ifndef REWARDSPROGRAMSERVICE_H
#define REWARDSPROGRAMSERVICE_H

#include "Service.h"

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// --- Adaptateur Employe ---
// Vue minimale d'un employe, fournie par le module Employes.
struct EmployeInfo {
    int id = 0;
    std::string matricule;
    std::string nom;
    std::string prenom;

    std::string nomComplet() const {
        return prenom.empty() ? nom : prenom + " " + nom;
    }
};

using FournisseurEmployes = std::function<std::optional<EmployeInfo>(int)>;

// --- Modeles du programme de recompenses ---

enum class TypeProgramme {
    Performance,
    Fidelite,
    Reconnaissance
};

std::string toString(TypeProgramme type);
TypeProgramme typeDepuisTexte(const std::string& texte);

class ProgrammeRecompense {
public:
    ProgrammeRecompense() = default;

    ProgrammeRecompense(int id,
                        std::string nom,
                        std::string description,
                        TypeProgramme type,
                        int pointsMaxParAttribution,
                        std::string dateDebut,
                        std::string dateFin = "",
                        bool actif = true);

    int getId() const { return id_; }
    const std::string& getNom() const { return nom_; }
    const std::string& getDescription() const { return description_; }
    TypeProgramme getType() const { return type_; }
    int getPointsMaxParAttribution() const { return pointsMaxParAttribution_; }
    const std::string& getDateDebut() const { return dateDebut_; }
    const std::string& getDateFin() const { return dateFin_; }
    bool estActif() const { return actif_; }

    void setNom(const std::string& nom);
    void setDescription(const std::string& description);
    void setType(TypeProgramme type);
    void setPointsMaxParAttribution(int points);
    void setPeriode(const std::string& dateDebut, const std::string& dateFin);

    void activer() { actif_ = true; }
    void desactiver() { actif_ = false; }

    bool estDisponible(const std::string& date) const;
    void afficher() const;

private:
    void validerPeriode() const;

    int id_ = 0;
    std::string nom_;
    std::string description_;
    TypeProgramme type_ = TypeProgramme::Performance;
    int pointsMaxParAttribution_ = 0;
    std::string dateDebut_;
    std::string dateFin_;
    bool actif_ = true;
};

class Recompense {
public:
    Recompense() = default;

    Recompense(int id,
               std::string nom,
               std::string description,
               int coutPoints,
               int stock);

    int getId() const { return id_; }
    const std::string& getNom() const { return nom_; }
    const std::string& getDescription() const { return description_; }
    int getCoutPoints() const { return coutPoints_; }
    int getStock() const { return stock_; }

    void setNom(const std::string& nom);
    void setDescription(const std::string& description);
    void setCoutPoints(int coutPoints);
    void setStock(int stock);

    bool estDisponible() const { return stock_ > 0; }

    void retirerDuStock();
    void reapprovisionner(int quantite);

    void afficher() const;

private:
    int id_ = 0;
    std::string nom_;
    std::string description_;
    int coutPoints_ = 0;
    int stock_ = 0;
};

struct AttributionRecompense {
    int id = 0;
    int idEmploye = 0;
    int idProgramme = 0;
    int points = 0;
    std::string motif;
    std::string date;
};

struct UtilisationRecompense {
    int id = 0;
    int idEmploye = 0;
    int idRecompense = 0;
    int points = 0;
    std::string date;
};

struct StatistiquesRecompenses {
    int totalProgrammes = 0;
    int programmesActifs = 0;
    int totalAttributions = 0;
    int pointsDistribues = 0;
    int totalUtilisations = 0;
    int pointsUtilises = 0;
    std::vector<std::pair<int, int>> topEmployes;
};

// --- Service principal ---
class RewardsProgramService : public Service {
public:
    RewardsProgramService(int id,
                          FournisseurEmployes fournisseur,
                          std::string cheminFichier = "data/recompenses.txt",
                          std::string dateReference = "");

    ~RewardsProgramService() override = default;

    // Programmes
    int ajouterProgramme(const std::string& nom,
                         const std::string& description,
                         TypeProgramme type,
                         int pointsMaxParAttribution,
                         const std::string& dateDebut,
                         const std::string& dateFin = "");

    void modifierProgramme(int idProgramme,
                           const std::string& nom,
                           const std::string& description,
                           TypeProgramme type,
                           int pointsMaxParAttribution,
                           const std::string& dateDebut,
                           const std::string& dateFin = "");

    void desactiverProgramme(int idProgramme);
    void activerProgramme(int idProgramme);

    const std::vector<ProgrammeRecompense>& programmes() const { return programmes_; }
    const ProgrammeRecompense* trouverProgramme(int idProgramme) const;

    // Catalogue
    int ajouterRecompense(const std::string& nom,
                          const std::string& description,
                          int coutPoints,
                          int stock);

    void modifierStock(int idRecompense, int nouveauStock);

    const std::vector<Recompense>& catalogue() const { return catalogue_; }
    const Recompense* trouverRecompense(int idRecompense) const;

    // Attribution & Utilisation
    int attribuerPoints(int idEmploye,
                        int idProgramme,
                        int points,
                        const std::string& motif);

    int utiliserRecompense(int idEmploye, int idRecompense);

    // Consultation
    int solde(int idEmploye) const;
    std::vector<AttributionRecompense> historique(int idEmploye) const;
    std::vector<UtilisationRecompense> utilisations(int idEmploye) const;
    StatistiquesRecompenses statistiques() const;
    void afficherStatistiques() const;

    // Persistance
    void charger();
    void sauvegarder() const;

    // Interface Service
    void display() const override;
    void execute() override;

private:
    void verifierEmploye(int idEmploye) const;
    ProgrammeRecompense& programmeModifiable(int idProgramme);
    Recompense& recompenseModifiable(int idRecompense);
    std::string dateDuJour() const;

    FournisseurEmployes fournisseur_;
    std::string cheminFichier_;
    std::string dateReference_;

    std::vector<ProgrammeRecompense> programmes_;
    std::vector<Recompense> catalogue_;
    std::vector<AttributionRecompense> attributions_;
    std::vector<UtilisationRecompense> utilisations_;

    int prochainIdProgramme_ = 1;
    int prochainIdRecompense_ = 1;
    int prochainIdAttribution_ = 1;
    int prochainIdUtilisation_ = 1;
};

#endif
