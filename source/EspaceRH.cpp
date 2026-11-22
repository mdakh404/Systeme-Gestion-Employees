#include "EspaceRH.h"
#include <ios>
#include <iostream>
#include <limits>
#include <string>

namespace {

std::string lireLigne(const std::string& prompt) {
    std::cout << prompt;
    std::string s;
    if (!std::getline(std::cin, s)) throw std::ios_base::failure("fin de saisie");
    return s;
}

void pause() {
    std::cout << "\nAppuyez sur Entree pour continuer...";
    if (!std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'))
        throw std::ios_base::failure("fin de saisie");
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

        std::string choix;
        try {
            choix = lireLigne("Choix : ");
        } catch (const std::ios_base::failure&) {
            std::cout << "\nFin de saisie. Retour au menu principal.\n";
            return;
        }

        try {
            if (choix == "0") return;

            if (choix == "1") {
                demandes_.consulterDemandes();
            } else if (choix == "2") {
                demandes_.traiterDemande();
            } else {
                std::cout << "Choix invalide.\n";
            }
        } catch (const std::ios_base::failure&) {
            std::cout << "\nFin de saisie. Retour au menu principal.\n";
            return;
        } catch (const std::exception& ex) {
            std::cout << "Erreur : " << ex.what() << "\n";
        }

        try {
            pause();
        } catch (const std::ios_base::failure&) {
            std::cout << "\nFin de saisie.\n";
            return;
        }
    }
}
