#ifndef DATEISO_H
#define DATEISO_H

#include <cctype>
#include <string>

// Utilitaires de date au format AAAA-MM-JJ (le format du type SQL DATE).
// Ils sont volontairement autonomes : ce module ne depend pas de la classe
// Date du module Employes.
namespace rewards {

inline bool anneeBissextile(int annee) {
    return (annee % 400 == 0) || (annee % 4 == 0 && annee % 100 != 0);
}

// Verifie le format ET la validite calendaire (ex: 2026-02-30 est refuse).
inline bool dateIsoValide(const std::string& date) {
    if (date.size() != 10) return false;
    if (date[4] != '-' || date[7] != '-') return false;

    for (std::size_t i = 0; i < date.size(); ++i) {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(static_cast<unsigned char>(date[i]))) return false;
    }

    const int annee = std::stoi(date.substr(0, 4));
    const int mois = std::stoi(date.substr(5, 2));
    const int jour = std::stoi(date.substr(8, 2));

    if (annee < 1900 || mois < 1 || mois > 12 || jour < 1) return false;

    static const int joursMois[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int jourMaximum = joursMois[mois - 1];
    if (mois == 2 && anneeBissextile(annee)) jourMaximum = 29;

    return jour <= jourMaximum;
}

// Comparaison lexicographique, correcte pour deux dates ISO valides.
inline int comparerDates(const std::string& gauche, const std::string& droite) {
    if (gauche < droite) return -1;
    if (droite < gauche) return 1;
    return 0;
}

} // namespace rewards

#endif
