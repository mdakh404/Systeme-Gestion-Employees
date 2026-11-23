#include "MenuEmploye.h"
#include "VueProfilEmploye.h"
#include "../01_Common/Navigation.h"
#include "../01_Common/ConsoleUI.h"
#include "../01_Common/InputValidator.h"
#include "../05_Conges_Absences/MenuConge.h"
#include "../05_Conges_Absences/MenuAbsences.h"
#include "../07_Rapports_Demandes/MenuDemandes.h"

void MenuEmploye::afficher(Navigation& navigation) {
    VueProfilEmploye vueProfil;
    MenuCongeUI menuConge;
    MenuAbsencesUI menuAbsences;
    MenuDemandesUI menuDemandes;

    while (true) {
        ConsoleUI::effacerEcran();
        ConsoleUI::afficherTitre("ESPACE EMPLOYE");
        ConsoleUI::afficherInfo("Connecte : " + navigation.getIdentifiantActuel()
            + " (ID " + std::to_string(navigation.getEmployeId()) + ")");
        ConsoleUI::afficherInfo("1. Mon profil");
        ConsoleUI::afficherInfo("2. Mes conges");
        ConsoleUI::afficherInfo("3. Mes absences");
        ConsoleUI::afficherInfo("4. Mes demandes administratives");
        ConsoleUI::afficherInfo("0. Deconnexion");

        const int choix = InputValidator::lireChoixMenu("Choix : ", 0, 4);
        if (choix == 0) {
            navigation.reinitialiserSession();
            break;
        }
        if (choix == 1) vueProfil.afficher(navigation);
        else if (choix == 2) menuConge.afficher(navigation, false);
        else if (choix == 3) menuAbsences.afficher(navigation, false);
        else if (choix == 4) menuDemandes.afficher(navigation, false);
    }
}
