#include "../../headers/admin/EspaceAdmin.h"
#include "../../headers/Date.h"

#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <exception>

using namespace std;

// ==========================================
// HELPERS DE SAISIE SURE
// HIGH-003 : evite les variables non initialisées
// MED-001  : rejette les suffixes parasites ("7.9abc")
// ==========================================

namespace {

// Lit un entier strict. Retourne false si la saisie est invalide.
bool lireEntier(const string& prompt, int& sortie) {
    cout << prompt;
    if (!(cin >> sortie)) {
        if (cin.eof()) return false;   // fin de flux : ne pas bloquer
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return false;
    }
    // Reject "1abc" : the whole line must be consumed
    char restant;
    if (cin.get(restant)) {
        if (restant != '\n') {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return false;
        }
    }
    return true;
}

// Lit un double strict. Retourne false si la saisie est invalide.
bool lireDouble(const string& prompt, double& sortie) {
    cout << prompt;
    if (!(cin >> sortie)) {
        if (cin.eof()) return false;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return false;
    }
    char restant;
    if (cin.get(restant)) {
        if (restant != '\n') {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return false;
        }
    }
    return true;
}

} // namespace

// ==========================================
// CONSTRUCTEUR
// ==========================================

EspaceAdmin::EspaceAdmin(GestionnaireEmployes& g)
    : gestionnaire(g) {
}

// ==========================================
// MENU ADMINISTRATEUR
// ==========================================

