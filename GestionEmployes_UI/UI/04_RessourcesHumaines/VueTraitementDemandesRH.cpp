#include "VueTraitementDemandesRH.h"
#include "../01_Common/Navigation.h"
#include "../01_Common/ConsoleUI.h"
#include "../01_Common/InputValidator.h"
#include "../../include/BackendTypes.h"

void VueTraitementDemandesRH::afficher(Navigation& navigation) {
    auto* gd = navigation.services().demandes;
    auto* gc = navigation.services().conges;
    if (!gd || !gc) {
        ConsoleUI::afficherErreur("Services demandes/conges non initialises.");
        ConsoleUI::pause();
        return;
    }

    while (true) {
        ConsoleUI::effacerEcran();
        ConsoleUI::afficherTitre("TRAITEMENT RH");
        ConsoleUI::afficherInfo("1. Conges en attente");
        ConsoleUI::afficherInfo("2. Traiter un conge (accepter/refuser)");
        ConsoleUI::afficherInfo("3. Demandes administratives en attente");
        ConsoleUI::afficherInfo("4. Traiter une demande administrative (accepter/refuser)");
        ConsoleUI::afficherInfo("0. Retour");

        const int choix = InputValidator::lireChoixMenu("Choix : ", 0, 4);
        if (choix == 0) break;

        try {
            if (choix == 1) {
                std::vector<std::string> h = {"ID", "Employe", "Debut", "Fin", "Jours", "Type", "Statut"};
                std::vector<std::vector<std::string>> lignes;
                for (const auto& c : gc->listerDemandesEnAttente())
                    lignes.push_back({std::to_string(c.Getid()), std::to_string(c.GetidEmploye()),
                                      integration::dateIsoVersFr(c.GetdateDebut()),
                                      integration::dateIsoVersFr(c.GetdateFin()),
                                      std::to_string(gc->dureeJours(c)), c.Gettype(), c.Getstatut()});
                ConsoleUI::afficherTableau(h, lignes);
            } else if (choix == 2) {
                int id = InputValidator::lireEntier("ID conge : ");
                bool ok = InputValidator::lireConfirmation("Accepter ce conge (N = refuser)");
                integration::traiterConge(navigation.services(), id, ok);
                ConsoleUI::afficherSucces(ok ? "Conge accepte." : "Conge refuse.");
            } else if (choix == 3) {
                std::vector<std::string> h = {"ID", "Employe", "Type", "Description", "Depot", "Statut"};
                std::vector<std::vector<std::string>> lignes;
                for (const auto& d : gd->listerEnAttente())
                    lignes.push_back({std::to_string(d.getId()), std::to_string(d.getIdEmploye()),
                                      d.getType(), d.getDescription(), d.getDateDepot().toString(),
                                      d.getStatut()});
                ConsoleUI::afficherTableau(h, lignes);
            } else if (choix == 4) {
                int id = InputValidator::lireEntier("ID demande : ");
                bool ok = InputValidator::lireConfirmation("Accepter cette demande (N = refuser)");
                integration::traiterDemande(navigation.services(), id, ok);
                ConsoleUI::afficherSucces(ok ? "Demande acceptee." : "Demande refusee.");
            }
            ConsoleUI::pause();
        } catch (const std::exception& e) {
            ConsoleUI::afficherErreur(e.what());
            ConsoleUI::pause();
        }
    }
}
