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

EspaceRH::EspaceRH(
    GestionnaireEmployes& gestion,
    MenuDemandes& demandes,
    MenuConge& conges,
    MenuAbsences& absences,
    PrimeService& primes,
    RetirementService& retraite,
    TrainingService& formations
)
    : gestion_(gestion),
      demandes_(demandes),
      conges_(conges),
      absences_(absences),
      primes_(primes),
      retraite_(retraite),
      formations_(formations) {}

void EspaceRH::executer() {
    menuConnecte();
}

void EspaceRH::menuServices() {
    while (true) {
        titre("Services RH & Remunerations");
        std::cout << "  1. Bonification des primes d'anciennete\n"
                  << "  2. Estimation de depart a la retraite\n"
                  << "  3. Catalogue et inscriptions aux formations\n"
                  << "  0. Retour au menu RH\n";

        std::string choix = lireLigne("Choix : ");
        if (choix == "0") return;

        try {
            if (choix == "1") {
                primes_.execute();
            } else if (choix == "2") {
                retraite_.execute();
            } else if (choix == "3") {
                formations_.execute();
            } else {
                std::cout << "Choix invalide.\n";
            }
        } catch (const std::exception& ex) {
            std::cout << "Erreur : " << ex.what() << "\n";
        }
        pause();
    }
}

void EspaceRH::menuConnecte() {
    while (true) {
        titre("Espace Responsable RH");
        std::cout << "  1. Consulter les demandes administratives\n"
                  << "  2. Traiter une demande administrative\n"
                  << "  3. Traitement des conges (validation & soldes)\n"
                  << "  4. Gestion des absences (enregistrement & suivi)\n"
                  << "  5. Services RH (primes, retraite, formations)\n"
                  << "  0. Retour au menu principal\n";

        std::string choix = lireLigne("Choix : ");

        try {
            if (choix == "0") return;

            if (choix == "1") {
                demandes_.consulterDemandes();
            } else if (choix == "2") {
                demandes_.traiterDemande();
            } else if (choix == "3") {
                conges_.lancerRH();
            } else if (choix == "4") {
                absences_.executer();
            } else if (choix == "5") {
                menuServices();
            } else {
                std::cout << "Choix invalide.\n";
            }
        } catch (const std::exception& ex) {
            std::cout << "Erreur : " << ex.what() << "\n";
        }

        pause();
    }
}
