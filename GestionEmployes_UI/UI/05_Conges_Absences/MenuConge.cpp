#include "MenuConge.h"
#include "../01_Common/Navigation.h"
#include "../01_Common/ConsoleUI.h"
#include "../01_Common/InputValidator.h"
#include "../../include/BackendTypes.h"

namespace {
// Dates stockees en AAAA-MM-JJ par le back-end, affichees en JJ/MM/AAAA.
std::vector<std::string> ligneConge(const Conge& c, const GestionnaireConges& gc, bool avecEmploye) {
    std::vector<std::string> l;
    l.push_back(std::to_string(c.Getid()));
    if (avecEmploye) l.push_back(std::to_string(c.GetidEmploye()));
    l.push_back(integration::dateIsoVersFr(c.GetdateDebut()));
    l.push_back(integration::dateIsoVersFr(c.GetdateFin()));
    l.push_back(std::to_string(gc.dureeJours(c)));
    l.push_back(c.Gettype());
    l.push_back(c.Getstatut());
    return l;
}

void afficherConges(const std::vector<Conge>& liste, const GestionnaireConges& gc, bool avecEmploye) {
    std::vector<std::string> h;
    h.push_back("ID");
    if (avecEmploye) h.push_back("Employe");
    for (const char* t : {"Debut", "Fin", "Jours", "Type", "Statut"}) h.push_back(t);
    std::vector<std::vector<std::string>> lignes;
    for (const auto& c : liste) lignes.push_back(ligneConge(c, gc, avecEmploye));
    ConsoleUI::afficherTableau(h, lignes);
}
} // namespace

void MenuCongeUI::afficher(Navigation& navigation, bool modeRH) {
    auto* gc = navigation.services().conges;
    if (!gc) {
        ConsoleUI::afficherErreur("Service conges non initialise.");
        ConsoleUI::pause();
        return;
    }

    while (true) {
        ConsoleUI::effacerEcran();
        ConsoleUI::afficherTitre(modeRH ? "CONGES (RH)" : "MES CONGES");
        if (modeRH) {
            ConsoleUI::afficherInfo("1. Lister tous les conges");
            ConsoleUI::afficherInfo("2. Lister les demandes en attente");
            ConsoleUI::afficherInfo("3. Traiter une demande (accepter/refuser)");
            ConsoleUI::afficherInfo("4. Bilan d'un employe");
            ConsoleUI::afficherInfo("0. Retour");
            const int choix = InputValidator::lireChoixMenu("Choix : ", 0, 4);
            if (choix == 0) break;
            try {
                if (choix == 1) {
                    afficherConges(gc->tousLesConges(), *gc, true);
                } else if (choix == 2) {
                    afficherConges(gc->listerDemandesEnAttente(), *gc, true);
                } else if (choix == 3) {
                    int id = InputValidator::lireEntier("ID conge : ");
                    bool ok = InputValidator::lireConfirmation("Accepter (N = refuser)");
                    integration::traiterConge(navigation.services(), id, ok);
                    ConsoleUI::afficherSucces(ok ? "Conge accepte." : "Conge refuse.");
                } else if (choix == 4) {
                    int idEmp = InputValidator::lireEntier("ID employe : ");
                    afficherConges(gc->consulterHistorique(idEmp), *gc, false);
                    const int annee = GestionnaireConges::anneeCourante();
                    ConsoleUI::afficherInfo("Solde annuel restant " + std::to_string(annee) + " : "
                        + std::to_string(gc->soldeRestant(idEmp, annee)) + " / "
                        + std::to_string(GestionnaireConges::QUOTA_ANNUEL) + " jours");
                }
                ConsoleUI::pause();
            } catch (const std::exception& e) {
                ConsoleUI::afficherErreur(e.what());
                ConsoleUI::pause();
            }
        } else {
            ConsoleUI::afficherInfo("1. Deposer une demande de conge");
            ConsoleUI::afficherInfo("2. Consulter mon historique et mon solde");
            ConsoleUI::afficherInfo("0. Retour");
            const int choix = InputValidator::lireChoixMenu("Choix : ", 0, 2);
            if (choix == 0) break;
            try {
                int idEmp = navigation.getEmployeId();
                if (idEmp <= 0) {
                    ConsoleUI::afficherErreur("Session employe invalide.");
                    ConsoleUI::pause();
                    continue;
                }
                if (choix == 1) {
                    std::string debut = InputValidator::lireDate("Date debut (JJ/MM/AAAA) : ");
                    std::string fin = InputValidator::lireDate("Date fin (JJ/MM/AAAA) : ");
                    std::string type = InputValidator::lireChoixListe("Type de conge :", integration::typesConge());
                    int id = integration::deposerConge(navigation.services(), idEmp, debut, fin, type);
                    ConsoleUI::afficherSucces("Demande deposee (statut : En attente). ID = " + std::to_string(id));
                } else if (choix == 2) {
                    afficherConges(gc->consulterHistorique(idEmp), *gc, false);
                    const int annee = GestionnaireConges::anneeCourante();
                    ConsoleUI::afficherInfo("Solde annuel restant " + std::to_string(annee) + " : "
                        + std::to_string(gc->soldeRestant(idEmp, annee)) + " jours");
                }
                ConsoleUI::pause();
            } catch (const std::exception& e) {
                ConsoleUI::afficherErreur(e.what());
                ConsoleUI::pause();
            }
        }
    }
}
