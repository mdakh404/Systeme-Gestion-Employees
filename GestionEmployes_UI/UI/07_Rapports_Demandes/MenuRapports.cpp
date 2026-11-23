#include "MenuRapports.h"
#include "../01_Common/Navigation.h"
#include "../01_Common/ConsoleUI.h"
#include "../01_Common/InputValidator.h"
#include "../../include/BackendTypes.h"

void MenuRapports::afficher(Navigation& navigation) {
    auto* ge = navigation.services().employes;
    auto* gd = navigation.services().departements;
    auto* gc = navigation.services().conges;
    auto* ga = navigation.services().absences;

    while (true) {
        ConsoleUI::effacerEcran();
        ConsoleUI::afficherTitre("RAPPORTS ET STATISTIQUES");
        ConsoleUI::afficherInfo("1. Effectifs globaux");
        ConsoleUI::afficherInfo("2. Effectifs par departement");
        ConsoleUI::afficherInfo("3. Resume conges / absences");
        ConsoleUI::afficherInfo("0. Retour");

        const int choix = InputValidator::lireChoixMenu("Choix : ", 0, 3);
        if (choix == 0) break;

        try {
            if (choix == 1) {
                std::size_t n = ge ? ge->nombre() : 0;
                ConsoleUI::afficherInfo("Nombre d'employes : " + std::to_string(n));
                if (gd) ConsoleUI::afficherInfo("Nombre de departements : " + std::to_string(gd->nombre()));
                ConsoleUI::pause();
            } else if (choix == 2) {
                if (!gd || !ge) {
                    ConsoleUI::afficherErreur("Services manquants.");
                    ConsoleUI::pause();
                    continue;
                }
                std::vector<std::string> h = {"ID Dep", "Nom", "Effectif"};
                std::vector<std::vector<std::string>> lignes;
                for (const auto& d : gd->lister()) {
                    lignes.push_back({
                        std::to_string(d.getId()),
                        d.getNom(),
                        std::to_string(ge->effectifDepartement(d.getId()))
                    });
                }
                ConsoleUI::afficherTableau(h, lignes);
                ConsoleUI::pause();
            } else if (choix == 3) {
                if (gc) {
                    const StatistiquesConges st = gc->calculerStatistiques();
                    ConsoleUI::afficherSousTitre("Conges");
                    ConsoleUI::afficherInfo("Demandes : " + std::to_string(st.total)
                        + " (en attente " + std::to_string(st.enAttente)
                        + ", acceptees " + std::to_string(st.acceptees)
                        + ", refusees " + std::to_string(st.refusees) + ")");
                    ConsoleUI::afficherInfo("Jours pris : " + std::to_string(st.joursPris));
                    ConsoleUI::afficherInfo("Taux d'acceptation : "
                        + std::to_string(static_cast<int>(st.tauxAcceptation + 0.5)) + " %");
                }
                if (ga) {
                    const auto actives = ga->consulterAbsencesActives();
                    std::size_t justifiees = 0;
                    for (const auto& a : actives) if (a.estJustifiee()) ++justifiees;
                    ConsoleUI::afficherSousTitre("Absences");
                    ConsoleUI::afficherInfo("Absences actives : " + std::to_string(actives.size())
                        + " (justifiees " + std::to_string(justifiees) + ")");
                }
                ConsoleUI::pause();
            }
        } catch (const std::exception& e) {
            ConsoleUI::afficherErreur(e.what());
            ConsoleUI::pause();
        }
    }
}
