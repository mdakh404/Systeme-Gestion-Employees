#include "EspaceRH.h"
#include <iostream>
#include <limits>
#include <string>

namespace {

std::string lireLigne(const std::string& prompt) {
    std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

void pause() {
    std::cout << "\nAppuyez sur Entree pour continuer...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void titre(const std::string& t) {
    std::cout << "\n========== " << t << " ==========\n";
}

} // namespace

EspaceRH::EspaceRH(GestionnaireEmployes& gestion, MenuDemandes& demandes)
    : gestion_(gestion), demandes_(demandes) {}

void EspaceRH::executer() {
    menuConnecte();
}

void EspaceRH::menuConnecte() {
    while (true) {
        titre("Espace Responsable RH");
        std::cout << "  1. Consulter les demandes administratives\n"
                  << "  2. Traiter une demande administrative\n"
                  << "  0. Retour\n";

        std::string choix = lireLigne("Choix : ");

        try {
            if (choix == "0") return;

            if (choix == "1") {
                demandes_.consulterDemandes();
            } else if (choix == "2") {
                demandes_.traiterDemande();
            } else {
                std::cout << "Choix invalide.\n";
            }
        } catch (const std::exception& ex) {
            std::cout << "Erreur : " << ex.what() << "\n";
        }

        pause();
    }
}
