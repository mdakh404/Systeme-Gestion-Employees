#include "VueProfilEmploye.h"
#include "../01_Common/Navigation.h"
#include "../01_Common/ConsoleUI.h"
#include "../01_Common/InputValidator.h"
#include "../../include/BackendTypes.h"

void VueProfilEmploye::afficher(Navigation& navigation) {
    auto* ge = navigation.services().employes;
    if (!ge) {
        ConsoleUI::afficherErreur("Service employes non initialise.");
        ConsoleUI::pause();
        return;
    }

    while (true) {
        ConsoleUI::effacerEcran();
        ConsoleUI::afficherTitre("MON PROFIL");
        ConsoleUI::afficherInfo("1. Consulter mes informations");
        ConsoleUI::afficherInfo("2. Modifier email / telephone");
        ConsoleUI::afficherInfo("3. Historique");
        ConsoleUI::afficherInfo("0. Retour");

        const int choix = InputValidator::lireChoixMenu("Choix : ", 0, 3);
        if (choix == 0) break;

        try {
            int id = navigation.getEmployeId();
            if (choix == 1) {
                auto o = ge->rechercher(id);
                if (o) {
                    ConsoleUI::afficherInfo("Matricule : " + o->getMatricule());
                    ConsoleUI::afficherInfo("Nom : " + o->getNom() + " " + o->getPrenom());
                    ConsoleUI::afficherInfo("CIN : " + o->getCin());
                    ConsoleUI::afficherInfo("Email : " + o->getEmail());
                    ConsoleUI::afficherInfo("Telephone : " + o->getTelephone());
                    ConsoleUI::afficherInfo("Statut : " + o->getStatut());
                    ConsoleUI::afficherInfo("Departement : "
                        + integration::nomDepartement(navigation.services(), o->getIdDepartement()));
                    ConsoleUI::afficherInfo("Embauche : " + o->getDateEmbauche().toString());
                    ConsoleUI::afficherInfo("Salaire base : "
                        + std::to_string(o->getSalaire().getSalaireBase()));
                } else {
                    ConsoleUI::afficherInfo("Login session : " + navigation.getIdentifiantActuel());
                    ConsoleUI::afficherInfo("ID session : " + std::to_string(id));
                    ConsoleUI::afficherInfo("(Fiche employe introuvable)");
                }
                ConsoleUI::pause();
            } else if (choix == 2) {
                if (!ge->rechercher(id)) {
                    ConsoleUI::afficherErreur("Fiche employe introuvable pour modification.");
                    ConsoleUI::pause();
                    continue;
                }
                std::string email = InputValidator::lireChaine("Nouvel email : ");
                std::string tel = InputValidator::lireChaine("Nouveau telephone : ");
                ge->modifierCoordonnees(id, email, tel);
                integration::sauvegarderEmployes(navigation.services());
                ConsoleUI::afficherSucces("Coordonnees mises a jour.");
                ConsoleUI::pause();
            } else if (choix == 3) {
                if (!ge->rechercher(id)) {
                    ConsoleUI::afficherInfo("(pas d'historique)");
                    ConsoleUI::pause();
                    continue;
                }
                for (const auto& l : ge->historique(id))
                    ConsoleUI::afficherInfo("- " + l);
                ConsoleUI::pause();
            }
        } catch (const std::exception& e) {
            ConsoleUI::afficherErreur(e.what());
            ConsoleUI::pause();
        }
    }
}
