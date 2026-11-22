#include "MenuConge.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <stdexcept>

using namespace std;


MenuConge::MenuConge(GestionnaireConges& gestionnaire, istream& entree)
    : gc(gestionnaire),
      entree(entree)
{
}

MenuConge::~MenuConge()
{
    //dtor
}



// Lit une ligne, enleve les espaces autour. Retourne false si le flux est ferme.
bool MenuConge::lireTexte(const string& invite, string& texte)
{
    cout << invite;
    if (!getline(entree, texte)) {
        return false;                       // fin de saisie : on ne boucle pas
    }
    size_t debut = texte.find_first_not_of(" \t\r");
    if (debut == string::npos) {
        texte = "";
        return true;
    }
    size_t fin = texte.find_last_not_of(" \t\r");
    texte = texte.substr(debut, fin - debut + 1);
    return true;
}

// Redemande jusqu'a obtenir un entier entre minimum et maximum.
bool MenuConge::lireEntier(const string& invite, int minimum, int maximum, int& valeur)
{
    string texte;
    while (true) {
        if (!lireTexte(invite, texte)) {
            return false;
        }

        bool ok = !texte.empty() && texte.size() <= 9;
        for (size_t i = 0; ok && i < texte.size(); i++) {
            if (!isdigit((unsigned char)texte[i])) {
                ok = false;
            }
        }

        if (ok) {
            int n = stoi(texte);
            if (n >= minimum && n <= maximum) {
                valeur = n;
                return true;
            }
        }
        cout << "Saisie invalide : entrez un nombre entre "
             << minimum << " et " << maximum << "." << endl;
    }
}

// Verifie seulement la FORME AAAA-MM-JJ.

bool MenuConge::formatDateOk(const string& date)
{
    if (date.size() != 10) {
        return false;
    }
    for (size_t i = 0; i < date.size(); i++) {
        if (i == 4 || i == 7) {
            if (date[i] != '-') {
                return false;
            }
        }
        else if (!isdigit((unsigned char)date[i])) {
            return false;
        }
    }
    return true;
}

// Redemande tant que la forme est mauvaise. "0" = annuler (retourne false).
bool MenuConge::lireDate(const string& invite, string& date)
{
    while (true) {
        if (!lireTexte(invite, date)) {
            return false;
        }
        if (date == "0") {
            return false;
        }
        if (formatDateOk(date)) {
            return true;
        }
        cout << "Format invalide. Exemple attendu : 2026-12-10" << endl;
    }
}



string MenuConge::indicateurStatut(const string& statut)
{
    if (statut.compare(0, 6, "Accept") == 0) {
        return "[ACCEPTEE]";
    }
    if (statut.compare(0, 5, "Refus") == 0) {
        return "[REFUSEE]";
    }
    return "[EN ATTENTE]";
}

void MenuConge::afficherTableau(const vector<Conge>& liste) const
{
    cout << endl;
    cout << left
         << setw(6)  << "ID"
         << setw(10) << "Employe"
         << setw(13) << "Debut"
         << setw(13) << "Fin"
         << setw(13) << "Type"
         << "Statut" << endl;
    cout << string(66, '-') << endl;

    vector<Conge>::const_iterator it;
    for (it = liste.begin(); it != liste.end(); ++it) {
        cout << left
             << setw(6)  << it->Getid()
             << setw(10) << it->GetidEmploye()
             << setw(13) << it->GetdateDebut()
             << setw(13) << it->GetdateFin()
             << setw(13) << it->Gettype()
             << indicateurStatut(it->Getstatut()) << endl;
    }
    cout << string(66, '-') << endl;
}

void MenuConge::afficherMenuEmploye() const
{
    cout << endl;
    cout << "===== ESPACE EMPLOYE =====" << endl;
    cout << "1. Deposer une demande de conge" << endl;
    cout << "2. Consulter mes conges (historique)" << endl;
    cout << "3. Mon bilan et mon solde" << endl;
    cout << "0. Retour" << endl;
}

void MenuConge::lancerEmploye(int idEmploye)
{
    if (idEmploye <= 0) {
        throw invalid_argument("Identifiant employe invalide.");
    }

    int choix = -1;
    do {
        afficherMenuEmploye();
        if (!lireEntier("Votre choix : ", 0, 3, choix)) {
            return;                         // fin de saisie
        }

        try {
            switch (choix) {
                case 1: deposerDemande(idEmploye);      break;
                case 2: consulterHistorique(idEmploye); break;
                case 3: gc.afficherStatistiques(idEmploye); break;
                default: break;
            }
        }
        catch (const invalid_argument& e) {
            cout << "Operation refusee : " << e.what() << endl;
        }
        catch (const runtime_error& e) {
            cout << "Erreur de fichier : " << e.what() << endl;
        }
    } while (choix != 0);
}

