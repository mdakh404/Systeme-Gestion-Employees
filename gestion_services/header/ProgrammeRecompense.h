#ifndef PROGRAMMERECOMPENSE_H
#define PROGRAMMERECOMPENSE_H

#include <string>

// Nature du programme. Le texte est celui stocke dans le fichier de donnees.
enum class TypeProgramme {
    Performance,
    Fidelite,
    Reconnaissance
};

std::string toString(TypeProgramme type);
TypeProgramme typeDepuisTexte(const std::string& texte);

// Un programme de recompenses : un cadre dans lequel le RH attribue des points
// (par exemple "Performance Q4"). Le programme definit le nombre maximum de
// points accordables en une seule attribution et sa periode de validite.
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

    // Disponible = drapeau actif ET date dans [dateDebut, dateFin].
    // Une date de fin vide signifie "pas de limite".
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

#endif
