#include "MenuAbsences.h"
#include "../01_Common/Navigation.h"
#include "../01_Common/ConsoleUI.h"
#include "../01_Common/InputValidator.h"
#include "../../include/BackendTypes.h"

namespace {
void afficherAbsences(const std::vector<Absence>& liste, bool avecEmploye) {
    std::vector<std::string> h;
    h.push_back("ID");
    if (avecEmploye) h.push_back("Employe");
    for (const char* t : {"Date", "Motif", "Justifiee", "Etat"}) h.push_back(t);
    std::vector<std::vector<std::string>> lignes;
    for (const auto& a : liste) {
        std::vector<std::string> l;
        l.push_back(std::to_string(a.getId()));
        if (avecEmploye) l.push_back(std::to_string(a.getIdEmploye()));
        l.push_back(a.getDateAbsence());
        l.push_back(a.getMotif());
        l.push_back(a.estJustifiee() ? "oui" : "non");
        l.push_back(a.estAnnulee() ? "annulee" : "active");
        lignes.push_back(l);
    }
    ConsoleUI::afficherTableau(h, lignes);
}
} // namespace

void MenuAbsencesUI::afficher(Navigation& navigation, bool modeRH) {
    auto* ga = navigation.services().absences;
    if (!ga) {
        ConsoleUI::afficherErreur("Service absences non initialise.");
        ConsoleUI::pause();
        return;
    }
    const BackendServices& services = navigation.services();

    while (true) {
        ConsoleUI::effacerEcran();
        ConsoleUI::afficherTitre(modeRH ? "ABSENCES (RH)" : "MES ABSENCES");
        if (modeRH) {
            ConsoleUI::afficherInfo("1. Lister les absences actives");
            ConsoleUI::afficherInfo("2. Historique d'un employe");
            ConsoleUI::afficherInfo("3. Enregistrer une absence");
            ConsoleUI::afficherInfo("4. Corriger une absence");
            ConsoleUI::afficherInfo("5. Annuler une absence");
            ConsoleUI::afficherInfo("6. Supprimer une absence");
            ConsoleUI::afficherInfo("0. Retour");
            const int choix = InputValidator::lireChoixMenu("Choix : ", 0, 6);
            if (choix == 0) break;
            try {
                if (choix == 1) {
                    afficherAbsences(ga->consulterAbsencesActives(), true);
                } else if (choix == 2) {
                    int idEmp = InputValidator::lireEntier("ID employe : ");
                    afficherAbsences(ga->consulterHistorique(idEmp), false);
                } else if (choix == 3) {
                    int idEmp = InputValidator::lireEntier("ID employe : ");
                    std::string date = InputValidator::lireDate("Date (JJ/MM/AAAA) : ");
                    std::string motif = InputValidator::lireChaine("Motif : ", true);
                    bool just = InputValidator::lireConfirmation("Justifiee");
                    auto id = integration::enregistrerAbsence(services, idEmp, date, motif, just);
                    if (id) ConsoleUI::afficherSucces("Absence enregistree. ID = " + std::to_string(*id));
                    else ConsoleUI::afficherErreur("Absence deja enregistree pour cet employe a cette date.");
                } else if (choix == 4) {
                    int id = InputValidator::lireEntier("ID absence : ");
                    std::string date = InputValidator::lireDate("Nouvelle date (JJ/MM/AAAA) : ");
                    std::string motif = InputValidator::lireChaine("Nouveau motif : ", true);
                    bool just = InputValidator::lireConfirmation("Justifiee");
                    if (integration::modifierAbsence(services, id, date, motif, just))
                        ConsoleUI::afficherSucces("Absence corrigee.");
                    else ConsoleUI::afficherErreur("Correction refusee (doublon a cette date).");
                } else if (choix == 5) {
                    int id = InputValidator::lireEntier("ID absence : ");
                    if (InputValidator::lireConfirmation("Confirmer l'annulation")) {
                        integration::annulerAbsence(services, id);
                        ConsoleUI::afficherSucces("Absence annulee (conservee dans l'historique).");
                    }
                } else if (choix == 6) {
                    int id = InputValidator::lireEntier("ID absence : ");
                    if (InputValidator::lireConfirmation("Confirmer la suppression definitive")) {
                        if (integration::supprimerAbsence(services, id)) ConsoleUI::afficherSucces("Supprimee.");
                        else ConsoleUI::afficherErreur("Introuvable.");
                    }
                }
                ConsoleUI::pause();
            } catch (const std::exception& e) {
                ConsoleUI::afficherErreur(e.what());
                ConsoleUI::pause();
            }
        } else {
            ConsoleUI::afficherInfo("1. Consulter mes absences");
            ConsoleUI::afficherInfo("0. Retour");
            const int choix = InputValidator::lireChoixMenu("Choix : ", 0, 1);
            if (choix == 0) break;
            try {
                afficherAbsences(ga->consulterHistorique(navigation.getEmployeId()), false);
                ConsoleUI::pause();
            } catch (const std::exception& e) {
                ConsoleUI::afficherErreur(e.what());
                ConsoleUI::pause();
            }
        }
    }
}
