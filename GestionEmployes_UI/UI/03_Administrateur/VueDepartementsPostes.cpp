#include "VueDepartementsPostes.h"
#include "../01_Common/Navigation.h"
#include "../01_Common/ConsoleUI.h"
#include "../01_Common/InputValidator.h"
#include "../../include/BackendTypes.h"

void VueDepartementsPostes::afficher(Navigation& navigation) {
    auto* gd = navigation.services().departements;
    auto* ge = navigation.services().employes;
    if (!gd || !ge) {
        ConsoleUI::afficherErreur("Service departements non initialise.");
        ConsoleUI::pause();
        return;
    }

    while (true) {
        ConsoleUI::effacerEcran();
        ConsoleUI::afficherTitre("DEPARTEMENTS");
        ConsoleUI::afficherInfo("1. Lister");
        ConsoleUI::afficherInfo("2. Ajouter");
        ConsoleUI::afficherInfo("3. Modifier");
        ConsoleUI::afficherInfo("4. Supprimer");
        ConsoleUI::afficherInfo("0. Retour");

        const int choix = InputValidator::lireChoixMenu("Choix : ", 0, 4);
        if (choix == 0) break;

        try {
            if (choix == 1) {
                auto liste = gd->lister();
                std::vector<std::string> enTetes = {"ID", "Nom", "Description", "Effectif"};
                std::vector<std::vector<std::string>> lignes;
                for (const auto& d : liste)
                    lignes.push_back({std::to_string(d.getId()), d.getNom(), d.getDescription(),
                                      std::to_string(ge->effectifDepartement(d.getId()))});
                ConsoleUI::afficherTableau(enTetes, lignes);
                ConsoleUI::pause();
            } else if (choix == 2) {
                std::string nom = InputValidator::lireChaine("Nom : ");
                std::string desc = InputValidator::lireChaine("Description : ", true);
                int id = gd->ajouter(nom, desc);
                integration::sauvegarderDepartements(navigation.services());
                ConsoleUI::afficherSucces("Departement cree. ID = " + std::to_string(id));
                ConsoleUI::pause();
            } else if (choix == 3) {
                int id = InputValidator::lireEntier("ID : ");
                std::string nom = InputValidator::lireChaine("Nouveau nom : ");
                std::string desc = InputValidator::lireChaine("Nouvelle description : ", true);
                gd->modifier(id, nom, desc);
                integration::sauvegarderDepartements(navigation.services());
                ConsoleUI::afficherSucces("Departement modifie.");
                ConsoleUI::pause();
            } else if (choix == 4) {
                int id = InputValidator::lireEntier("ID a supprimer : ");
                if (InputValidator::lireConfirmation("Confirmer")) {
                    gd->supprimer(id, *ge); // refuse si des employes y sont affectes
                    integration::sauvegarderDepartements(navigation.services());
                    ConsoleUI::afficherSucces("Departement supprime.");
                }
                ConsoleUI::pause();
            }
        } catch (const std::exception& e) {
            ConsoleUI::afficherErreur(e.what());
            ConsoleUI::pause();
        }
    }
}
