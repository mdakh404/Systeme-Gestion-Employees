#ifndef MENU_DEMANDES_UI_H
#define MENU_DEMANDES_UI_H

class Navigation;

class MenuDemandesUI {
public:
    // modeRH = true : liste + traitement ; false : depot + consultation personnelle
    void afficher(Navigation& navigation, bool modeRH = false);
};

#endif
