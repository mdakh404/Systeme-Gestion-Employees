// UI/01_Common/InputValidator.h
#ifndef INPUT_VALIDATOR_H
#define INPUT_VALIDATOR_H

#include <string>
#include <vector>

class InputValidator {
public:
    static int lireEntier(const std::string& invite);
    static int lireChoixMenu(const std::string& invite, int minVal, int maxVal);
    static std::string lireChaine(const std::string& invite, bool autoriserVide = false);
    static std::string lireDate(const std::string& invite); // JJ/MM/AAAA
    static double lireDouble(const std::string& invite);
    static bool lireConfirmation(const std::string& invite);
    // Affiche les options numerotees (1..n) et renvoie la valeur choisie
    static std::string lireChoixListe(const std::string& titre, const std::vector<std::string>& options);
};

#endif // INPUT_VALIDATOR_H
