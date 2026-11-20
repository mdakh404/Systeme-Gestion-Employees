#include <iostream>
#include <string>
#include <exception>
#include "GestionnaireConges.h"
#include "MenuConge.h"

using namespace std;

static int executer()
{
    GestionnaireConges gc;
    MenuConge menu(gc);

    int choix = -1;
    do {
        cout << endl << "===== GESTION DES CONGES =====" << endl;
        cout << "1. Espace Employe" << endl;
        cout << "2. Espace Responsable RH" << endl;
        cout << "0. Quitter" << endl;
        cout << "Choix : ";

        string ligne;
        if (!getline(cin, ligne)) {
            break;
        }
        choix = (ligne.size() == 1 && ligne[0] >= '0' && ligne[0] <= '2') ? ligne[0] - '0' : -1;

        if (choix == 1) {
            cout << "Votre ID employe : ";
            string idTexte;
            if (!getline(cin, idTexte)) {
                break;
            }
            try {
                menu.lancerEmploye(stoi(idTexte));
            }
            catch (const exception&) {
                cout << "ID employe invalide." << endl;
            }
        }
        else if (choix == 2) {
            menu.lancerRH();
        }
        else if (choix != 0) {
            cout << "Choix invalide." << endl;
        }
    } while (choix != 0);

    cout << "Au revoir." << endl;
    return 0;
}

int main()
{
    try {
        return executer();
    }
    catch (const exception& e) {
        cerr << "Erreur fatale : " << e.what() << endl;
        cerr << "Verifiez le fichier data/conges.txt (format : id;idEmploye;debut;fin;type;statut)." << endl;
        return 1;
    }
}
