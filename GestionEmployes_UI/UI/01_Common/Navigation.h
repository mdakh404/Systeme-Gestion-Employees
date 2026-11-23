// UI/01_Common/Navigation.h
#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <string>
#include "../../include/BackendTypes.h"

enum class RoleUtilisateur {
    Invite,
    Employe,
    RH,
    Administrateur
};

class Navigation {
private:
    RoleUtilisateur roleActuel_ = RoleUtilisateur::Invite;
    std::string identifiantActuel_;  // login ou matricule affiche
    int employeId_ = 0;              // id backend de l'utilisateur connecte
    BackendServices services_;

    void orienterSelonRole();

public:
    explicit Navigation(const BackendServices& services);

    void definirSession(RoleUtilisateur role, const std::string& identifiant, int employeId);
    void reinitialiserSession();

    RoleUtilisateur getRoleActuel() const;
    std::string getIdentifiantActuel() const;
    int getEmployeId() const;
    std::string getLibelleRole() const;
    BackendServices& services();
    const BackendServices& services() const;

    void executerBouclePrincipale();
};

#endif // NAVIGATION_H
