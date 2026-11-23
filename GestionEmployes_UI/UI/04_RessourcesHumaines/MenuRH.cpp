#include "MenuRH.h"
#include "VueTraitementDemandesRH.h"
#include "../01_Common/Navigation.h"
#include "../01_Common/ConsoleUI.h"
#include "../01_Common/InputValidator.h"
#include "../03_Administrateur/VueGestionEmployes.h"
#include "../03_Administrateur/VueDepartementsPostes.h"
#include "../03_Administrateur/VueSalaires.h"
#include "../05_Conges_Absences/MenuConge.h"
#include "../05_Conges_Absences/MenuAbsences.h"
#include "../07_Rapports_Demandes/MenuDemandes.h"
#include "../07_Rapports_Demandes/MenuRapports.h"

void MenuRH::afficher(Navigation& navigation) {
    VueGestionEmployes vueEmployes;
    VueDepartementsPostes vueDeps;
    VueSalaires vueSalaires;
    MenuCongeUI menuConge;
    MenuAbsencesUI menuAbsences;
    MenuDemandesUI menuDemandes;
    MenuRapports menuRapports;
    VueTraitementDemandesRH vueTraitement;

    while (true) {
        ConsoleUI::effacerEcran();
        ConsoleUI::afficherTitre("ESPACE RESSOURCES HUMAINES");
        ConsoleUI::afficherInfo("Connecte : " + navigation.getIdentifiantActuel());
        ConsoleUI::afficherInfo("1. Gestion des employes");
        ConsoleUI::afficherInfo("2. Departements");
        ConsoleUI::afficherInfo("3. Salaires");
        ConsoleUI::afficherInfo("4. Traiter les conges");
        ConsoleUI::afficherInfo("5. Absences");
        ConsoleUI::afficherInfo("6. Demandes administratives");
        ConsoleUI::afficherInfo("7. Traitement demandes RH");
        ConsoleUI::afficherInfo("8. Rapports");
        ConsoleUI::afficherInfo("0. Deconnexion");

        const int choix = InputValidator::lireChoixMenu("Choix : ", 0, 8);
        if (choix == 0) {
            navigation.reinitialiserSession();
            break;
        }
        if (choix == 1) vueEmployes.afficher(navigation);
        else if (choix == 2) vueDeps.afficher(navigation);
        else if (choix == 3) vueSalaires.afficher(navigation);
        else if (choix == 4) menuConge.afficher(navigation, true); // mode RH
        else if (choix == 5) menuAbsences.afficher(navigation, true);
        else if (choix == 6) menuDemandes.afficher(navigation, true);
        else if (choix == 7) vueTraitement.afficher(navigation);
        else if (choix == 8) menuRapports.afficher(navigation);
    }
}
