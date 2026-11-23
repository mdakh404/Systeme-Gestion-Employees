#include "DbPersistence.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace db {

// =====================================================================
// Utilitaires
// =====================================================================

std::string q(const std::string& valeur) {
    return "'" + Database::instance().echapper(valeur) + "'";
}

std::string n(int valeur) {
    return std::to_string(valeur);
}

std::string dateSql(const Date& d) {
    std::ostringstream os;
    os << std::setfill('0') << std::setw(4) << d.annee << '-'
       << std::setw(2) << d.mois << '-' << std::setw(2) << d.jour;
    return os.str();
}

std::string dateFrVersSql(const std::string& fr) {
    if (fr.size() != 10 || fr[2] != '/' || fr[5] != '/') {
        throw std::invalid_argument("date attendue au format JJ/MM/AAAA : " + fr);
    }
    return fr.substr(6, 4) + "-" + fr.substr(3, 2) + "-" + fr.substr(0, 2);
}

namespace {

std::string enMinuscules(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

std::string versIdentifiantSql(const std::string& texte) {   // "Sans solde" -> "sans_solde"
    std::string s = enMinuscules(texte);
    std::replace(s.begin(), s.end(), ' ', '_');
    return s;
}

std::string typeCongeVersSql(const std::string& type) { return versIdentifiantSql(type); }

std::string typeCongeDepuisSql(const std::string& type) {
    if (type == "annuel") return "Annuel";
    if (type == "maladie") return "Maladie";
    if (type == "sans_solde") return "Sans solde";
    return "Exceptionnel";   // exceptionnel, maternite, paternite
}

std::string statutCongeVersSql(const std::string& statut) {
    if (statut == "Acceptee" || statut == "Accept\xC3\xA9" "e") return "approuve";
    if (statut == "Refusee" || statut == "Refus\xC3\xA9" "e") return "refuse";
    return "en_attente";
}

std::string statutCongeDepuisSql(const std::string& statut) {
    if (statut == "approuve") return "Acceptee";
    if (statut == "refuse" || statut == "annule") return "Refusee";
    return "En attente";
}

std::string statutDemandeVersSql(const std::string& statut) {
    if (statut == DemandeAdministrative::STATUT_ACCEPTEE) return "traitee";
    if (statut == DemandeAdministrative::STATUT_REFUSEE) return "rejetee";
    return "en_attente";
}

std::string statutDemandeDepuisSql(const std::string& statut) {
    if (statut == "traitee") return DemandeAdministrative::STATUT_ACCEPTEE;
    if (statut == "rejetee") return DemandeAdministrative::STATUT_REFUSEE;
    return DemandeAdministrative::STATUT_EN_ATTENTE;   // en_attente, en_cours
}

int versEntier(const std::optional<std::string>& champ, int parDefaut = 0) {
    return champ ? std::stoi(*champ) : parDefaut;
}

std::string versTexte(const std::optional<std::string>& champ) {
    return champ ? *champ : std::string();
}

} // namespace

std::string statutEmployeVersSql(const std::string& statut) {
    return versIdentifiantSql(statut);   // "Actif"->actif, "En conge"->en_conge, "Inactif"->inactif
}

std::string statutEmployeDepuisSql(const std::string& statut) {
    if (statut == "actif") return "Actif";
    if (statut == "en_conge") return "En conge";
    if (statut == "suspendu") return "Suspendu";
    return "Inactif";   // inactif, demissionnaire, licencie, retraite
}

// =====================================================================
// Collecteur
// =====================================================================

bool Collecteur::essayer(const std::string& etiquette, const std::function<void()>& action) {
    Database& base = Database::instance();
    base.executer("SAVEPOINT ems_sp");
    try {
        action();
        base.executer("RELEASE SAVEPOINT ems_sp");
        return true;
    } catch (const std::exception& ex) {
        try {
            base.executer("ROLLBACK TO SAVEPOINT ems_sp");
            base.executer("RELEASE SAVEPOINT ems_sp");
        } catch (...) {}
        erreurs_.push_back(etiquette + " : " + ex.what());
        return false;
    }
}

void Collecteur::lancerSiErreurs(const std::string& contexte) const {
    if (erreurs_.empty()) return;
    std::string message = contexte + " : " + std::to_string(erreurs_.size())
                          + " enregistrement(s) refuse(s) par la base de donnees";
    for (const auto& e : erreurs_) message += "\n  - " + e;
    throw std::runtime_error(message);
}

// =====================================================================
// Conges
// =====================================================================

namespace {

void upsertConge(const Conge& c) {
    Database::instance().executer(
        "INSERT INTO conge (id, employe_id, date_debut, date_fin, type_conge, statut) VALUES ("
        + n(c.Getid()) + ", " + n(c.GetidEmploye()) + ", " + q(c.GetdateDebut()) + ", "
        + q(c.GetdateFin()) + ", " + q(typeCongeVersSql(c.Gettype())) + ", "
        + q(statutCongeVersSql(c.Getstatut())) + ") "
        "ON DUPLICATE KEY UPDATE date_debut = VALUES(date_debut), date_fin = VALUES(date_fin), "
        "type_conge = VALUES(type_conge), statut = VALUES(statut)");
}

} // namespace

std::vector<Conge> chargerConges() {
    auto lignes = Database::instance().requete(
        "SELECT id, employe_id, DATE_FORMAT(date_debut, '%Y-%m-%d'), "
        "DATE_FORMAT(date_fin, '%Y-%m-%d'), type_conge, statut FROM conge ORDER BY id");

    std::vector<Conge> res;
    for (const auto& l : lignes) {
        res.emplace_back(versEntier(l[0]), versEntier(l[1]), versTexte(l[2]), versTexte(l[3]),
                         typeCongeDepuisSql(versTexte(l[4])), statutCongeDepuisSql(versTexte(l[5])));
    }
    return res;
}

void sauvegarderConges(const std::vector<Conge>& conges) {
    Database& base = Database::instance();
    Transaction tx(base);
    Collecteur col;
    for (const auto& c : conges) {
        col.essayer("conge " + n(c.Getid()) + " (employe " + n(c.GetidEmploye()) + ")",
                    [&] { upsertConge(c); });
    }
    tx.commit();
    col.lancerSiErreurs("Sauvegarde des conges");
}

void ajouterConge(const Conge& conge) {
    upsertConge(conge);
}

bool mettreAJourStatutConge(int idConge, const std::string& statutCpp) {
    Database& base = Database::instance();
    if (base.requete("SELECT 1 FROM conge WHERE id = " + n(idConge)).empty()) return false;
    base.executer("UPDATE conge SET statut = " + q(statutCongeVersSql(statutCpp))
                  + " WHERE id = " + n(idConge));
    return true;
}

// =====================================================================
// Demandes administratives
// =====================================================================

std::vector<DemandeAdministrative> lireDemandes() {
    auto lignes = Database::instance().requete(
        "SELECT id, employe_id, type_demande, DATE_FORMAT(date_depot, '%d/%m/%Y'), statut, "
        "description FROM demande_administrative ORDER BY id");

    std::vector<DemandeAdministrative> res;
    for (const auto& l : lignes) {
        try {
            res.emplace_back(versEntier(l[0]), versEntier(l[1]), versTexte(l[2]), versTexte(l[5]),
                             Date::fromString(versTexte(l[3])),
                             statutDemandeDepuisSql(versTexte(l[4])));
        } catch (const std::exception& ex) {
            std::cerr << "Demande " << versTexte(l[0]) << " ignoree (donnees invalides) : "
                      << ex.what() << "\n";
        }
    }
    return res;
}

void ecrireDemandes(const std::vector<DemandeAdministrative>& demandes) {
    Database& base = Database::instance();
    Transaction tx(base);
    Collecteur col;
    for (const auto& d : demandes) {
        col.essayer("demande " + n(d.getId()) + " (employe " + n(d.getIdEmploye()) + ")", [&] {
            base.executer(
                "INSERT INTO demande_administrative "
                "(id, employe_id, type_demande, date_depot, statut, description) VALUES ("
                + n(d.getId()) + ", " + n(d.getIdEmploye()) + ", " + q(d.getType()) + ", "
                + q(dateSql(d.getDateDepot())) + ", " + q(statutDemandeVersSql(d.getStatut()))
                + ", " + q(d.getDescription()) + ") "
                "ON DUPLICATE KEY UPDATE type_demande = VALUES(type_demande), "
                "date_depot = VALUES(date_depot), statut = VALUES(statut), "
                "description = VALUES(description)");
        });
    }
    tx.commit();
    col.lancerSiErreurs("Sauvegarde des demandes");
}

// =====================================================================
// Absences
// =====================================================================

AbsencesLues chargerAbsences() {
    Database& base = Database::instance();
    auto lignes = base.requete(
        "SELECT id, employe_id, DATE_FORMAT(date_absence, '%d/%m/%Y'), IFNULL(motif, ''), "
        "justifiee, annulee FROM absence ORDER BY id");

    AbsencesLues res;
    for (const auto& l : lignes) {
        try {
            res.liste.emplace_back(versEntier(l[0]), versEntier(l[1]), versTexte(l[2]),
                                   versTexte(l[3]), versEntier(l[4]) != 0, versEntier(l[5]) != 0);
        } catch (const std::exception& ex) {
            std::cerr << "Absence " << versTexte(l[0]) << " ignoree (donnees invalides) : "
                      << ex.what() << "\n";
        }
    }
    auto maxId = base.requete("SELECT IFNULL(MAX(id), 0) FROM absence");
    if (!maxId.empty()) res.dernierId = versEntier(maxId[0][0]);
    return res;
}

void sauvegarderAbsences(const std::vector<Absence>& absences, int idSupprime) {
    Database& base = Database::instance();
    Transaction tx(base);   // ROLLBACK automatique si une instruction echoue (exception)

    if (idSupprime > 0) {
        base.executer("DELETE FROM absence WHERE id = " + n(idSupprime));
    }
    for (const auto& a : absences) {
        base.executer(
            "INSERT INTO absence (id, employe_id, date_absence, motif, justifiee, annulee) VALUES ("
            + n(a.getId()) + ", " + n(a.getIdEmploye()) + ", "
            + q(dateFrVersSql(a.getDateAbsence())) + ", " + q(a.getMotif()) + ", "
            + (a.estJustifiee() ? "1" : "0") + ", " + (a.estAnnulee() ? "1" : "0") + ") "
            "ON DUPLICATE KEY UPDATE date_absence = VALUES(date_absence), motif = VALUES(motif), "
            "justifiee = VALUES(justifiee), annulee = VALUES(annulee)");
    }
    tx.commit();
}

} // namespace db
