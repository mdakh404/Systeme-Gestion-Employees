#pragma once
#include <exception>
#include <functional>
#include <string>
#include <vector>

#include "Absence.h"
#include "Conge.h"
#include "Database.h"
#include "Date.h"
#include "DemandeAdministrative.h"

// Persistance MySQL des conges, demandes et absences + utilitaires partages.
// Ces fonctions ne sont appelees que lorsque Database::instance().estConnectee() est vrai.
namespace db {

// ---- utilitaires SQL ----
std::string q(const std::string& valeur);          // 'valeur' echappee (injection-safe)
std::string n(int valeur);                         // entier -> texte
std::string dateSql(const Date& d);                // Date -> 'AAAA-MM-JJ' (sans quotes)
std::string dateFrVersSql(const std::string& fr);  // "JJ/MM/AAAA" -> "AAAA-MM-JJ"

// ---- correspondance vocabulaire C++ <-> vocabulaire SQL ----
std::string statutEmployeVersSql(const std::string& statut);
std::string statutEmployeDepuisSql(const std::string& statut);

// Execute des operations une par une dans une meme transaction : une erreur SQL n'annule que
// l'operation en cause (SAVEPOINT) ; les erreurs sont reportees a la fin par lancerSiErreurs().
class Collecteur {
    std::vector<std::string> erreurs_;

public:
    // Renvoie true si l'operation a reussi.
    bool essayer(const std::string& etiquette, const std::function<void()>& action);
    bool aucuneErreur() const { return erreurs_.empty(); }
    void lancerSiErreurs(const std::string& contexte) const;
};

// ---- conges ----
std::vector<Conge> chargerConges();
void sauvegarderConges(const std::vector<Conge>& conges);
void ajouterConge(const Conge& conge);
bool mettreAJourStatutConge(int idConge, const std::string& statutCpp);

// ---- demandes administratives ----
std::vector<DemandeAdministrative> lireDemandes();
void ecrireDemandes(const std::vector<DemandeAdministrative>& demandes);

// ---- absences ----
struct AbsencesLues {
    std::vector<Absence> liste;
    int dernierId = 0;
};
AbsencesLues chargerAbsences();
// Tout ou rien : en cas d'erreur, rien n'est enregistre et l'exception est relancee.
void sauvegarderAbsences(const std::vector<Absence>& absences, int idSupprime = 0);

} // namespace db
