#ifndef GESTION_ABSENCES_MENU_ABSENCES_H
#define GESTION_ABSENCES_MENU_ABSENCES_H

#include "GestionnaireAbsences.h"

#include <functional>
#include <optional>
#include <string>

class GestionnaireFichiersAbsences;

class MenuAbsences {
public:
    // Un résultat vide signale un doublon. Les autres erreurs sont signalées par exception.
    using Enregistrement = std::function<std::optional<int>(
        int, const std::string&, const std::string&, bool)>;

    using Modification = std::function<bool(int, const std::string&, const std::string&, bool)>;
    using Annulation = std::function<void(int)>;
    using Suppression = std::function<bool(int)>;

    MenuAbsences(const GestionnaireAbsences& gestionnaire, Enregistrement enregistrer,
                 Modification modifier, Annulation annuler, Suppression supprimer);
    MenuAbsences(GestionnaireAbsences& gestionnaire, const GestionnaireFichiersAbsences& fichier);
    void executer() const;

private:
    void saisirAbsence() const;
    void afficherAbsences() const;
    void afficherHistorique() const;
    void rechercherAbsence() const;
    void corrigerAbsence() const;
    void annulerAbsence() const;
    void supprimerAbsence() const;

    const GestionnaireAbsences& gestionnaire;
    Enregistrement enregistrer;
    Modification modifier;
    Annulation annuler;
    Suppression supprimer;
};

#endif
