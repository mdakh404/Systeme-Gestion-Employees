#include "EspaceEmploye.h"
#include "Exceptions.h"
#include <iostream>
#include <limits>
#include <string>

static std::string lireLigne(const std::string& prompt) {
    std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

static void pause() {
    std::cout << "\nAppuyez sur Entree pour continuer...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static void titre(const std::string& t) {
    std::cout << "\n========== " << t << " ==========\n";
}

EspaceEmploye::EspaceEmploye(GestionnaireEmployes& gestion, MenuDemandes& demandes, MenuConge& conges)
    : gestion_(gestion), demandes_(demandes), conges_(conges) {}

void EspaceEmploye::executer() {
    while (true) {
        titre("Espace Employe");
        std::cout << "  1. Se connecter (matricule ou CIN)\n"
                  << "  0. Retour\n";
        std::string choix = lireLigne("Choix : ");
        if (choix == "0") return;
        if (choix == "1") {
            if (connecter()) {
                menuConnecte();
                deconnecter();
            }
        } else {
            std::cout << "Choix invalide.\n";
        }
    }
}

bool EspaceEmploye::connecter() {
    titre("Connexion");
    std::string identifiant = lireLigne("Matricule ou CIN : ");
    if (identifiant.empty()) {
        std::cout << "Identifiant obligatoire.\n";
        return false;
    }

    const Employe* e = gestion_.rechercherParMatricule(identifiant);
    if (!e) e = gestion_.rechercherParCin(identifiant);
    if (!e) {
        std::cout << "Aucun employe trouve. Acces refuse.\n";
        return false;
    }

    idConnecte_ = e->getId();
    std::cout << "Bienvenue, " << e->getPrenom() << " " << e->getNom()
              << " (" << e->getMatricule() << ").\n";
    return true;
}

void EspaceEmploye::deconnecter() {
    idConnecte_ = 0;
}

const Employe& EspaceEmploye::employeConnecte() const {
    const Employe* e = gestion_.rechercher(idConnecte_);
    if (!e) throw EmployeIntrouvable(idConnecte_);
    return *e;
}

void EspaceEmploye::afficherProfil() const {
    titre("Mon profil");
    afficherInfosPersonnelles();
    afficherInfosProfessionnelles();
    afficherDepartement();
}

void EspaceEmploye::afficherInfosPersonnelles() const {
    const Employe& e = employeConnecte();
    titre("Informations personnelles");
    std::cout << "  Nom            : " << e.getNom() << "\n"
              << "  Prenom         : " << e.getPrenom() << "\n"
              << "  CIN            : " << e.getCin() << "\n"
              << "  Date naissance : " << e.getDateNaissance().toString() << "\n"
              << "  Email          : " << e.getEmail() << "\n"
              << "  Telephone      : " << e.getTelephone() << "\n";
}

void EspaceEmploye::afficherInfosProfessionnelles() const {
    const Employe& e = employeConnecte();
    titre("Informations professionnelles");
    std::cout << "  Matricule      : " << e.getMatricule() << "\n"
              << "  Date embauche  : " << e.getDateEmbauche().toString() << "\n"
              << "  Statut         : " << e.getStatut() << "\n"
              << "  Anciennete     : " << e.anciennete() << " an(s)\n";
}

void EspaceEmploye::afficherDepartement() const {
    const Employe& e = employeConnecte();
    titre("Mon departement");
    if (e.getIdDepartement() == 0) {
        std::cout << "  Vous n'etes affecte a aucun departement.\n";
        return;
    }
    std::cout << "  Identifiant    : " << e.getIdDepartement() << "\n";
}

void EspaceEmploye::modifierCoordonnees() {
    const Employe& actuel = employeConnecte();
    titre("Modifier mes coordonnees");
    std::cout << "  Email actuel     : " << actuel.getEmail() << "\n"
              << "  Telephone actuel : " << actuel.getTelephone() << "\n"
              << "  (Entree seule = conserver la valeur actuelle)\n";

    std::string email = lireLigne("  Nouvel email     : ");
    std::string tel   = lireLigne("  Nouveau telephone: ");
    if (email.empty()) email = actuel.getEmail();
    if (tel.empty())   tel   = actuel.getTelephone();

    try {
        gestion_.modifierCoordonnees(idConnecte_, email, tel);
        std::cout << "Coordonnees mises a jour.\n";
    } catch (const DonneesInvalides& ex) {
        std::cout << ex.what() << "\n";
    } catch (const std::exception& ex) {
        std::cout << "Erreur : " << ex.what() << "\n";
    }
}

void EspaceEmploye::deposerDemande() {
    demandes_.deposerDemande(idConnecte_);
}

void EspaceEmploye::mesDemandes() const {
    demandes_.consulterMesDemandes(idConnecte_);
}

void EspaceEmploye::mesConges() {
    conges_.lancerEmploye(idConnecte_);
}

void EspaceEmploye::menuConnecte() {
    while (idConnecte_ != 0) {
        const Employe& e = employeConnecte();
        titre("Espace Employe - " + e.getPrenom() + " " + e.getNom());
        std::cout << "  1. Consulter mon profil complet\n"
                  << "  2. Consulter mes informations personnelles\n"
                  << "  3. Consulter mes informations professionnelles\n"
                  << "  4. Consulter mon departement\n"
                  << "  5. Modifier mon email et mon telephone\n"
                  << "  6. Deposer une demande administrative\n"
                  << "  7. Consulter mes demandes administratives\n"
                  << "  8. Gestion de mes conges (depot & historique)\n"
                  << "  0. Se deconnecter\n";
        std::string choix = lireLigne("Choix : ");

        try {
            if (choix == "0") return;
            else if (choix == "1") afficherProfil();
            else if (choix == "2") afficherInfosPersonnelles();
            else if (choix == "3") afficherInfosProfessionnelles();
            else if (choix == "4") afficherDepartement();
            else if (choix == "5") modifierCoordonnees();
            else if (choix == "6") deposerDemande();
            else if (choix == "7") mesDemandes();
            else if (choix == "8") mesConges();
            else std::cout << "Choix invalide.\n";
        } catch (const std::exception& ex) {
            std::cout << "Erreur : " << ex.what() << "\n";
            return;
        }
        pause();
    }
}
