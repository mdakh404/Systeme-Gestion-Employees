#include "Employe.h"
#include <iostream>

Employe::Employe(int id, const std::string& matricule, const DonneesEmploye& d)
    : Personne(id, d.nom, d.prenom, d.cin, d.dateNaissance, d.email, d.telephone),
      matricule_(matricule), dateEmbauche_(d.dateEmbauche), statut_(d.statut),
      salaire_(d.salaireBase) {}

void Employe::afficherInfos() const {
    Personne::afficherInfos();
    std::cout << "     Matricule: " << matricule_
              << " | Embauche: " << dateEmbauche_.toString()
              << " | Statut: " << statut_
              << " | Departement: " << (idDepartement_ ? std::to_string(idDepartement_) : "-")
              << " | Poste: " << (idPoste_ ? std::to_string(idPoste_) : "-")
              << " | Salaire de base: " << salaire_.getSalaireBase() << "\n";
}

void Employe::modifierInformations(const DonneesEmploye& d) {
    setNom(d.nom);
    setPrenom(d.prenom);
    setCin(d.cin);
    setEmail(d.email);
    setTelephone(d.telephone);
    setDateNaissance(d.dateNaissance);
    dateEmbauche_ = d.dateEmbauche;
    statut_ = d.statut;
    salaire_.setSalaireBase(d.salaireBase);
    ajouterHistorique("Modification des informations");
}

void Employe::affecterDepartement(int idDepartement) {
    idDepartement_ = idDepartement;
    ajouterHistorique(idDepartement == 0 ? "Retrait du departement"
                                         : "Affectation au departement " + std::to_string(idDepartement));
}

void Employe::affecterPoste(int idPoste) {
    idPoste_ = idPoste;
    ajouterHistorique("Affectation au poste " + std::to_string(idPoste));
}

int Employe::anciennete(const Date& reference) const {
    return Date::anneesEntre(dateEmbauche_, reference);
}

void Employe::ajouterHistorique(const std::string& evenement) {
    historique_.push_back(Date::aujourdhui().toString() + " - " + evenement);
}
