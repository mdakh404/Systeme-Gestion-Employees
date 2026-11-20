#pragma once
#include <string>
#include <vector>
#include "Personne.h"
#include "Salaire.h"
#include "Date.h"

// Donnees necessaires a la creation ou a la modification d'un employe.
struct DonneesEmploye {
    std::string nom, prenom, cin, email, telephone;
    Date dateNaissance;
    Date dateEmbauche;
    std::string statut = "Actif";
    double salaireBase = 0.0;
};

class Employe : public Personne {
    std::string matricule_;
    Date dateEmbauche_;
    std::string statut_;
    int idDepartement_ = 0;   // 0 = non affecte (gere par GestionnaireDepartements)
    int idPoste_ = 0;         // 0 = non affecte (gere par le module Postes)
    Salaire salaire_;         // composition
    std::vector<std::string> historique_;

public:
    Employe(int id, const std::string& matricule, const DonneesEmploye& d);

    void afficherInfos() const override;

    // Modification (appelee par GestionnaireEmployes apres validation)
    void modifierInformations(const DonneesEmploye& d);
    void modifierStatut(const std::string& statut);

    // Affectations : utilisees par les gestionnaires Departements / Postes
    void affecterDepartement(int idDepartement);
    void affecterPoste(int idPoste);

    // Anciennete en annees completes (utile pour l'equipe Salaires)
    int anciennete(const Date& reference = Date::aujourdhui()) const;

    // Historique
    void ajouterHistorique(const std::string& evenement);
    const std::vector<std::string>& getHistorique() const { return historique_; }

    // Getters
    const std::string& getMatricule() const { return matricule_; }
    const Date& getDateEmbauche() const { return dateEmbauche_; }
    const std::string& getStatut() const { return statut_; }
    int getIdDepartement() const { return idDepartement_; }
    int getIdPoste() const { return idPoste_; }
    const Salaire& getSalaire() const { return salaire_; }
    Salaire& getSalaire() { return salaire_; }

    // Surcharge d'operateur : deux employes sont egaux s'ils ont le meme id
    bool operator==(const Employe& o) const { return id_ == o.id_; }
};
