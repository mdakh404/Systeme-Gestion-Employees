#include "AssuranceMaladieService.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>

void AssuranceMaladieService::affilier(int idEmploye, int annee,
                                       int tauxPourcent, Montant plafondAnnuel) {
    if (idEmploye <= 0 || annee < 1 || annee > 9999 ||
        tauxPourcent < 1 || tauxPourcent > 100 || plafondAnnuel <= 0) {
        throw std::invalid_argument("Parametres d'affiliation invalides.");
    }
    const CleCouverture cle{idEmploye, annee};
    const Couverture couverture{tauxPourcent, plafondAnnuel};
    if (!donnees.couvertures.emplace(cle, couverture).second) {
        throw std::invalid_argument("Employe deja affilie pour cette annee.");
    }
}
void AssuranceMaladieService::deposer(int idDossier, int idEmploye,
                                      int anneeSoins, Montant frais) {
    if (idDossier <= 0 || frais <= 0) {
        throw std::invalid_argument("Identifiant ou montant des frais invalide.");
    }
    if (donnees.couvertures.count({idEmploye, anneeSoins}) == 0) {
        throw std::invalid_argument("Aucune couverture pour l'annee des soins.");
    }
    const Demande demande{idDossier, idEmploye, anneeSoins, frais, 0, Statut::EnAttente};
    if (!donnees.demandes.emplace(idDossier, demande).second) {
        throw std::invalid_argument("Ce numero de dossier existe deja.");
    }
}
AssuranceMaladieService::Montant
AssuranceMaladieService::plafondRestant(int idEmploye, int annee) const {
    const auto couverture = donnees.couvertures.find({idEmploye, annee});
    if (couverture == donnees.couvertures.end()) {
        throw std::invalid_argument("Affiliation introuvable.");
    }
    Montant restant = couverture->second.plafond;
    for (const auto& ligne : donnees.demandes) {
        const Demande& demande = ligne.second;
        if (demande.idEmploye == idEmploye && demande.anneeSoins == annee) {
            restant -= demande.remboursement;
        }
    }
    return restant;
}
AssuranceMaladieService::Montant
AssuranceMaladieService::traiter(int idDossier, bool accepter) {
    const auto position = donnees.demandes.find(idDossier);
    if (position == donnees.demandes.end()) {
        throw std::invalid_argument("Dossier introuvable.");
    }
    Demande& demande = position->second;
    if (demande.statut != Statut::EnAttente) {
        throw std::logic_error("Ce dossier a deja ete traite.");
    }
    Montant montant = 0;
    if (accepter) {
        const auto& couverture =
            donnees.couvertures.at({demande.idEmploye, demande.anneeSoins});
        const Montant theorique =
            (demande.frais / 100) * couverture.taux +
            ((demande.frais % 100) * couverture.taux + 50) / 100;
        montant = std::min(theorique,
                           plafondRestant(demande.idEmploye, demande.anneeSoins));
    }
    donnees.ordreDecisions.push_back(idDossier);
    demande.statut = accepter ? Statut::Acceptee : Statut::Refusee;
    demande.remboursement = montant;
    return montant;
}
std::vector<AssuranceMaladieService::Demande>
AssuranceMaladieService::historique(int idEmploye) const {
    std::vector<Demande> resultat;
    for (const auto& ligne : donnees.demandes) {
        if (ligne.second.idEmploye == idEmploye) resultat.push_back(ligne.second);
    }
    return resultat;
}
std::vector<AssuranceMaladieService::Demande>
AssuranceMaladieService::demandesEnAttente() const {
    std::vector<Demande> resultat;
    for (const auto& ligne : donnees.demandes) {
        if (ligne.second.statut == Statut::EnAttente) resultat.push_back(ligne.second);
    }
    return resultat;
}
void AssuranceMaladieService::sauvegarder(std::ostream& sortie) const {
    sortie << "ASSURANCE_MALADIE_V1\n";
    for (const auto& ligne : donnees.couvertures) {
        sortie << "AFFILIATION " << ligne.first.first << ' '
               << ligne.first.second << ' ' << ligne.second.taux << ' '
               << ligne.second.plafond << '\n';
    }
    for (const auto& ligne : donnees.demandes) {
        const Demande& d = ligne.second;
        sortie << "DOSSIER " << d.id << ' ' << d.idEmploye << ' '
               << d.anneeSoins << ' ' << d.frais << '\n';
    }
    for (int id : donnees.ordreDecisions) {
        const bool accepte = donnees.demandes.at(id).statut == Statut::Acceptee;
        sortie << "DECISION " << id << ' ' << (accepte ? 1 : 0) << '\n';
    }
    sortie << "FIN\n";
    sortie.flush();
    if (!sortie) throw std::runtime_error("Echec de la sauvegarde de l'assurance.");
}
void AssuranceMaladieService::charger(std::istream& entree) {
    std::string type;
    if (!(entree >> type) || type != "ASSURANCE_MALADIE_V1") {
        throw std::runtime_error("Format du fichier d'assurance invalide.");
    }
    Donnees precedent;
    std::swap(precedent, donnees);
    try {
        while (entree >> type) {
            if (type == "FIN") {
                if (entree >> type || !entree.eof() || entree.bad()) {
                    throw std::runtime_error("Donnees inattendues apres FIN.");
                }
                return;
            }
            if (type == "AFFILIATION") {
                int employe, annee, taux; Montant plafond;
                if (!(entree >> employe >> annee >> taux >> plafond)) throw std::runtime_error("Affiliation incomplete.");
                affilier(employe, annee, taux, plafond);
            } else if (type == "DOSSIER") {
                int dossier, employe, annee; Montant frais;
                if (!(entree >> dossier >> employe >> annee >> frais)) throw std::runtime_error("Dossier incomplet.");
                deposer(dossier, employe, annee, frais);
            } else if (type == "DECISION") {
                int dossier, decision;
                if (!(entree >> dossier >> decision) || (decision != 0 && decision != 1)) throw std::runtime_error("Decision invalide.");
                traiter(dossier, decision == 1);
            } else {
                throw std::runtime_error("Enregistrement inconnu.");
            }
        }
        throw std::runtime_error("Fichier incomplet : marqueur FIN absent.");
    } catch (...) {
        std::swap(precedent, donnees);
        throw;
    }
}

