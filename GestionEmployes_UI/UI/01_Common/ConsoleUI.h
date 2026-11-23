// UI/01_Common/ConsoleUI.h
#ifndef CONSOLE_UI_H
#define CONSOLE_UI_H

#include <string>
#include <vector>

class ConsoleUI {
public:
    static void effacerEcran();
    static void afficherTitre(const std::string& titre);
    static void afficherSousTitre(const std::string& sousTitre);
    static void afficherSeparateur(char motif = '-', int largeur = 50);
    static void afficherInfo(const std::string& message);
    static void afficherSucces(const std::string& message);
    static void afficherErreur(const std::string& message);
    static void afficherTableau(
        const std::vector<std::string>& enTetes,
        const std::vector<std::vector<std::string>>& lignes
    );
    static void pause(const std::string& message = "Appuyez sur Entree pour continuer...");
};

#endif // CONSOLE_UI_H
