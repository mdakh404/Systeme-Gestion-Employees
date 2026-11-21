#include "GestionnaireFichiersAbsences.h"
#include "MenuAbsences.h"

#include <exception>
#include <iostream>
#include <algorithm>
#include <array>
#include <string>

namespace {

bool employeExisteDansLaDemonstration(int id) {
    constexpr std::array<int, 4> identifiants{1, 2, 15, 22};
    return std::find(identifiants.begin(), identifiants.end(), id) != identifiants.end();
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc > 2) {
            std::cerr << "Usage : gestion_absences [fichier-des-absences]\n";
            return 1;
        }
#ifdef GESTION_ABSENCES_FICHIER_DEMO
        const std::string cheminParDefaut = GESTION_ABSENCES_FICHIER_DEMO;
#else
        const std::string cheminParDefaut = "data/absences.txt";
#endif
        const GestionnaireFichiersAbsences fichier(argc == 2 ? argv[1] : cheminParDefaut);
        auto gestionnaire = fichier.chargerAbsences(employeExisteDansLaDemonstration);
        const MenuAbsences menu(gestionnaire, fichier);
        menu.executer();
        return 0;
    } catch (const std::exception& erreur) {
        std::cerr << "Démarrage impossible : " << erreur.what() << '\n';
        return 1;
    }
}
