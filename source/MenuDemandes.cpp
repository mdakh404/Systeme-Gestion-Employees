#include "MenuDemandes.h"
#include "ExceptionsDemandes.h"
#include <iostream>
#include <limits>

MenuDemandes::MenuDemandes(GestionnaireDemandes& gestion,
                           GestionnaireFichiersDemandes& fichiers)
    : gestion_(gestion), fichiers_(fichiers) {}

void MenuDemandes::sauvegarder() const {
    fichiers_.ecrireDemandes(gestion_.lister());
}

std::string MenuDemandes::lireLigne(const std::string& prompt) {
    std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

void MenuDemandes::afficherListe(const std::vector<DemandeAdministrative>& demandes,
                                 const std::string& titre) const {
    std::cout << "\n========== " << titre << " ==========\n";

    if (demandes.empty()) {
        std::cout << "Aucune demande.\n";
        return;
    }

    std::cout << "Nombre de demandes : " << demandes.size() << "\n";
    for (const auto& d : demandes) {
        std::cout << "\n---------- DEMANDE ----------\n";
        d.afficherInfos();
    }
    std::cout << "\n================================\n";
}

// ==========================================
// ESPACE EMPLOYE
// ==========================================

void MenuDemandes::deposerDemande(int idEmploye) {
    std::cout << "\n========== DEPOSER UNE DEMANDE ADMINISTRATIVE ==========\n";

    std::cout << "Types proposes :\n";
    auto types = DemandeAdministrative::typesProposes();
    for (size_t i = 0; i < types.size(); ++i)
        std::cout << "  " << i + 1 << ". " << types[i] << "\n";

    std::string choix = lireLigne("Choix : ");
    std::string type;

    if (choix.size() == 1 && choix[0] >= '1' &&
        choix[0] <= static_cast<char>('0' + types.size())) {
        type = types[static_cast<size_t>(choix[0] - '1')];
    } else {
        std::cout << "Choix invalide.\n";
        return;
    }

    if (type == "Autre") {
        type = lireLigne("Precisez le type de demande : ");
    }

    std::string description = lireLigne("Description : ");

    DonneesDemande donnees;
    donnees.idEmploye = idEmploye;
    donnees.type = type;
    donnees.description = description;
    donnees.dateDepot = Date::aujourdhui();

    try {
        int id = gestion_.creerDemande(donnees);
        sauvegarder();

        std::cout << "\nDemande enregistree (id = " << id << ").\n";
        std::cout << "Statut : " << DemandeAdministrative::STATUT_EN_ATTENTE << "\n";
    } catch (const std::exception& ex) {
        std::cout << "\n" << ex.what() << "\n";
    }
}

void MenuDemandes::consulterMesDemandes(int idEmploye) const {
    afficherListe(gestion_.listerParEmploye(idEmploye), "MES DEMANDES");
}

// ==========================================
// ESPACE ADMINISTRATEUR / RESPONSABLE RH
// ==========================================

void MenuDemandes::consulterDemandes() const {
    std::cout << "\n1. Toutes les demandes\n"
              << "2. Demandes en attente\n"
              << "Choix : ";

    std::string choix = lireLigne("");

    if (choix == "1")
        afficherListe(gestion_.lister(), "TOUTES LES DEMANDES");
    else if (choix == "2")
        afficherListe(gestion_.listerEnAttente(), "DEMANDES EN ATTENTE");
    else
        std::cout << "Choix invalide.\n";
}

void MenuDemandes::traiterDemande() {
    std::cout << "\n========== TRAITER UNE DEMANDE ==========\n";

    afficherListe(gestion_.listerEnAttente(), "DEMANDES EN ATTENTE");

    if (gestion_.listerEnAttente().empty()) return;

    std::string idSaisi = lireLigne("Identifiant de la demande : ");
    std::string decision = lireLigne("Decision (A = Accepter / R = Refuser) : ");

    int id = 0;
    try {
        id = std::stoi(idSaisi);
    } catch (const std::exception&) {
        std::cout << "Identifiant invalide.\n";
        return;
    }

    std::string statut;
    if (decision == "A" || decision == "a")
        statut = DemandeAdministrative::STATUT_ACCEPTEE;
    else if (decision == "R" || decision == "r")
        statut = DemandeAdministrative::STATUT_REFUSEE;
    else {
        std::cout << "Decision invalide.\n";
        return;
    }

    try {
        gestion_.traiterDemande(id, statut);
        sauvegarder();

        std::cout << "\nDemande " << id << " : " << statut << ".\n";
    } catch (const std::exception& ex) {
        std::cout << "\n" << ex.what() << "\n";
    }
}
