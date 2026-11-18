#pragma once
#include <string>
#include "IPersistanceEmployes.h"
#include "IPersistanceDepartements.h"

// Persistance dans des fichiers texte (CSV, separateur ';').
//
// employes.csv : une ligne = un employe
//   id;nom;prenom;cin;naissance;email;tel;matricule;embauche;statut;idDept;idPoste;salaire;historique(|)
// departements.csv : une ligne = un departement
//   id;nom;description
class GestionnaireFichiers : public IPersistanceEmployes, public IPersistanceDepartements {
    std::string cheminEmployes_;
    std::string cheminDepartements_;
public:
    explicit GestionnaireFichiers(const std::string& cheminEmployes = "employes.csv",
                                  const std::string& cheminDepartements = "departements.csv")
        : cheminEmployes_(cheminEmployes), cheminDepartements_(cheminDepartements) {}

    // fichier absent => liste vide (premier lancement)
    std::vector<Employe> lireEmployes() override;
    void sauvegarderEmployes(const std::vector<Employe>& employes) override;

    std::vector<Departement> lireDepartements() override;
    void sauvegarderDepartements(const std::vector<Departement>& departements) override;
};
