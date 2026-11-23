#pragma once
// ---------------------------------------------------------------------------
// Point d'entree unique de l'UI vers le back-end REEL
// (depot Systeme-Gestion-Employees, branche main).
//
// Les vues UI n'incluent que ce fichier. Il expose :
//   - les vrais en-tetes du back-end (aucune copie, aucun stub) ;
//   - les deux briques encore absentes de `main` (departements, auth),
//     isolees dans include/integration/ ;
//   - BackendServices : les instances injectees depuis main.cpp.
// ---------------------------------------------------------------------------

// ---- Back-end reel ----
#include "Date.h"
#include "Exceptions.h"
#include "ExceptionsDemandes.h"
#include "Employe.h"
#include "GestionnaireEmployes.h"
#include "Departement.h"
#include "Conge.h"
#include "GestionnaireConges.h"
#include "Absence.h"
#include "GestionnaireAbsences.h"
#include "GestionnaireFichiersAbsences.h"
#include "DemandeAdministrative.h"
#include "GestionnaireDemandes.h"
#include "GestionnaireFichiersDemandes.h"
#include "PrimeService.h"

// ---- Briques manquantes dans le back-end main (temporaires) ----
#include "integration/GestionnaireDepartements.h"
#include "integration/AuthService.h"

// Conteneur des services (rempli une seule fois dans main.cpp)
struct BackendServices {
    GestionnaireDepartements* departements = nullptr;
    GestionnaireEmployes* employes = nullptr;
    GestionnaireConges* conges = nullptr;
    GestionnaireAbsences* absences = nullptr;
    GestionnaireDemandes* demandes = nullptr;
    PrimeService* primes = nullptr;

    // Persistance (le back-end ne sauvegarde pas tout seul absences / demandes)
    const GestionnaireFichiersAbsences* fichiersAbsences = nullptr;
    const GestionnaireFichiersDemandes* fichiersDemandes = nullptr;

    AuthService* auth = nullptr;
};

#include "integration/Operations.h"
