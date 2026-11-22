#pragma once
#include <string>

// Fonctions de validation des champs (aucune dependance aux classes metier)
namespace Validation {
    bool nomValide(const std::string& s);        // lettres, espace, '-', ''' (2..50)
    bool emailValide(const std::string& s);
    bool telephoneValide(const std::string& s);  // [+]chiffres (8..15)
    bool cinValide(const std::string& s);        // alphanumerique (5..12)
    bool nomDepartementValide(const std::string& s); // lettres, chiffres, espace, - ' & (2..50)
    bool sansSeparateur(const std::string& s);   // pas de ';' '|' ni retour ligne (format CSV)
}
