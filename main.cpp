#include "EspaceEmploye.h"
#include "GestionnaireFichiers.h"
#include "GestionnaireEmployes.h"
#include <iostream>
#include <string>

static void initialiserDemo(GestionnaireEmployes& gestion, GestionnaireFichiers& fichiers) {
    if (gestion.nombre() > 0) return;

    fichiers.sauvegarderDepartements({
        Departement(1, "Ressources Humaines", "Gestion du personnel"),
        Departement(2, "Informatique", "Developpement et infrastructure")
    });

    DonneesEmploye sara;
    sara.nom = "Alaoui";
    sara.prenom = "Sara";
    sara.cin = "AB12345";
    sara.dateNaissance = Date(15, 3, 1995);
    sara.email = "sara.alaoui@entreprise.ma";
    sara.telephone = "0612345678";
    sara.dateEmbauche = Date(1, 9, 2020);
    sara.statut = "Actif";
    sara.salaireBase = 8500.0;
    int idSara = gestion.ajouter(sara);
    gestion.affecterDepartement(idSara, 1);

    DonneesEmploye youssef;
    youssef.nom = "Benali";
    youssef.prenom = "Youssef";
    youssef.cin = "CD67890";
    youssef.dateNaissance = Date(22, 7, 1990);
    youssef.email = "youssef.benali@entreprise.ma";
    youssef.telephone = "0698765432";
    youssef.dateEmbauche = Date(10, 1, 2018);
    youssef.statut = "Actif";
    youssef.salaireBase = 12000.0;
    int idYoussef = gestion.ajouter(youssef);
    gestion.affecterDepartement(idYoussef, 2);

    std::cout << "Donnees de demonstration creees.\n"
              << "  Sara    -> matricule EMP0001  /  CIN AB12345\n"
              << "  Youssef -> matricule EMP0002  /  CIN CD67890\n\n";
}

static void testsIntegration(GestionnaireEmployes& gestion) {
    std::cout << "\n===== Tests d'integration (espace Employe) =====\n";
    int ok = 0, ko = 0;
    auto verif = [&](bool cond, const std::string& msg) {
        std::cout << (cond ? "[OK] " : "[KO] ") << msg << "\n";
        (cond ? ++ok : ++ko);
    };

    const Employe* sara = gestion.rechercherParMatricule("EMP0001");
    const Employe* youssef = gestion.rechercherParMatricule("EMP0002");
    verif(sara != nullptr, "Connexion possible par matricule EMP0001");
    verif(youssef != nullptr, "Deuxieme employe charge (EMP0002)");
    verif(gestion.rechercherParCin("AB12345") == sara, "Connexion possible par CIN");
    verif(gestion.rechercherParMatricule("EMP9999") == nullptr, "Matricule inconnu refuse");
    verif(sara && sara->getIdDepartement() == 1, "Sara est affectee au departement 1");
    verif(youssef && youssef->getIdDepartement() == 2, "Youssef est affecte au departement 2");

    if (sara && youssef) {
        const std::string emailYoussef = youssef->getEmail();
        const std::string telSara = sara->getTelephone();
        try {
            gestion.modifierCoordonnees(sara->getId(), "sara.nouveau@entreprise.ma", "0611111111");
            const Employe* sara2 = gestion.rechercher(sara->getId());
            const Employe* youssef2 = gestion.rechercher(youssef->getId());
            verif(sara2 && sara2->getEmail() == "sara.nouveau@entreprise.ma",
                  "Sara peut modifier son email");
            verif(sara2 && sara2->getTelephone() == "0611111111",
                  "Sara peut modifier son telephone");
            verif(youssef2 && youssef2->getEmail() == emailYoussef,
                  "Le dossier de Youssef n'est pas modifie");
            gestion.modifierCoordonnees(sara->getId(), "sara.alaoui@entreprise.ma", telSara);
        } catch (const std::exception& ex) {
            verif(false, std::string("modifierCoordonnees : ") + ex.what());
        }

        bool refuse = false;
        try {
            gestion.modifierCoordonnees(sara->getId(), "email-invalide", "0612345678");
        } catch (const DonneesInvalides&) {
            refuse = true;
        }
        verif(refuse, "Email invalide refuse");
    }

    std::cout << "Resultat : " << ok << " OK / " << ko << " KO\n";
}

int main(int argc, char* argv[]) {
    try {
        GestionnaireFichiers fichiers("employes.csv", "departements.csv");
        GestionnaireEmployes gestion(fichiers);
        gestion.charger();
        initialiserDemo(gestion, fichiers);

        if (argc > 1 && std::string(argv[1]) == "--test") {
            testsIntegration(gestion);
            return 0;
        }

        while (true) {
            std::cout << "\n========== Gestion des employes ==========\n"
                      << "  1. Espace Administrateur  (membre 3)\n"
                      << "  2. Espace Responsable RH  (membre 2)\n"
                      << "  3. Espace Employe         (membre 4)\n"
                      << "  4. Lancer les tests d'integration\n"
                      << "  0. Quitter\n"
                      << "Choix : ";
            std::string choix;
            std::getline(std::cin, choix);

            if (choix == "0") break;
            if (choix == "1") {
                std::cout << "Espace Administrateur : a integrer par le membre 3.\n";
            } else if (choix == "2") {
                std::cout << "Espace Responsable RH : a integrer par le membre 2.\n";
            } else if (choix == "3") {
                EspaceEmploye espace(gestion, fichiers);
                espace.executer();
            } else if (choix == "4") {
                testsIntegration(gestion);
            } else {
                std::cout << "Choix invalide.\n";
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "Erreur fatale : " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
