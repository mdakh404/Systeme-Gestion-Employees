#include "ConsoleUI.h"
#include <iostream>
#include <iomanip>
#include <limits>

#ifdef _WIN32
#include <cstdlib>
#else
#include <cstdlib>
#endif

void ConsoleUI::effacerEcran() {
#ifdef _WIN32
    if (std::system("cls") != 0) {}
#else
    if (std::system("clear") != 0) {}
#endif
}

void ConsoleUI::afficherTitre(const std::string& titre) {
    afficherSeparateur('=');
    std::cout << "  " << titre << "\n";
    afficherSeparateur('=');
}

void ConsoleUI::afficherSousTitre(const std::string& sousTitre) {
    std::cout << "\n--- " << sousTitre << " ---\n";
}

void ConsoleUI::afficherSeparateur(char motif, int largeur) {
    std::cout << std::string(static_cast<std::size_t>(largeur), motif) << "\n";
}

void ConsoleUI::afficherInfo(const std::string& message) {
    std::cout << message << "\n";
}

void ConsoleUI::afficherSucces(const std::string& message) {
    std::cout << "[OK] " << message << "\n";
}

void ConsoleUI::afficherErreur(const std::string& message) {
    std::cout << "[ERREUR] " << message << "\n";
}

void ConsoleUI::afficherTableau(
    const std::vector<std::string>& enTetes,
    const std::vector<std::vector<std::string>>& lignes
) {
    if (enTetes.empty()) return;

    std::vector<std::size_t> largeurs(enTetes.size(), 0);
    for (std::size_t i = 0; i < enTetes.size(); ++i) {
        largeurs[i] = enTetes[i].size();
    }
    for (const auto& ligne : lignes) {
        for (std::size_t i = 0; i < ligne.size() && i < largeurs.size(); ++i) {
            if (ligne[i].size() > largeurs[i]) largeurs[i] = ligne[i].size();
        }
    }

    for (std::size_t i = 0; i < enTetes.size(); ++i) {
        std::cout << std::left << std::setw(static_cast<int>(largeurs[i] + 2)) << enTetes[i];
    }
    std::cout << "\n";
    for (std::size_t i = 0; i < enTetes.size(); ++i) {
        std::cout << std::string(largeurs[i], '-') << "  ";
    }
    std::cout << "\n";

    if (lignes.empty()) {
        std::cout << "(aucune donnee)\n";
        return;
    }
    for (const auto& ligne : lignes) {
        for (std::size_t i = 0; i < enTetes.size(); ++i) {
            const std::string cell = (i < ligne.size()) ? ligne[i] : "";
            std::cout << std::left << std::setw(static_cast<int>(largeurs[i] + 2)) << cell;
        }
        std::cout << "\n";
    }
}

void ConsoleUI::pause(const std::string& message) {
    std::cout << "\n" << message;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
