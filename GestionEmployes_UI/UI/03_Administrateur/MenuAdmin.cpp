#include "MenuAdmin.h"
#include "VueGestionEmployes.h"
#include "VueDepartementsPostes.h"
#include "VueSalaires.h"
#include "../01_Common/Navigation.h"
#include "../01_Common/ConsoleUI.h"
#include "../01_Common/InputValidator.h"
#include "../04_RessourcesHumaines/VueTraitementDemandesRH.h"
#include "../05_Conges_Absences/MenuConge.h"
#include "../05_Conges_Absences/MenuAbsences.h"
#include "../07_Rapports_Demandes/MenuDemandes.h"
#include "../07_Rapports_Demandes/MenuRapports.h"

void MenuAdmin::afficher(Navigation& navigation) {
    VueGestionEmployes vueEmployes;
    VueDepartementsPostes vueDeps;
    VueSalaires vueSalaires;
    MenuCongeUI menuConge;
    MenuAbsencesUI menuAbsences;
    MenuDemandesUI menuDemandes;
    MenuRapports menuRapports;
    VueTraitementDemandesRH vueDemandesRH;

    while (true) {
        ConsoleUI::effacerEcran();
        ConsoleUI::afficherTitre("ESPACE ADMINISTRATEUR");
        ConsoleUI::afficherInfo("Connecte : " + navigation.getIdentifiantActuel()
            + " | " + navigation.getLibelleRole());
        ConsoleUI::afficherInfo("1. Gestion des employes");
        ConsoleUI::afficherInfo("2. Departements et postes");
        ConsoleUI::afficherInfo("3. Salaires");
        ConsoleUI::afficherInfo("4. Conges");
        ConsoleUI::afficherInfo("5. Absences");
        ConsoleUI::afficherInfo("6. Demandes administratives");
        ConsoleUI::afficherInfo("7. Traitement RH (demandes / conges)");
        ConsoleUI::afficherInfo("8. Rapports et statistiques");
        ConsoleUI::afficherInfo("0. Deconnexion");

        const int choix = InputValidator::lireChoixMenu("Choix : ", 0, 8);
        if (choix == 0) {
            navigation.reinitialiserSession();
            break;
        }
        if (choix == 1) vueEmployes.afficher(navigation);
        else if (choix == 2) vueDeps.afficher(navigation);
        else if (choix == 3) vueSalaires.afficher(navigation);
        else if (choix == 4) menuConge.afficher(navigation, true);
        else if (choix == 5) menuAbsences.afficher(navigation, true);
        else if (choix == 6) menuDemandes.afficher(navigation, true);
        else if (choix == 7) vueDemandesRH.afficher(navigation);
        else if (choix == 8) menuRapports.afficher(navigation);
    }
}
