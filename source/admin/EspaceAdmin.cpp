#include "../../headers/admin/EspaceAdmin.h"
#include "../../headers/Date.h"

#include <iostream>
#include <string>
#include <vector>
#include <exception>

using namespace std;

namespace {

bool lireLigne(string& s) {
    if (!getline(cin, s)) return false;
    return true;
}

bool lireEntier(int& val) {
    string s;
    if (!getline(cin, s)) return false;
    try {
        val = stoi(s);
        return true;
    } catch (...) {
        return false;
    }
}

bool lireDouble(double& val) {
    string s;
    if (!getline(cin, s)) return false;
    try {
        val = stod(s);
        return true;
    } catch (...) {
        return false;
    }
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

    while (true) {
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
        int choix = -1;
        if (!lireEntier(choix)) {
            if (cin.eof()) break;
            cout << "\nChoix invalide !\n";
            continue;
        }

        if (choix == 0) {
            cout << "\nRetour au menu principal.\n";
            break;
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

            default:
                cout << "\nChoix invalide !\n";
                break;
        }
    }
}

// ==========================================
// 1. AJOUTER UN EMPLOYE
// ==========================================

void EspaceAdmin::ajouterEmploye() {

    DonneesEmploye donnees;

    cout << "\n========== AJOUTER UN EMPLOYE ==========\n";

    cout << "Nom : ";
    if (!lireLigne(donnees.nom)) return;

    cout << "Prenom : ";
    if (!lireLigne(donnees.prenom)) return;

    cout << "CIN : ";
    if (!lireLigne(donnees.cin)) return;

    cout << "Email : ";
    if (!lireLigne(donnees.email)) return;

    cout << "Telephone : ";
    if (!lireLigne(donnees.telephone)) return;

    string naissance;
    string embauche;

    cout << "Date de naissance (JJ/MM/AAAA) : ";
    if (!lireLigne(naissance)) return;

    cout << "Date d'embauche (JJ/MM/AAAA) : ";
    if (!lireLigne(embauche)) return;

    try {
        donnees.dateNaissance = Date::fromString(naissance);
        donnees.dateEmbauche = Date::fromString(embauche);

        cout << "Salaire de base : ";
        if (!lireDouble(donnees.salaireBase)) {
            if (cin.eof()) return;
            cout << "\nErreur lors de l'ajout : salaire de base invalide (nombre attendu)\n";
            return;
        }

        donnees.statut = "Actif";

        int id = gestionnaire.ajouter(donnees);
        try {
            gestionnaire.sauvegarderDansFichier();
        } catch (...) {
            gestionnaire.supprimer(id);   // la base a refuse l'employe : on annule l'ajout en memoire
            throw;
        }

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

    cout << "\n========== MODIFIER UN EMPLOYE ==========\n";

    cout << "Entrer l'identifiant de l'employe : ";
    int id = 0;
    if (!lireEntier(id)) {
        if (!cin.eof()) cout << "\nIdentifiant invalide !\n";
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
        if (!lireLigne(donnees.nom)) return;

        cout << "Nouveau prenom : ";
        if (!lireLigne(donnees.prenom)) return;

        cout << "Nouveau CIN : ";
        if (!lireLigne(donnees.cin)) return;

        cout << "Nouvel email : ";
        if (!lireLigne(donnees.email)) return;

        cout << "Nouveau telephone : ";
        if (!lireLigne(donnees.telephone)) return;

        string naissance;
        string embauche;

        cout << "Nouvelle date de naissance (JJ/MM/AAAA) : ";
        if (!lireLigne(naissance)) return;

        cout << "Nouvelle date d'embauche (JJ/MM/AAAA) : ";
        if (!lireLigne(embauche)) return;

        donnees.dateNaissance = Date::fromString(naissance);
        donnees.dateEmbauche = Date::fromString(embauche);

        cout << "Nouveau salaire de base : ";
        if (!lireDouble(donnees.salaireBase)) {
            if (cin.eof()) return;
            cout << "\nSalaire de base invalide !\n";
            return;
        }

        cout << "Nouveau statut (Actif/Inactif) : ";
        if (!lireLigne(donnees.statut)) return;

        gestionnaire.modifier(id, donnees); gestionnaire.sauvegarderDansFichier();

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

    cout << "\n========== SUPPRIMER UN EMPLOYE ==========\n";

    cout << "Entrer l'identifiant de l'employe : ";
    int id = 0;
    if (!lireEntier(id)) {
        if (!cin.eof()) cout << "\nIdentifiant invalide !\n";
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
        string conf;
        if (!lireLigne(conf)) return;
        char confirmation = conf.empty() ? 'N' : conf[0];

        if (confirmation == 'O' || confirmation == 'o') {
            gestionnaire.supprimer(id); gestionnaire.sauvegarderDansFichier();
            cout << "\nEmploye supprime avec succes !\n";
        }
        else {
            cout << "\nSuppression annulee.\n";
        }

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
    if (!lireLigne(nom)) return;

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

    cout << "\n========== CONSULTER UN EMPLOYE ==========\n";

    cout << "Entrer l'identifiant de l'employe : ";
    int id = 0;
    if (!lireEntier(id)) {
        if (!cin.eof()) cout << "\nIdentifiant invalide !\n";
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
