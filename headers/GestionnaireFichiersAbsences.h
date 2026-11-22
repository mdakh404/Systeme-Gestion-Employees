#ifndef GESTION_ABSENCES_GESTIONNAIRE_FICHIERS_ABSENCES_H
#define GESTION_ABSENCES_GESTIONNAIRE_FICHIERS_ABSENCES_H

#include "GestionnaireAbsences.h"

#include <string>
#include <vector>

class GestionnaireFichiersAbsences {
public:
    explicit GestionnaireFichiersAbsences(const std::string& chemin = "data/absences.txt");

    void sauvegarderAbsences(const GestionnaireAbsences& gestionnaire) const;
    void sauvegarderAbsences(const GestionnaireAbsences& gestionnaire,
                             const GestionnaireAbsences& etatInitial) const;
    void sauvegarderSuppression(const GestionnaireAbsences& gestionnaire,
                                const GestionnaireAbsences& etatInitial, int idSupprime) const;
    GestionnaireAbsences chargerAbsences() const;
    GestionnaireAbsences chargerAbsences(GestionnaireAbsences::VerificationEmploye employeExiste) const;

private:
    GestionnaireAbsences charger(std::vector<std::string>* lignesOriginales) const;
    void sauvegarder(const GestionnaireAbsences& gestionnaire,
                     const GestionnaireAbsences* etatInitial, int idSupprime = 0) const;
    std::string chemin;
};

#endif
