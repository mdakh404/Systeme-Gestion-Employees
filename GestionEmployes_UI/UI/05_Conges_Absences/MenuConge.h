#ifndef MENU_CONGE_UI_H
#define MENU_CONGE_UI_H

class Navigation;

class MenuCongeUI {
public:
    // modeRH = true : traitement + liste complete ; false : depot + historique personnel
    void afficher(Navigation& navigation, bool modeRH = false);
};

#endif
