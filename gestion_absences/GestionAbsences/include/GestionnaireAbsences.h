#ifndef GESTION_ABSENCES_GESTIONNAIRE_ABSENCES_H
#define GESTION_ABSENCES_GESTIONNAIRE_ABSENCES_H

#include "Absence.h"

#include <functional>
#include <optional>
#include <string>
#include <vector>

class GestionnaireAbsences {
public:
    using VerificationEmploye = std::function<bool(int)>;

    GestionnaireAbsences() = default;
    explicit GestionnaireAbsences(const std::string& dateReference);
    explicit GestionnaireAbsences(VerificationEmploye employeExiste);
    GestionnaireAbsences(const std::string& dateReference, VerificationEmploye employeExiste);
    // Retourne false en cas de doublon. Une entree invalide leve std::invalid_argument.
    bool enregistrerAbsence(int idEmploye, const std::string& dateAbsence,
                            const std::string& motif, bool justifiee);

    bool modifierAbsence(int idAbsence, const std::string& nouvelleDate,
                         const std::string& nouveauMotif, bool nouvelleJustification);
    void annulerAbsence(int idAbsence);
    bool supprimerAbsence(int idAbsence);

    const std::vector<Absence>& consulterAbsences() const;
    std::vector<Absence> consulterAbsencesActives() const;
    std::vector<Absence> consulterHistorique(int idEmploye) const;
    std::optional<Absence> rechercherAbsence(int idAbsence) const;

private:
    friend class GestionnaireFichiersAbsences;

    static std::string normaliserMotif(const std::string& motif);
    void verifierEmploye(int idEmploye) const;
    void verifierDelais(const std::string& dateAbsence, bool justifiee) const;
    bool contientDoublon(int idEmploye, const std::string& dateAbsence, int idIgnore = 0) const;
    Absence& trouverActive(int idAbsence);
    void restaurerAbsence(Absence absence);

    std::vector<Absence> absences;
    int dernierIdAttribue = 0;
    std::optional<int> jourReference;
    VerificationEmploye employeExiste;
};

#endif
