#include "MenuDemandes.h"
#include "../01_Common/Navigation.h"
#include "../01_Common/ConsoleUI.h"
#include "../01_Common/InputValidator.h"
#include "../../include/BackendTypes.h"

namespace {
void afficherDemandes(const std::vector<DemandeAdministrative>& liste, bool avecEmploye) {
    std::vector<std::string> h;
    h.push_back("ID");
    if (avecEmploye) h.push_back("Employe");
    for (const char* t : {"Type", "Description", "Depot", "Statut"}) h.push_back(t);
    std::vector<std::vector<std::string>> lignes;
    for (const auto& d : liste) {
        std::vector<std::string> l;
        l.push_back(std::to_string(d.getId()));
        if (avecEmploye) l.push_back(std::to_string(d.getIdEmploye()));
        l.push_back(d.getType());
        l.push_back(d.getDescription());
        l.push_back(d.getDateDepot().toString());
        l.push_back(d.getStatut());
        lignes.push_back(l);
    }
    ConsoleUI::afficherTableau(h, lignes);
}
} // namespace

void MenuDemandesUI::afficher(Navigation& navigation, bool modeRH) {
    auto* gd = navigation.services().demandes;
    if (!gd) {
        ConsoleUI::afficherErreur("Service demandes non initialise.");
        ConsoleUI::pause();
        return;
    }

    while (true) {
        ConsoleUI::effacerEcran();
        ConsoleUI::afficherTitre(modeRH ? "DEMANDES ADMINISTRATIVES (RH)" : "MES DEMANDES");
        if (modeRH) {
            ConsoleUI::afficherInfo("1. Lister toutes les demandes");
            ConsoleUI::afficherInfo("2. Lister les demandes en attente");
            ConsoleUI::afficherInfo("3. Traiter une demande (accepter/refuser)");
            ConsoleUI::afficherInfo("0. Retour");
            const int choix = InputValidator::lireChoixMenu("Choix : ", 0, 3);
            if (choix == 0) break;
            try {
                if (choix == 1) {
                    afficherDemandes(gd->lister(), true);
                } else if (choix == 2) {
                    afficherDemandes(gd->listerEnAttente(), true);
                } else if (choix == 3) {
                    int id = InputValidator::lireEntier("ID demande : ");
                    bool ok = InputValidator::lireConfirmation("Accepter (N = refuser)");
                    integration::traiterDemande(navigation.services(), id, ok);
                    ConsoleUI::afficherSucces(ok ? "Demande acceptee." : "Demande refusee.");
                }
                ConsoleUI::pause();
            } catch (const std::exception& e) {
                ConsoleUI::afficherErreur(e.what());
                ConsoleUI::pause();
            }
        } else {
            ConsoleUI::afficherInfo("1. Deposer une demande");
            ConsoleUI::afficherInfo("2. Consulter mes demandes");
            ConsoleUI::afficherInfo("0. Retour");
            const int choix = InputValidator::lireChoixMenu("Choix : ", 0, 2);
            if (choix == 0) break;
            try {
                int idEmp = navigation.getEmployeId();
                if (choix == 1) {
                    std::string type = InputValidator::lireChoixListe(
                        "Type de demande :", DemandeAdministrative::typesProposes());
                    std::string description = InputValidator::lireChaine("Description (3-200 caracteres) : ");
                    int id = integration::deposerDemande(navigation.services(), idEmp, type, description);
                    ConsoleUI::afficherSucces("Demande deposee (statut : En attente). ID = " + std::to_string(id));
                } else if (choix == 2) {
                    afficherDemandes(gd->listerParEmploye(idEmp), false);
                }
                ConsoleUI::pause();
            } catch (const std::exception& e) {
                ConsoleUI::afficherErreur(e.what());
                ConsoleUI::pause();
            }
        }
    }
}
