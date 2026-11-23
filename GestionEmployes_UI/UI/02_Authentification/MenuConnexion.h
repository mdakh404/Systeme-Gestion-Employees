// UI/02_Authentification/MenuConnexion.h
#ifndef MENU_CONNEXION_H
#define MENU_CONNEXION_H

class Navigation;

class MenuConnexion {
public:
    // Retourne true si connexion reussie (session definie), false si l'utilisateur quitte
    bool afficherEtConnecter(Navigation& navigation);
};

#endif // MENU_CONNEXION_H
