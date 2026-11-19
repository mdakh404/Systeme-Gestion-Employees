#pragma once
#include <map>
#include <string>
#include <vector>
#include "Employe.h"
#include "Exceptions.h"
#include "IPersistanceEmployes.h"

// Couche logique metier : operations CRUD sur les employes.
// Utilise std::map (recherche par id en O(log n)) et l'interface de persistance.
class GestionnaireEmployes {
    std::map<int, Employe> employes_;
    IPersistanceEmployes& persistance_;
    int prochainId_ = 1;

    static constexpr int AGE_MIN_EMBAUCHE = 18;   // regle metier modifiable

    void valider(const DonneesEmploye& d) const;
    void verifierCinUnique(const std::string& cin, int idIgnore = 0) const;
    void sauvegarder();
    static std::string fabriquerMatricule(int id);

public:
    explicit GestionnaireEmployes(IPersistanceEmployes& persistance);

    void charger();                                       // lit le fichier

    int ajouter(const DonneesEmploye& d);                 // retourne l'id cree
    void modifier(int id, const DonneesEmploye& d);       // throws EmployeIntrouvable
    void supprimer(int id);                               // throws EmployeIntrouvable

    const Employe* rechercher(int id) const;              // nullptr si absent
    Employe& trouver(int id);                             // throws EmployeIntrouvable
    std::vector<const Employe*> rechercherParNom(const std::string& motif) const;

    std::vector<Employe> lister() const;                  // tries par id
    const std::vector<std::string>& historique(int id) const; // throws EmployeIntrouvable
    std::size_t nombre() const { return employes_.size(); }

    // ---- Relation Employe -> Departement (0 = non affecte) ----
    // L'appelant (gestion des departements) verifie que le departement existe.
    void affecterDepartement(int idEmploye, int idDepartement);  // throws EmployeIntrouvable
    std::vector<const Employe*> employesDuDepartement(int idDepartement) const;
    std::size_t effectifDepartement(int idDepartement) const;    // pour "supprimer si vide"

    // Pour les autres modules (Departements / Postes) : sauvegarde apres affectation
    void enregistrerModifications() { sauvegarder(); }
};