// ======================================================================
//  Interface console (display / execute)
// ======================================================================
namespace {

std::string lireLigne(const std::string& invite) {
    std::cout << invite;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

int lireEntier(const std::string& invite) {
    const std::string s = lireLigne(invite);
    try {
        std::size_t pos = 0;
        const int v = std::stoi(s, &pos);
        if (pos != s.size()) throw std::invalid_argument("x");
        return v;
    } catch (const std::exception&) {
        throw std::invalid_argument("Saisie invalide : un nombre entier est attendu.");
    }
}

// Saisie en MAD (ex : 250.50) -> centimes
AssuranceMaladieService::Montant lireMontantMAD(const std::string& invite) {
    std::string s = lireLigne(invite);
    std::replace(s.begin(), s.end(), ',', '.');
    try {
        std::size_t pos = 0;
        const double v = std::stod(s, &pos);
        if (pos != s.size() || !std::isfinite(v) || v <= 0) throw std::invalid_argument("x");
        return static_cast<AssuranceMaladieService::Montant>(std::llround(v * 100.0));
    } catch (const std::exception&) {
        throw std::invalid_argument("Montant invalide (nombre positif attendu).");
    }
}

std::string formaterMAD(AssuranceMaladieService::Montant c) {
    std::ostringstream os;
    os << (c / 100) << '.' << std::setw(2) << std::setfill('0') << (c % 100) << " MAD";
    return os.str();
}

const char* texteStatut(AssuranceMaladieService::Statut s) {
    switch (s) {
        case AssuranceMaladieService::Statut::EnAttente: return "En attente";
        case AssuranceMaladieService::Statut::Acceptee:  return "Acceptee";
        default:                                         return "Refusee";
    }
}

void afficherDemande(const AssuranceMaladieService::Demande& d) {
    std::cout << "  Dossier #" << d.id << " | employe " << d.idEmploye
              << " | annee " << d.anneeSoins
              << " | frais " << formaterMAD(d.frais)
              << " | rembourse " << formaterMAD(d.remboursement)
              << " | " << texteStatut(d.statut) << '\n';
}

} // namespace

void AssuranceMaladieService::display() const {
    std::cout << "Service #" << id << " - " << name << '\n'
              << "  Description : " << description << '\n'
              << "  Affiliations : " << donnees.couvertures.size()
              << " | Dossiers : " << donnees.demandes.size() << '\n';
}

void AssuranceMaladieService::execute() {
    while (true) {
        std::cout << "\n--- " << name << " ---\n"
                  << "  1. Affilier un employe\n"
                  << "  2. Deposer un dossier de remboursement\n"
                  << "  3. Traiter un dossier en attente\n"
                  << "  4. Consulter le plafond restant\n"
                  << "  5. Historique d'un employe\n"
                  << "  6. Dossiers en attente\n"
                  << "  0. Retour\n";
        const std::string choix = lireLigne("Choix : ");
        if (choix == "0") return;

        try {
            if (choix == "1") {
                const int idEmp = lireEntier("Id de l'employe : ");
                if (verificateur_ && !verificateur_(idEmp))
                    throw std::invalid_argument("Employe introuvable.");
                const int annee = lireEntier("Annee de couverture : ");
                const int taux  = lireEntier("Taux de remboursement (1-100 %) : ");
                const Montant plafond = lireMontantMAD("Plafond annuel (MAD) : ");
                affilier(idEmp, annee, taux, plafond);
                std::cout << "Employe affilie.\n";
            } else if (choix == "2") {
                const int idEmp = lireEntier("Id de l'employe : ");
                const int annee = lireEntier("Annee des soins : ");
                const Montant frais = lireMontantMAD("Frais medicaux (MAD) : ");
                int idDossier = 1;
                if (!donnees.demandes.empty()) idDossier = donnees.demandes.rbegin()->first + 1;
                deposer(idDossier, idEmp, annee, frais);
                std::cout << "Dossier #" << idDossier << " depose (en attente).\n";
            } else if (choix == "3") {
                const auto attente = demandesEnAttente();
                if (attente.empty()) { std::cout << "Aucun dossier en attente.\n"; continue; }
                for (const auto& d : attente) afficherDemande(d);
                const int idDossier = lireEntier("Id du dossier a traiter : ");
                const std::string rep = lireLigne("Accepter ? (o/n) : ");
                if (rep != "o" && rep != "O" && rep != "n" && rep != "N")
                    throw std::invalid_argument("Reponse attendue : o ou n.");
                const Montant m = traiter(idDossier, rep == "o" || rep == "O");
                if (rep == "o" || rep == "O") std::cout << "Dossier accepte. Remboursement : " << formaterMAD(m) << '\n';
                else std::cout << "Dossier refuse.\n";
            } else if (choix == "4") {
                const int idEmp = lireEntier("Id de l'employe : ");
                const int annee = lireEntier("Annee : ");
                std::cout << "Plafond restant : " << formaterMAD(plafondRestant(idEmp, annee)) << '\n';
            } else if (choix == "5") {
                const int idEmp = lireEntier("Id de l'employe : ");
                const auto h = historique(idEmp);
                if (h.empty()) std::cout << "Aucun dossier pour cet employe.\n";
                for (const auto& d : h) afficherDemande(d);
            } else if (choix == "6") {
                const auto attente = demandesEnAttente();
                if (attente.empty()) std::cout << "Aucun dossier en attente.\n";
                for (const auto& d : attente) afficherDemande(d);
            } else {
                std::cout << "Choix invalide.\n";
            }
        } catch (const std::exception& ex) {
            std::cout << "Erreur : " << ex.what() << '\n';
        }
    }
}