void EspaceAdmin::menuAdmin() {

    int choix;

    do {
        cout << "\n========================================\n";
        cout << "        ESPACE ADMINISTRATEUR\n";
        cout << "========================================\n";
        cout << "1. Ajouter un employe\n";
        cout << "2. Modifier un employe\n";
        cout << "3. Supprimer un employe\n";
        cout << "4. Rechercher un employe\n";
        cout << "5. Consulter un employe\n";
        cout << "6. Afficher tous les employes\n";
        cout << "0. Retour\n";
        cout << "========================================\n";

        cout << "Votre choix : ";

        if (!(cin >> choix)) {
            // EOF ou saisie non numerique
            if (cin.eof()) {
                cout << "\nFin de saisie. Retour au menu principal.\n";
                return;
            }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "\nSaisie invalide.\n";
            choix = -1;
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        switch (choix) {

            case 1:
                ajouterEmploye();
                break;

            case 2:
                modifierEmploye();
                break;

            case 3:
                supprimerEmploye();
                break;

            case 4:
                rechercherEmploye();
                break;

            case 5:
                consulterEmploye();
                break;

            case 6:
                afficherEmployes();
                break;

            case 0:
                cout << "\nRetour au menu principal.\n";
                break;

            default:
                cout << "\nChoix invalide !\n";
        }

    } while (choix != 0);
}


// ==========================================
// 1. AJOUTER UN EMPLOYE
// ==========================================

void EspaceAdmin::ajouterEmploye() {

    DonneesEmploye donnees;

    cout << "\n========== AJOUTER UN EMPLOYE ==========\n";

    cout << "Nom : ";
    getline(cin, donnees.nom);

    cout << "Prenom : ";
    getline(cin, donnees.prenom);

    cout << "CIN : ";
    getline(cin, donnees.cin);

    cout << "Email : ";
    getline(cin, donnees.email);

    cout << "Telephone : ";
    getline(cin, donnees.telephone);

    string naissance;
    string embauche;

    cout << "Date de naissance (JJ/MM/AAAA) : ";
    getline(cin, naissance);

    cout << "Date d'embauche (JJ/MM/AAAA) : ";
    getline(cin, embauche);

    try {

        donnees.dateNaissance = Date::fromString(naissance);
        donnees.dateEmbauche = Date::fromString(embauche);

        if (!lireDouble("Salaire de base : ", donnees.salaireBase)) {
            cout << "\nSalaire invalide. Ajout annule.\n";
            return;
        }

        donnees.statut = "Actif";

        int id = gestionnaire.ajouter(donnees);

        cout << "\nEmploye ajoute avec succes !\n";
        cout << "Identifiant de l'employe : " << id << endl;

    }
    catch (const exception& e) {

        cout << "\nErreur lors de l'ajout : "
             << e.what() << endl;
    }
}


// ==========================================
// 2. MODIFIER UN EMPLOYE
// ==========================================

void EspaceAdmin::modifierEmploye() {

    int id = 0;

    cout << "\n========== MODIFIER UN EMPLOYE ==========\n";

    if (!lireEntier("Entrer l'identifiant de l'employe : ", id)) {
        cout << "\nIdentifiant invalide.\n";
        return;
    }

    try {

        const Employe* employe = gestionnaire.rechercher(id);

        if (employe == nullptr) {

            cout << "\nEmploye introuvable !\n";
            return;
        }

        cout << "\nInformations actuelles :\n";
        employe->afficherInfos();

        DonneesEmploye donnees;

        cout << "\n========== NOUVELLES INFORMATIONS ==========\n";

        cout << "Nouveau nom : ";
        getline(cin, donnees.nom);

        cout << "Nouveau prenom : ";
        getline(cin, donnees.prenom);

        cout << "Nouveau CIN : ";
        getline(cin, donnees.cin);

        cout << "Nouvel email : ";
        getline(cin, donnees.email);

        cout << "Nouveau telephone : ";
        getline(cin, donnees.telephone);

        string naissance;
        string embauche;

        cout << "Nouvelle date de naissance (JJ/MM/AAAA) : ";
        getline(cin, naissance);

        cout << "Nouvelle date d'embauche (JJ/MM/AAAA) : ";
        getline(cin, embauche);

        donnees.dateNaissance = Date::fromString(naissance);
        donnees.dateEmbauche = Date::fromString(embauche);

        if (!lireDouble("Nouveau salaire de base : ", donnees.salaireBase)) {
            cout << "\nSalaire invalide. Modification annulee.\n";
            return;
        }

        cout << "Nouveau statut (Actif/Inactif) : ";
        getline(cin, donnees.statut);

        gestionnaire.modifier(id, donnees);

        cout << "\nModification effectuee avec succes !\n";

    }
    catch (const exception& e) {

        cout << "\nErreur lors de la modification : "
             << e.what() << endl;
    }
}


// ==========================================
// 3. SUPPRIMER UN EMPLOYE
// ==========================================

void EspaceAdmin::supprimerEmploye() {

    int id = 0;
    char confirmation;

    cout << "\n========== SUPPRIMER UN EMPLOYE ==========\n";

    if (!lireEntier("Entrer l'identifiant de l'employe : ", id)) {
        cout << "\nIdentifiant invalide.\n";
        return;
    }

    try {

        const Employe* employe = gestionnaire.rechercher(id);

        if (employe == nullptr) {

            cout << "\nEmploye introuvable !\n";
            return;
        }

        cout << "\nInformations de l'employe :\n";
        employe->afficherInfos();

        cout << "\nVoulez-vous vraiment supprimer cet employe ? (O/N) : ";
        cin >> confirmation;

        if (confirmation == 'O' || confirmation == 'o') {

            gestionnaire.supprimer(id);

            cout << "\nEmploye supprime avec succes !\n";
        }
        else {

            cout << "\nSuppression annulee.\n";
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

    }
    catch (const exception& e) {

        cout << "\nErreur lors de la suppression : "
             << e.what() << endl;
    }
}


// ==========================================
// 4. RECHERCHER UN EMPLOYE PAR NOM
// ==========================================

void EspaceAdmin::rechercherEmploye() {

    string nom;

    cout << "\n========== RECHERCHER UN EMPLOYE ==========\n";

    cout << "Entrer le nom ou le prenom : ";
    getline(cin, nom);

    try {

        vector<const Employe*> resultats =
            gestionnaire.rechercherParNom(nom);

        if (resultats.empty()) {

            cout << "\nAucun employe trouve.\n";
            return;
        }

        cout << "\nNombre de resultats : "
             << resultats.size() << endl;

        for (size_t i = 0; i < resultats.size(); i++) {

            cout << "\n--------- EMPLOYE "
                 << i + 1 << " ---------\n";

            resultats[i]->afficherInfos();
        }

    }
    catch (const exception& e) {

        cout << "\nErreur lors de la recherche : "
             << e.what() << endl;
    }
}


// ==========================================
// 5. CONSULTER UN EMPLOYE PAR IDENTIFIANT
// ==========================================

void EspaceAdmin::consulterEmploye() {

    int id = 0;

    cout << "\n========== CONSULTER UN EMPLOYE ==========\n";

    if (!lireEntier("Entrer l'identifiant de l'employe : ", id)) {
        cout << "\nIdentifiant invalide.\n";
        return;
    }

    try {

        const Employe* employe = gestionnaire.rechercher(id);

        if (employe == nullptr) {

            cout << "\nEmploye introuvable !\n";
            return;
        }

        cout << "\n========== INFORMATIONS DE L'EMPLOYE ==========\n";

        employe->afficherInfos();

        cout << "\n==============================================\n";

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

    }
    catch (const exception& e) {

        cout << "\nErreur lors de la consultation : "
             << e.what() << endl;
    }
}


// ==========================================
// 6. AFFICHER TOUS LES EMPLOYES
// ==========================================

void EspaceAdmin::afficherEmployes() {

    cout << "\n========== LISTE DES EMPLOYES ==========\n";

    try {

        vector<Employe> employes = gestionnaire.lister();

        if (employes.empty()) {

            cout << "\nAucun employe enregistre.\n";
            return;
        }

        cout << "\nNombre total d'employes : "
             << employes.size() << endl;

        for (size_t i = 0; i < employes.size(); i++) {

            cout << "\n------------ EMPLOYE "
                 << i + 1 << " ------------\n";

            employes[i].afficherInfos();
        }

        cout << "\n========================================\n";

    }
    catch (const exception& e) {

        cout << "\nErreur lors de l'affichage : "
             << e.what() << endl;
    }
}
