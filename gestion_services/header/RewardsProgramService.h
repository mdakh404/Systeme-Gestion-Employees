#ifndef REWARDSPROGRAMSERVICE_H
#define REWARDSPROGRAMSERVICE_H

#include "EmployeInfo.h"
#include "ProgrammeRecompense.h"
#include "Recompense.h"
#include "Service.h"

#include <string>
#include <utility>
#include <vector>

// Une attribution de points (registre en ajout seul : jamais modifiee).
struct AttributionRecompense {
    int id = 0;
    int idEmploye = 0;
    int idProgramme = 0;
    int points = 0;
    std::string motif;
    std::string date;
};

// Une utilisation de points contre un article du catalogue.
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
    std::vector<std::pair<int, int>> topEmployes; // (idEmploye, points nets), tries
};

// Service "Rewards Program" : programmes de recompenses, catalogue d'articles,
// attribution de points et utilisation des points.
//
// Le service ne connait pas la classe Employe : il recoit un FournisseurEmployes.
// Voir EmployeInfo.h et le README pour le raccordement au module Employes.
class RewardsProgramService : public Service {
public:
    RewardsProgramService(int id,
                          FournisseurEmployes fournisseur,
                          std::string cheminFichier = "data/recompenses.txt",
                          std::string dateReference = "");

    ~RewardsProgramService() override = default;

    // --- Programmes ---
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

    // --- Catalogue ---
    int ajouterRecompense(const std::string& nom,
                          const std::string& description,
                          int coutPoints,
                          int stock);

    void modifierStock(int idRecompense, int nouveauStock);

    const std::vector<Recompense>& catalogue() const { return catalogue_; }
    const Recompense* trouverRecompense(int idRecompense) const;

    // --- Attribution / utilisation ---
    int attribuerPoints(int idEmploye,
                        int idProgramme,
                        int points,
                        const std::string& motif);

    int utiliserRecompense(int idEmploye, int idRecompense);

    // --- Consultation ---
    int solde(int idEmploye) const;
    std::vector<AttributionRecompense> historique(int idEmploye) const;
    std::vector<UtilisationRecompense> utilisations(int idEmploye) const;
    StatistiquesRecompenses statistiques() const;
    void afficherStatistiques() const;

    // --- Persistance ---
    void charger();
    void sauvegarder() const;

    // --- Interface Service ---
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
