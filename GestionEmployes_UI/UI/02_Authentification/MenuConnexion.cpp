#include "MenuConnexion.h"
#include "../01_Common/Navigation.h"
#include "../01_Common/ConsoleUI.h"
#include "../01_Common/InputValidator.h"
#include "../../include/BackendTypes.h"

// Trois espaces, comme le menu principal du back-end (source/main.cpp).
bool MenuConnexion::afficherEtConnecter(Navigation& navigation) {
    AuthService* auth = navigation.services().auth;

    while (true) {
        ConsoleUI::effacerEcran();
        ConsoleUI::afficherTitre("AUTHENTIFICATION - SYSTEME DE GESTION DES EMPLOYES");
        ConsoleUI::afficherInfo("1. Espace Administrateur");
        ConsoleUI::afficherInfo("2. Espace Responsable RH");
        ConsoleUI::afficherInfo("3. Espace Employe");
        ConsoleUI::afficherInfo("0. Quitter");

        const int choix = InputValidator::lireChoixMenu("Choix : ", 0, 3);
        if (choix == 0) return false;

        if (!auth) {
            ConsoleUI::afficherErreur("Service d'authentification non initialise.");
            ConsoleUI::pause();
            continue;
        }

        try {
            std::optional<SessionAuth> session;
            if (choix == 3) {
                ConsoleUI::afficherSousTitre("Connexion employe");
                session = auth->authentifierEmploye(InputValidator::lireChaine("Matricule ou CIN : "));
            } else {
                ConsoleUI::afficherSousTitre(choix == 1 ? "Connexion administrateur" : "Connexion RH");
                const std::string login = InputValidator::lireChaine("Login : ");
                const std::string motDePasse = InputValidator::lireChaine("Mot de passe : ");
                session = auth->authentifierCompte(login, motDePasse,
                                                   choix == 1 ? "administrateur" : "responsable_rh");
            }

            if (!session) {
                ConsoleUI::afficherErreur("Identifiants incorrects.");
                ConsoleUI::pause();
                continue;
            }

            RoleUtilisateur role = RoleUtilisateur::Employe;
            if (session->role == "administrateur") role = RoleUtilisateur::Administrateur;
            else if (session->role == "responsable_rh") role = RoleUtilisateur::RH;

            navigation.definirSession(role, session->login, session->employeId);
            ConsoleUI::afficherSucces("Connexion reussie. Role : " + navigation.getLibelleRole());
            ConsoleUI::pause();
            return true;
        } catch (const std::exception& e) {
            ConsoleUI::afficherErreur(std::string("Echec authentification : ") + e.what());
            ConsoleUI::pause();
        }
    }
}