void MenuConge::deposerDemande(int idEmploye)
{
    cout << endl << "--- Deposer une demande (0 = annuler) ---" << endl;

    string debut, fin;
    if (!lireDate("Date de debut (AAAA-MM-JJ) : ", debut)) {
        cout << "Depot annule." << endl;
        return;
    }
    if (!lireDate("Date de fin   (AAAA-MM-JJ) : ", fin)) {
        cout << "Depot annule." << endl;
        return;
    }

    cout << "Type de conge :" << endl;
    cout << "  1. Annuel" << endl;
    cout << "  2. Maladie" << endl;
    cout << "  3. Sans solde" << endl;
    cout << "  4. Exceptionnel" << endl;

    int numero = 0;
    if (!lireEntier("Votre choix (0 = annuler) : ", 0, 4, numero) || numero == 0) {
        cout << "Depot annule." << endl;
        return;
    }

    string types[4] = { "Annuel", "Maladie", "Sans solde", "Exceptionnel" };

   
    gc.deposerDemande(idEmploye, debut, fin, types[numero - 1]);
}

void MenuConge::consulterHistorique(int idEmploye)
{
    vector<Conge> historique = gc.consulterHistorique(idEmploye);

    cout << endl << "--- Mes conges (employe " << idEmploye << ") ---" << endl;
    if (historique.empty()) {
        cout << "Aucune demande enregistree." << endl;
        return;
    }
    afficherTableau(historique);
}

void MenuConge::afficherMenuRH() const
{
    cout << endl;
    cout << "===== ESPACE RESPONSABLE RH =====" << endl;
    cout << "1. Traiter les demandes en attente" << endl;
    cout << "2. Consulter tous les conges" << endl;
    cout << "3. Bilan d'un employe" << endl;
    cout << "4. Statistiques globales" << endl;
    cout << "0. Retour" << endl;
}

void MenuConge::lancerRH()
{
    int choix = -1;
    do {
        afficherMenuRH();
        if (!lireEntier("Votre choix : ", 0, 4, choix)) {
            return;
        }

        try {
            switch (choix) {
                case 1: traiterDemandes();          break;
                case 2: consulterTousLesConges();   break;
                case 3: bilanEmploye();             break;
                case 4: gc.afficherStatistiques();  break;
                default: break;
            }
        }
        catch (const invalid_argument& e) {
            cout << "Operation refusee : " << e.what() << endl;
        }
        catch (const runtime_error& e) {
            cout << "Erreur de fichier : " << e.what() << endl;
        }
    } while (choix != 0);
}

void MenuConge::traiterDemandes()
{
    vector<Conge> attente = gc.listerDemandesEnAttente();

    cout << endl << "--- Demandes en attente ---" << endl;
    if (attente.empty()) {
        cout << "Aucune demande en attente." << endl;
        return;
    }
    afficherTableau(attente);

    int id = 0;
    if (!lireEntier("ID de la demande a traiter (0 = annuler) : ", 0, 999999999, id) || id == 0) {
        cout << "Traitement annule." << endl;
        return;
    }

   
    Conge cherche(id, 0, "", "", "");
    vector<Conge>::iterator it = find(attente.begin(), attente.end(), cherche);
    if (it == attente.end()) {
        cout << "Demande introuvable ou deja traitee. Aucune modification." << endl;
        return;
    }

    cout << endl << *it;
    int duree = gc.dureeJours(*it);
    cout << "Duree demandee : " << duree << " jour(s)" << endl;

    if (it->Gettype() == "Annuel") {
        int annee = stoi(it->GetdateDebut().substr(0, 4));
        int previsionnel = gc.soldeRestant(it->GetidEmploye(), annee)
                         - gc.joursDansAnnee(*it, annee);
        cout << "Solde previsionnel : " << previsionnel << " / "
             << GestionnaireConges::QUOTA_ANNUEL << " jours" << endl;
        if (previsionnel < 0) {
            cout << "ATTENTION : le quota annuel serait depasse." << endl;
        }
    }
    else {
        cout << "Ce type de conge ne consomme pas le solde annuel." << endl;
    }

    int decision = 0;
    if (!lireEntier("1. Accepter   2. Refuser   0. Annuler : ", 0, 2, decision) || decision == 0) {
        cout << "Traitement annule." << endl;
        return;
    }

    gc.traiterDemande(id, decision == 1);   
    cout << "Demande " << id << (decision == 1 ? " acceptee." : " refusee.") << endl;
}

void MenuConge::consulterTousLesConges()
{
    vector<Conge> tous = gc.tousLesConges();

    cout << endl << "--- Tous les conges ---" << endl;
    if (tous.empty()) {
        cout << "Aucun conge enregistre." << endl;
        return;
    }
    afficherTableau(tous);
}

void MenuConge::bilanEmploye()
{
    int id = 0;
    if (!lireEntier("ID de l'employe (0 = annuler) : ", 0, 999999999, id) || id == 0) {
        return;
    }
    consulterHistorique(id);
    gc.afficherStatistiques(id);
}
