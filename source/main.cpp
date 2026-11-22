#include "EspaceEmploye.h"
#include "EspaceRH.h"
#include "GestionnaireEmployes.h"
#include "GestionnaireDemandes.h"
#include "GestionnaireFichiersDemandes.h"
#include "MenuDemandes.h"
#include "GestionnaireConges.h"
#include "MenuConge.h"
#include "GestionnaireAbsences.h"
#include "GestionnaireFichiersAbsences.h"
#include "MenuAbsences.h"
#include "PrimeService.h"
#include "RetirementService.h"
#include "TrainingService.h"
#include "admin/EspaceAdmin.h"

#include <exception>
#include <iostream>
#include <string>

int main() {
    // 1. Instanciation des gestionnaires métier
    GestionnaireEmployes gestion;
    try {
        gestion.chargerDepuisFichier("data/employes.txt");
    } catch (const std::exception& ex) {
        std::cerr << "Employes : " << ex.what() << "\n";
    }
    GestionnaireDemandes gestionDemandes;
    GestionnaireFichiersDemandes fichiersDemandes;
    MenuDemandes menuDemandes(gestionDemandes, fichiersDemandes);

    GestionnaireConges gestionConges;
    MenuConge menuConge(gestionConges);

    GestionnaireFichiersAbsences fichiersAbsences("data/absences.txt");
    GestionnaireAbsences gestionAbsences([&gestion](int id) { return gestion.rechercher(id) != nullptr; });
    try {
        gestionAbsences = fichiersAbsences.chargerAbsences(
            [&gestion](int id) { return gestion.rechercher(id) != nullptr; }
        );
    } catch (const std::exception& ex) {
        std::cerr << "Absences : " << ex.what() << "\n";
    }
    MenuAbsences menuAbsences(gestionAbsences, fichiersAbsences);

    PrimeService primeService(1, gestion);
    RetirementService retirementService(2, "Service Retraite", "Gestion des departs en retraite", 60, 15, 0.02, 0.80, &gestion);
    TrainingService trainingService(3);

    // 2. Chargement des donnees persistees
    try {
        gestionDemandes.charger(fichiersDemandes.lireDemandes());
    } catch (const std::exception& ex) {
        std::cerr << "Demandes : " << ex.what() << "\n";
    }

    try {
        gestionConges.chargerDepuisFichier();
    } catch (const std::exception& ex) {
        std::cerr << "Conges : " << ex.what() << "\n";
    }

    try {
        trainingService.loadFromFile("data/formations.txt");
    } catch (const std::exception& ex) {
        std::cerr << "Formations : " << ex.what() << "\n";
    }

    // 3. Boucle principale de l'application
    while (true) {
        std::cout << "\n============================================\n"
                  << "  SYSTEME DE GESTION DES EMPLOYES (EMS C++)\n"
                  << "============================================\n"
                  << "1. Espace Administrateur\n"
                  << "2. Espace Responsable RH\n"
                  << "3. Espace Employe\n"
                  << "0. Quitter\n"
                  << "Choix : ";

        std::string choix;
        if (!std::getline(std::cin, choix)) break;

        try {
            if (choix == "0") break;
            if (choix == "1") {
                EspaceAdmin admin(gestion);
                admin.menuAdmin();
            } else if (choix == "2") {
                EspaceRH rh(gestion, menuDemandes, menuConge, menuAbsences, primeService, retirementService, trainingService);
                rh.executer();
            } else if (choix == "3") {
                EspaceEmploye espace(gestion, menuDemandes, menuConge);
                espace.executer();
            } else {
                std::cout << "Choix invalide.\n";
            }
        } catch (const std::exception& ex) {
            std::cerr << "Erreur : " << ex.what() << "\n";
        }
    }

    // 4. Sauvegarde automatique avant fermeture
    try {
        gestion.sauvegarderDansFichier("data/employes.txt");
        gestionConges.sauvegarderDansFichier();
        fichiersAbsences.sauvegarderAbsences(gestionAbsences);
        trainingService.saveToFile("data/formations.txt");
    } catch (const std::exception& ex) {
        std::cerr << "Erreur lors de la sauvegarde : " << ex.what() << "\n";
    }

    std::cout << "\nFin du programme. Au revoir !\n";
    return 0;
}
