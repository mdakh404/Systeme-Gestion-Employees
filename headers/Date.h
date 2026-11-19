#pragma once
#include <string>

// Date simple JJ/MM/AAAA
struct Date {
    int jour = 1, mois = 1, annee = 1970;

    Date() = default;
    Date(int j, int m, int a) : jour(j), mois(m), annee(a) {}

    bool estValide() const;
    std::string toString() const;                 // "JJ/MM/AAAA"
    static Date fromString(const std::string& s); // throws std::invalid_argument
    static Date aujourdhui();
    static int anneesEntre(const Date& debut, const Date& fin); // annees completes

    bool operator==(const Date& o) const;
    bool operator<(const Date& o) const;
    bool operator<=(const Date& o) const { return *this < o || *this == o; }
};
