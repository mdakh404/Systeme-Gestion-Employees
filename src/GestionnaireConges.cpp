#include "GestionnaireConges.h"

#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iterator>
#include <cctype>
#include <iomanip>

using namespace std;

GestionnaireConges::GestionnaireConges() {
    chargerDepuisFichier();
}

bool GestionnaireConges::dateValide(string date) const {

    if (date.size() != 10 || date[4] != '-' || date[7] != '-') {
        return false;
    }
    for (size_t i = 0; i < date.size(); i++) {
        if (i != 4 && i != 7 && !isdigit((unsigned char)date[i])) {
            return false;
        }
    }

    int annee, mois, jour;
    char sep1, sep2;

    stringstream ss(date);
    ss >> annee >> sep1 >> mois >> sep2 >> jour;
    if (ss.fail() || sep1 != '-' || sep2 != '-' || !ss.eof()) {
        return false;
    }
    if (annee < 1 || mois < 1 || mois > 12 || jour < 1) {
        return false;
    }
    int joursMois[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };
    if ((annee % 400 == 0) ||
        (annee % 4 == 0 && annee % 100 != 0)) {
        joursMois[1] = 29;
    }

    return jour <= joursMois[mois - 1];
}

bool GestionnaireConges::chevauchement(
    int idEmploye, string debut, string fin) const {

    auto it = find_if(
        conges.begin(),
        conges.end(),
        [idEmploye, debut, fin](const Conge& c) {

            if (c.GetidEmploye() != idEmploye) {
                return false;
            }

            if (!estAcceptee(c.Getstatut()) &&
                !estEnAttente(c.Getstatut())) {
                return false;
            }

            return debut <= c.GetdateFin() &&
                   c.GetdateDebut() <= fin;
        }
    );

     return it != conges.end();
}

void GestionnaireConges::deposerDemande(
    int idEmploye, string debut, string fin, string type) {

    if (idEmploye <= 0) {
        throw invalid_argument("Identifiant employe invalide.");
    }

    if (!dateValide(debut) || !dateValide(fin)) {
        throw invalid_argument(
            "Date invalide. Format attendu : AAAA-MM-JJ.");
    }

    if (debut > fin) {
        throw invalid_argument(
            "La date de debut doit etre avant la date de fin.");
    }

    if (type != "Annuel" &&
        type != "Maladie" &&
        type != "Sans solde" &&
        type != "Exceptionnel") {

        throw invalid_argument("Type de conge invalide.");
    }

    if (chevauchement(idEmploye, debut, fin)) {
        throw invalid_argument(
            "Une autre demande existe deja sur cette periode.");
    }

    int nouvelId = 1;

    for (const Conge& c : conges) {
        if (c.Getid() >= nouvelId) {
            nouvelId = c.Getid() + 1;
        }
    }

    Conge nouvelleDemande(
        nouvelId,
        idEmploye,
        debut,
        fin,
        type,
        "En attente"
    );

    conges.push_back(nouvelleDemande);

    sauvegarderDansFichier();

    cout << "\nDemande deposee avec succes !" << endl;
    cout << "ID de la demande : " << nouvelId << endl;
    cout << "Statut : En attente" << endl;
}

vector<Conge> GestionnaireConges::consulterHistorique(
    int idEmploye) const {

    vector<Conge> historique;

    auto it = conges.begin();

    while (it != conges.end()) {

        it = find_if(
            it,
            conges.end(),
            [idEmploye](const Conge& c) {
                return c.GetidEmploye() == idEmploye;
            }
        );

        if (it != conges.end()) {
            historique.push_back(*it);
            ++it;
        }
    }

    return historique;
}

bool GestionnaireConges::estAcceptee(const string& statut) {
    return statut == "Acceptée" || statut == "Acceptee";
}

bool GestionnaireConges::estEnAttente(const string& statut) {
    return statut == "En attente";
}

bool GestionnaireConges::estRefusee(const string& statut) {
    return statut == "Refusée" || statut == "Refusee";
}

long GestionnaireConges::joursDepuisEpoque(const string& date) {
    int a = stoi(date.substr(0, 4));
    int m = stoi(date.substr(5, 2));
    int j = stoi(date.substr(8, 2));

    a -= m <= 2;
    long era = (a >= 0 ? a : a - 399) / 400;
    long yoe = a - era * 400;
    long doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + j - 1;
    long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;

    return era * 146097 + doe - 719468;
}

