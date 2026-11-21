#include "EspaceEmploye.h"
#include "EspaceRH.h"
#include "GestionnaireEmployes.h"
#include "GestionnaireDemandes.h"
#include "GestionnaireFichiersDemandes.h"
#include "MenuDemandes.h"
#include "admin/EspaceAdmin.h"

#include <exception>
#include <iostream>
#include <string>

int main() {
    GestionnaireEmployes gestion;
    GestionnaireDemandes gestionDemandes;
    GestionnaireFichiersDemandes fichiersDemandes;
    MenuDemandes menuDemandes(gestionDemandes, fichiersDemandes);

    // Chargement des demandes administratives depuis le fichier
    try {
        gestionDemandes.charger(fichiersDemandes.lireDemandes());
    } catch (const std::exception& ex) {
        std::cerr << "Attention : " << ex.what() << "\n";
    }

    while (true) {
        std::cout << "\n========== Gestion des employes ==========\n"
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
                EspaceRH rh(gestion, menuDemandes);
                rh.executer();
            } else if (choix == "3") {
                EspaceEmploye espace(gestion, menuDemandes);
                espace.executer();
            } else {
                std::cout << "Choix invalide.\n";
            }
        } catch (const std::exception& ex) {
            std::cerr << "Erreur : " << ex.what() << "\n";
        }
    }

    std::cout << "\nFin du programme.\n";
    return 0;
}
