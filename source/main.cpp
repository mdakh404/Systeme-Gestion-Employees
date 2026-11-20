#include "EspaceEmploye.h"
#include "GestionnaireEmployes.h"
#include "admin/EspaceAdmin.h"

#include <exception>
#include <iostream>
#include <string>

int main() {
    GestionnaireEmployes gestion;

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
                std::cout << "Espace Responsable RH : a integrer.\n";
            } else if (choix == "3") {
                EspaceEmploye espace(gestion);
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