int GestionnaireConges::dureeJours(const Conge& c) const {
    return (int)(joursDepuisEpoque(c.GetdateFin())
               - joursDepuisEpoque(c.GetdateDebut())) + 1;
}

bool GestionnaireConges::chevauchementAccepte(const Conge& cible) const {

    auto it = find_if(
        conges.begin(),
        conges.end(),
        [&cible](const Conge& c) {
            return c.Getid() != cible.Getid()
                && c.GetidEmploye() == cible.GetidEmploye()
                && estAcceptee(c.Getstatut())
                && cible.GetdateDebut() <= c.GetdateFin()
                && c.GetdateDebut() <= cible.GetdateFin();
        }
    );

    return it != conges.end();
}

vector<Conge> GestionnaireConges::listerDemandesEnAttente() const {

    vector<Conge> enAttente;

    copy_if(
        conges.begin(),
        conges.end(),
        back_inserter(enAttente),
        [](const Conge& c) { return estEnAttente(c.Getstatut()); }
    );

    sort(
        enAttente.begin(),
        enAttente.end(),
        [](const Conge& a, const Conge& b) {
            return a.GetdateDebut() < b.GetdateDebut();
        }
    );

    return enAttente;
}

void GestionnaireConges::traiterDemande(int idConge, bool accepter) {

    auto it = find_if(
        conges.begin(),
        conges.end(),
        [idConge](const Conge& c) { return c.Getid() == idConge; }
    );

    if (it == conges.end()) {
        throw invalid_argument("Demande de conge introuvable.");
    }

    if (!estEnAttente(it->Getstatut())) {
        throw invalid_argument(
            "Cette demande a deja ete traitee (" + it->Getstatut() + ").");
    }

    if (accepter && chevauchementAccepte(*it)) {
        throw invalid_argument(
            "Impossible d'accepter : chevauchement avec un conge deja accepte.");
    }

    it->Setstatut(accepter ? "Acceptée" : "Refusée");

    sauvegarderDansFichier();
}

int GestionnaireConges::soldeRestant(int idEmploye) const {

    int pris = 0;

    for (const Conge& c : conges) {
        if (c.GetidEmploye() == idEmploye
            && c.Gettype() == "Annuel"
            && estAcceptee(c.Getstatut())) {
            pris += dureeJours(c);
        }
    }

    return QUOTA_ANNUEL - pris;
}

StatistiquesConges GestionnaireConges::calculerStatistiques(int idEmploye) const {

    StatistiquesConges s;

    for (const Conge& c : conges) {

        if (idEmploye != -1 && c.GetidEmploye() != idEmploye) {
            continue;
        }

        s.total++;

        if (estAcceptee(c.Getstatut())) {
            s.acceptees++;
            s.joursPris += dureeJours(c);
        }
        else if (estRefusee(c.Getstatut())) {
            s.refusees++;
        }
        else {
            s.enAttente++;
        }
    }

    int traitees = s.acceptees + s.refusees;
    s.tauxAcceptation = traitees > 0 ? 100.0 * s.acceptees / traitees : 0.0;

    return s;
}

void GestionnaireConges::afficherStatistiques(int idEmploye) const {

    StatistiquesConges s = calculerStatistiques(idEmploye);

    cout << "\n===== STATISTIQUES";
    if (idEmploye != -1) {
        cout << " - EMPLOYE " << idEmploye;
    }
    else {
        cout << " GLOBALES";
    }
    cout << " =====" << endl;

    cout << left
         << setw(28) << "Total des demandes"   << ": " << s.total     << endl
         << setw(28) << "[EN ATTENTE]"         << ": " << s.enAttente << endl
         << setw(28) << "[ACCEPTEE]"           << ": " << s.acceptees << endl
         << setw(28) << "[REFUSEE]"            << ": " << s.refusees  << endl
         << setw(28) << "Total jours pris"     << ": " << s.joursPris << endl
         << setw(28) << "Taux d'acceptation"   << ": "
         << fixed << setprecision(1) << s.tauxAcceptation << " %" << endl;

    if (idEmploye != -1) {
        cout << setw(28) << "Solde annuel restant" << ": "
             << soldeRestant(idEmploye) << " / " << QUOTA_ANNUEL << " jours" << endl;
    }
}

void GestionnaireConges::chargerDepuisFichier() {
    conges = fichiers.chargerTousLesConges();
}

void GestionnaireConges::sauvegarderDansFichier() const {
    fichiers.sauvegarderTousLesConges(conges);
}

GestionnaireConges::~GestionnaireConges()
{
    //dtor
}

