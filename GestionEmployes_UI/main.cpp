// main.cpp — Point d'entree de l'UI console, branchee sur le back-end reel.
// L'ordre d'initialisation reprend celui de source/main.cpp du back-end.
#include "UI/01_Common/Navigation.h"
#include "UI/01_Common/ConsoleUI.h"
#include "include/BackendTypes.h"
#include "Database.h"

#include <exception>
#include <iostream>
#include <string>

namespace {
template <typename Action>
void charger(const std::string& module, const Action& action) {
    try {
        action();
    } catch (const std::exception& ex) {
        std::cerr << "[Chargement] " << module << " : " << ex.what() << "\n";
    }
}
} // namespace

int main() {
    try {
        // 0. Connexion MySQL si configuree, sinon mode fichiers data/*.txt
        if (Database::instance().connecter()) {
            std::cout << "[BD] Connecte a MySQL : " << Database::instance().description() << "\n";
        } else {
            std::cout << "[BD] Base de donnees indisponible -> mode fichiers data/*.txt\n";
        }

        // 1. Services du back-end
        GestionnaireEmployes gestionnaireEmployes;
        charger("Employes", [&] { gestionnaireEmployes.chargerDepuisFichier("data/employes.txt"); });

        GestionnaireDepartements gestionnaireDepartements;
        charger("Departements", [&] { gestionnaireDepartements.chargerDepuisFichier(); });

        GestionnaireConges gestionnaireConges;
        charger("Conges", [&] { gestionnaireConges.chargerDepuisFichier(); });

        GestionnaireFichiersAbsences fichiersAbsences("data/absences.txt");
        auto employeExiste = [&gestionnaireEmployes](int id) {
            return gestionnaireEmployes.rechercher(id) != nullptr;
        };
        GestionnaireAbsences gestionnaireAbsences(employeExiste);
        charger("Absences", [&] { gestionnaireAbsences = fichiersAbsences.chargerAbsences(employeExiste); });

        GestionnaireDemandes gestionnaireDemandes;
        GestionnaireFichiersDemandes fichiersDemandes;
        charger("Demandes", [&] { gestionnaireDemandes.charger(fichiersDemandes.lireDemandes()); });

        PrimeService primeService(1, gestionnaireEmployes);

        AuthService auth(gestionnaireEmployes);
        if (auth.initialiserComptesParDefaut())
            std::cout << "[Auth] data/comptes_ui.txt cree (admin/admin, rh/rh) : changez ces mots de passe.\n";

        // 2. Injection dans l'UI
        BackendServices services;
        services.departements = &gestionnaireDepartements;
        services.employes = &gestionnaireEmployes;
        services.conges = &gestionnaireConges;
        services.absences = &gestionnaireAbsences;
        services.demandes = &gestionnaireDemandes;
        services.primes = &primeService;
        services.fichiersAbsences = &fichiersAbsences;
        services.fichiersDemandes = &fichiersDemandes;
        services.auth = &auth;

        Navigation navigation(services);
        navigation.executerBouclePrincipale();

        // 3. Sauvegarde finale (chaque operation sauvegarde deja ; filet de securite)
        try {
            gestionnaireEmployes.sauvegarderDansFichier("data/employes.txt");
            gestionnaireDepartements.sauvegarderDansFichier();
            gestionnaireConges.sauvegarderDansFichier();
            fichiersAbsences.sauvegarderAbsences(gestionnaireAbsences);
            fichiersDemandes.ecrireDemandes(gestionnaireDemandes.lister());
        } catch (const std::exception& ex) {
            ConsoleUI::afficherErreur(std::string("Sauvegarde finale : ") + ex.what());
        }

        Database::instance().deconnecter();
        return 0;
    } catch (const std::exception& ex) {
        ConsoleUI::afficherErreur(std::string("Erreur inattendue : ") + ex.what());
        ConsoleUI::pause();
        return 1;
    } catch (...) {
        ConsoleUI::afficherErreur("Erreur inconnue.");
        ConsoleUI::pause();
        return 1;
    }
}
