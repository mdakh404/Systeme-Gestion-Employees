#include "VueSalaires.h"
#include "../01_Common/Navigation.h"
#include "../01_Common/ConsoleUI.h"
#include "../01_Common/InputValidator.h"
#include "../../include/BackendTypes.h"

void VueSalaires::afficher(Navigation& navigation) {
    auto* ge = navigation.services().employes;
    if (!ge) {
        ConsoleUI::afficherErreur("Service employes non initialise.");
        ConsoleUI::pause();
        return;
    }

    while (true) {
        ConsoleUI::effacerEcran();
        ConsoleUI::afficherTitre("SALAIRES");
        ConsoleUI::afficherInfo("1. Consulter salaire d'un employe");
        ConsoleUI::afficherInfo("2. Lister salaires (tous)");
        ConsoleUI::afficherInfo("3. Historique employe");
        ConsoleUI::afficherInfo("0. Retour");

        const int choix = InputValidator::lireChoixMenu("Choix : ", 0, 3);
        if (choix == 0) break;

        try {
            if (choix == 1) {
                int id = InputValidator::lireEntier("ID employe : ");
                Employe e = ge->trouver(id);
                ConsoleUI::afficherInfo(e.getMatricule() + " | " + e.getNom() + " " + e.getPrenom());
                ConsoleUI::afficherInfo("Salaire base : " + std::to_string(e.getSalaire().getSalaireBase()));
                ConsoleUI::afficherInfo("Anciennete : " + std::to_string(e.anciennete()) + " an(s)");
                // Calcul delegue au back-end (PrimeService) : Salaire::getPrimeAnciennete() n'est pas alimente.
                if (auto* primes = navigation.services().primes) {
                    ConsoleUI::afficherInfo("Taux prime : "
                        + std::to_string(primes->calculerTaux(e.anciennete()) * 100.0) + " %");
                    ConsoleUI::afficherInfo("Prime anciennete : " + std::to_string(primes->calculerMontant(e)));
                }
                ConsoleUI::pause();
            } else if (choix == 2) {
                auto liste = ge->lister();
                std::vector<std::string> h = {"Matricule", "Nom", "Salaire base"};
                std::vector<std::vector<std::string>> lignes;
                for (const auto& e : liste)
                    lignes.push_back({e.getMatricule(), e.getNom(),
                                      std::to_string(static_cast<int>(e.getSalaire().getSalaireBase()))});
                ConsoleUI::afficherTableau(h, lignes);
                ConsoleUI::pause();
            } else if (choix == 3) {
                int id = InputValidator::lireEntier("ID employe : ");
                auto hist = ge->historique(id);
                ConsoleUI::afficherSousTitre("Historique");
                for (const auto& ligne : hist) ConsoleUI::afficherInfo("- " + ligne);
                if (hist.empty()) ConsoleUI::afficherInfo("(vide)");
                ConsoleUI::pause();
            }
        } catch (const std::exception& e) {
            ConsoleUI::afficherErreur(e.what());
            ConsoleUI::pause();
        }
    }
}
