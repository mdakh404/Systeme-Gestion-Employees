#pragma once
#include "GestionnaireEmployes.h"
#include "MenuDemandes.h"
#include "MenuConge.h"
#include "MenuAbsences.h"
#include "PrimeService.h"
#include "RetirementService.h"
#include "TrainingService.h"

// Espace Responsable RH - couche 1 (presentation).
// Regroupe le traitement des demandes administratives, conges, absences et services RH.
class EspaceRH {
    GestionnaireEmployes& gestion_;
    MenuDemandes& demandes_;
    MenuConge& conges_;
    MenuAbsences& absences_;
    PrimeService& primes_;
    RetirementService& retraite_;
    TrainingService& formations_;

    void menuServices();
    void menuConnecte();

public:
    explicit EspaceRH(
        GestionnaireEmployes& gestion,
        MenuDemandes& demandes,
        MenuConge& conges,
        MenuAbsences& absences,
        PrimeService& primes,
        RetirementService& retraite,
        TrainingService& formations
    );

    void executer();
};
