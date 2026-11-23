#include "InputValidator.h"
#include <iostream>
#include <limits>
#include <sstream>
#include <cctype>

static void viderBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int InputValidator::lireEntier(const std::string& invite) {
    while (true) {
        std::cout << invite;
        int v = 0;
        if (std::cin >> v) {
            viderBuffer();
            return v;
        }
        std::cout << "Saisie invalide. Entrez un nombre entier.\n";
        viderBuffer();
    }
}

int InputValidator::lireChoixMenu(const std::string& invite, int minVal, int maxVal) {
    while (true) {
        int v = lireEntier(invite);
        if (v >= minVal && v <= maxVal) return v;
        std::cout << "Choix hors intervalle [" << minVal << " - " << maxVal << "].\n";
    }
}

std::string InputValidator::lireChaine(const std::string& invite, bool autoriserVide) {
    while (true) {
        std::cout << invite;
        std::string s;
        std::getline(std::cin, s);
        // trim simple
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
        if (!s.empty() || autoriserVide) return s;
        std::cout << "La saisie ne peut pas etre vide.\n";
    }
}

std::string InputValidator::lireDate(const std::string& invite) {
    while (true) {
        std::string s = lireChaine(invite);
        // Verification forme JJ/MM/AAAA
        if (s.size() == 10 && s[2] == '/' && s[5] == '/') {
            bool digits = true;
            for (int i : {0,1,3,4,6,7,8,9}) {
                if (!std::isdigit(static_cast<unsigned char>(s[i]))) { digits = false; break; }
            }
            if (digits) return s;
        }
        std::cout << "Format attendu : JJ/MM/AAAA\n";
    }
}

double InputValidator::lireDouble(const std::string& invite) {
    while (true) {
        std::cout << invite;
        double v = 0.0;
        if (std::cin >> v) {
            viderBuffer();
            return v;
        }
        std::cout << "Saisie invalide. Entrez un nombre.\n";
        viderBuffer();
    }
}

bool InputValidator::lireConfirmation(const std::string& invite) {
    while (true) {
        std::string s = lireChaine(invite + " (O/N) : ");
        if (s == "O" || s == "o" || s == "oui" || s == "Oui") return true;
        if (s == "N" || s == "n" || s == "non" || s == "Non") return false;
        std::cout << "Repondez par O ou N.\n";
    }
}

std::string InputValidator::lireChoixListe(const std::string& titre,
                                           const std::vector<std::string>& options) {
    std::cout << titre << "\n";
    for (std::size_t i = 0; i < options.size(); ++i)
        std::cout << "  " << (i + 1) << ". " << options[i] << "\n";
    const int choix = lireChoixMenu("Choix : ", 1, static_cast<int>(options.size()));
    return options[static_cast<std::size_t>(choix - 1)];
}
