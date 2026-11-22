#include "Date.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <stdexcept>

static bool bissextile(int a) { return (a % 4 == 0 && a % 100 != 0) || a % 400 == 0; }

static int joursDansMois(int m, int a) {
    static const int j[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return (m == 2 && bissextile(a)) ? 29 : j[m - 1];
}

bool Date::estValide() const {
    if (annee < 1900 || annee > 2100) return false;
    if (mois < 1 || mois > 12) return false;
    return jour >= 1 && jour <= joursDansMois(mois, annee);
}

std::string Date::toString() const {
    std::ostringstream os;
    os << std::setfill('0') << std::setw(2) << jour << '/'
       << std::setw(2) << mois << '/' << std::setw(4) << annee;
    return os.str();
}

Date Date::fromString(const std::string& s) {
    std::istringstream is(s);
    int j, m, a;
    char s1, s2;
    if (!(is >> j >> s1 >> m >> s2 >> a) || s1 != '/' || s2 != '/' || !is.eof())
        throw std::invalid_argument("format de date invalide (attendu JJ/MM/AAAA): " + s);
    Date d(j, m, a);
    if (!d.estValide()) throw std::invalid_argument("date invalide: " + s);
    return d;
}

Date Date::aujourdhui() {
    std::time_t t = std::time(nullptr);
    std::tm* lt = std::localtime(&t);
    return Date(lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900);
}

int Date::anneesEntre(const Date& debut, const Date& fin) {
    int n = fin.annee - debut.annee;
    if (fin.mois < debut.mois || (fin.mois == debut.mois && fin.jour < debut.jour)) --n;
    return n < 0 ? 0 : n;
}

bool Date::operator==(const Date& o) const {
    return annee == o.annee && mois == o.mois && jour == o.jour;
}

bool Date::operator<(const Date& o) const {
    if (annee != o.annee) return annee < o.annee;
    if (mois != o.mois) return mois < o.mois;
    return jour < o.jour;
}
