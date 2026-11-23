#include "Navigation.h"
#include "ConsoleUI.h"
#include "InputValidator.h"
#include "../02_Authentification/MenuConnexion.h"
#include "../03_Administrateur/MenuAdmin.h"
#include "../04_RessourcesHumaines/MenuRH.h"
#include "../06_Employe/MenuEmploye.h"

Navigation::Navigation(const BackendServices& services) : services_(services) {}

void Navigation::definirSession(RoleUtilisateur role, const std::string& identifiant, int employeId) {
    roleActuel_ = role;
    identifiantActuel_ = identifiant;
    employeId_ = employeId;
}

void Navigation::reinitialiserSession() {
    roleActuel_ = RoleUtilisateur::Invite;
    identifiantActuel_.clear();
    employeId_ = 0;
}

RoleUtilisateur Navigation::getRoleActuel() const { return roleActuel_; }
std::string Navigation::getIdentifiantActuel() const { return identifiantActuel_; }
int Navigation::getEmployeId() const { return employeId_; }

std::string Navigation::getLibelleRole() const {
    switch (roleActuel_) {
        case RoleUtilisateur::Employe: return "Employe";
        case RoleUtilisateur::RH: return "Ressources Humaines";
        case RoleUtilisateur::Administrateur: return "Administrateur";
        default: return "Non connecte";
    }
}

BackendServices& Navigation::services() { return services_; }
const BackendServices& Navigation::services() const { return services_; }

void Navigation::orienterSelonRole() {
    if (roleActuel_ == RoleUtilisateur::Administrateur) {
        MenuAdmin menu;
        menu.afficher(*this);
        return;
    }
    if (roleActuel_ == RoleUtilisateur::RH) {
        MenuRH menu;
        menu.afficher(*this);
        return;
    }
    if (roleActuel_ == RoleUtilisateur::Employe) {
        MenuEmploye menu;
        menu.afficher(*this);
        return;
    }
}

void Navigation::executerBouclePrincipale() {
    MenuConnexion menuConnexion;
    while (true) {
        const bool connecte = menuConnexion.afficherEtConnecter(*this);
        if (!connecte) {
            if (InputValidator::lireConfirmation("Voulez-vous vraiment quitter l'application")) {
                ConsoleUI::afficherInfo("Au revoir.");
                break;
            }
            continue;
        }
        orienterSelonRole();
    }
}
