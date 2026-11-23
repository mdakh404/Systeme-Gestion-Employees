#include "VueGestionEmployes.h"
#include "../01_Common/Navigation.h"
#include "../01_Common/ConsoleUI.h"
#include "../01_Common/InputValidator.h"
#include "../../include/BackendTypes.h"
#include <string>
#include <vector>

static DonneesEmploye saisirDonneesEmploye() {
    DonneesEmploye d;
    d.nom = InputValidator::lireChaine("Nom : ");
    d.prenom = InputValidator::lireChaine("Prenom : ");
    d.cin = InputValidator::lireChaine("CIN : ");
    d.email = InputValidator::lireChaine("Email : ");
    d.telephone = InputValidator::lireChaine("Telephone : ");
    d.dateNaissance = Date::fromString(InputValidator::lireDate("Date naissance (JJ/MM/AAAA) : "));
    d.dateEmbauche = Date::fromString(InputValidator::lireDate("Date embauche (JJ/MM/AAAA) : "));
    d.salaireBase = InputValidator::lireDouble("Salaire de base : ");
    d.statut = "Actif";
    return d;
}

static void afficherListeEmployes(const BackendServices& services) {
    const auto liste = services.employes->lister();
    std::vector<std::string> enTetes = {"ID", "Matricule", "Nom", "Prenom", "Statut", "Departement", "Salaire"};
    std::vector<std::vector<std::string>> lignes;
    for (const auto& e : liste) {
        lignes.push_back({
            std::to_string(e.getId()),
            e.getMatricule(),
            e.getNom(),
            e.getPrenom(),
            e.getStatut(),
            integration::nomDepartement(services, e.getIdDepartement()),
            std::to_string(static_cast<int>(e.getSalaire().getSalaireBase()))
        });
    }
    ConsoleUI::afficherTableau(enTetes, lignes);
}

void VueGestionEmployes::afficher(Navigation& navigation) {
    auto* ge = navigation.services().employes;
    if (!ge) {
        ConsoleUI::afficherErreur("Service employes non initialise.");
        ConsoleUI::pause();
        return;
    }

    while (true) {
        ConsoleUI::effacerEcran();
        ConsoleUI::afficherTitre("GESTION DES EMPLOYES");
        ConsoleUI::afficherInfo("1. Lister les employes");
        ConsoleUI::afficherInfo("2. Ajouter un employe");
        ConsoleUI::afficherInfo("3. Modifier un employe");
        ConsoleUI::afficherInfo("4. Supprimer un employe");
        ConsoleUI::afficherInfo("5. Rechercher (ID / CIN / matricule / nom)");
        ConsoleUI::afficherInfo("6. Changer le statut");
        ConsoleUI::afficherInfo("7. Affecter a un departement");
        ConsoleUI::afficherInfo("0. Retour");

        const int choix = InputValidator::lireChoixMenu("Choix : ", 0, 7);
        if (choix == 0) break;

        try {
            if (choix == 1) {
                ConsoleUI::afficherSousTitre("Liste des employes");
                afficherListeEmployes(navigation.services());
                ConsoleUI::pause();
            } else if (choix == 2) {
                ConsoleUI::afficherSousTitre("Nouvel employe");
                DonneesEmploye d = saisirDonneesEmploye();
                int id = ge->ajouter(d);
                integration::sauvegarderEmployes(navigation.services());
                ConsoleUI::afficherSucces("Employe ajoute. ID = " + std::to_string(id));
                ConsoleUI::pause();
            } else if (choix == 3) {
                ConsoleUI::afficherSousTitre("Modifier un employe");
                int id = InputValidator::lireEntier("ID employe : ");
                const Employe* existant = ge->rechercher(id);
                if (!existant) throw EmployeIntrouvable(id);
                DonneesEmploye d = saisirDonneesEmploye();
                d.statut = existant->getStatut(); // le statut se change via l'option 6
                ge->modifier(id, d);
                integration::sauvegarderEmployes(navigation.services());
                ConsoleUI::afficherSucces("Employe modifie.");
                ConsoleUI::pause();
            } else if (choix == 4) {
                ConsoleUI::afficherSousTitre("Supprimer un employe");
                int id = InputValidator::lireEntier("ID a supprimer : ");
                if (InputValidator::lireConfirmation("Confirmer la suppression")) {
                    ge->supprimer(id);
                    integration::sauvegarderEmployes(navigation.services());
                    ConsoleUI::afficherSucces("Employe supprime.");
                }
                ConsoleUI::pause();
            } else if (choix == 5) {
                ConsoleUI::afficherSousTitre("Recherche");
                ConsoleUI::afficherInfo("1. Par ID  2. Par CIN  3. Par matricule  4. Par nom");
                int mode = InputValidator::lireChoixMenu("Mode : ", 1, 4);
                if (mode == 1) {
                    int id = InputValidator::lireEntier("ID : ");
                    auto o = ge->rechercher(id);
                    if (o) {
                        ConsoleUI::afficherInfo(o->getMatricule() + " | " + o->getNom() + " " + o->getPrenom()
                            + " | " + o->getStatut());
                    } else ConsoleUI::afficherErreur("Introuvable.");
                } else if (mode == 2) {
                    auto o = ge->rechercherParCin(InputValidator::lireChaine("CIN : "));
                    if (o) ConsoleUI::afficherInfo(std::to_string(o->getId()) + " | " + o->getMatricule() + " | " + o->getNom());
                    else ConsoleUI::afficherErreur("Introuvable.");
                } else if (mode == 3) {
                    auto o = ge->rechercherParMatricule(InputValidator::lireChaine("Matricule : "));
                    if (o) ConsoleUI::afficherInfo(std::to_string(o->getId()) + " | " + o->getNom());
                    else ConsoleUI::afficherErreur("Introuvable.");
                } else {
                    auto liste = ge->rechercherParNom(InputValidator::lireChaine("Motif nom : "));
                    for (const auto& e : liste)
                        ConsoleUI::afficherInfo(std::to_string(e->getId()) + " | " + e->getMatricule()
                            + " | " + e->getNom() + " " + e->getPrenom());
                    if (liste.empty()) ConsoleUI::afficherErreur("Aucun resultat.");
                }
                ConsoleUI::pause();
            } else if (choix == 6) {
                int id = InputValidator::lireEntier("ID employe : ");
                std::string st = InputValidator::lireChoixListe("Nouveau statut :", integration::statutsEmploye());
                ge->modifierStatut(id, st);
                integration::sauvegarderEmployes(navigation.services());
                ConsoleUI::afficherSucces("Statut mis a jour.");
                ConsoleUI::pause();
            } else if (choix == 7) {
                int idEmp = InputValidator::lireEntier("ID employe : ");
                int idDep = InputValidator::lireEntier("ID departement (0 = retirer) : ");
                integration::affecterDepartement(navigation.services(), idEmp, idDep);
                ConsoleUI::afficherSucces("Affectation effectuee.");
                ConsoleUI::pause();
            }
        } catch (const std::exception& e) {
            ConsoleUI::afficherErreur(e.what());
            ConsoleUI::pause();
        }
    }
}
