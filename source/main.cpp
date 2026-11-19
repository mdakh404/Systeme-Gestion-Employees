#include <iostream>
#include <exception>

#include "../headers/GestionnaireFichiers.h"
#include "../headers/GestionnaireEmployes.h"
#include "../headers/admin/EspaceAdmin.h"

using namespace std;

int main() {

    try {

        // 1. Creation du gestionnaire de fichiers
        GestionnaireFichiers fichiers;

        // 2. Creation du gestionnaire des employes
        GestionnaireEmployes gestionnaire(fichiers);

        // 3. Chargement des employes existants
        gestionnaire.charger();

        // 4. Creation de l'espace administrateur
        EspaceAdmin admin(gestionnaire);

        // 5. Affichage du menu administrateur
        cout << "\n====================================\n";
        cout << " BIENVENUE DANS LA GESTION DES EMPLOYES\n";
        cout << "====================================\n";

        admin.menuAdmin();

    }
    catch (const exception& e) {

        cout << "\nErreur dans le programme : "
             << e.what() << endl;

    }

    cout << "\nFin du programme.\n";

    return 0;
}
