#include "Date.h"
#include "GestionnaireDepartements.h"
#include "GestionnaireEmployes.h"

#include <exception>
#include <iostream>
#include <string>

int main() {
    try {
        bool toutEstCorrect = true;
        const auto verifier = [&toutEstCorrect](bool condition, const std::string& action) {
            std::cout << (condition ? "[OK] " : "[ECHEC] ") << action << '\n';
            toutEstCorrect = toutEstCorrect && condition;
        };

        const Date date = Date::fromString("22/11/2026");
        verifier(date.estValide() && date.toString() == "22/11/2026",
                 "lecture et formatage d'une date");

        GestionnaireDepartements gestionnaireDepartements;
        const int idDepartement =
            gestionnaireDepartements.ajouter("Informatique", "Equipe technique");
        verifier(gestionnaireDepartements.contient(idDepartement) &&
                     gestionnaireDepartements.nombre() == 1 &&
                     gestionnaireDepartements.trouver(idDepartement).getNom() == "Informatique",
                 "ajout et consultation d'un departement");

        gestionnaireDepartements.modifier(
            idDepartement, "Technologie", "Equipe des systemes");
        verifier(gestionnaireDepartements.rechercher(idDepartement)->getNom() == "Technologie" &&
                     gestionnaireDepartements.lister().size() == 1,
                 "modification et liste des departements");

        GestionnaireEmployes gestionnaireEmployes(gestionnaireDepartements);
        DonneesEmploye donnees;
        donnees.nom = "Alaoui";
        donnees.prenom = "Sara";
        donnees.cin = "AB12345";
        donnees.email = "sara@example.com";
        donnees.telephone = "0612345678";
        donnees.dateNaissance = Date(15, 3, 1995);
        donnees.dateEmbauche = Date(1, 9, 2020);
        donnees.statut = "Actif";
        donnees.salaireBase = 8500.0;

        const int idEmploye = gestionnaireEmployes.ajouter(donnees);
        verifier(gestionnaireEmployes.nombre() == 1 &&
                     gestionnaireEmployes.rechercher(idEmploye).has_value(),
                 "ajout et recherche d'un employe par identifiant");
        verifier(gestionnaireEmployes.rechercherParCin("AB12345").has_value() &&
                     gestionnaireEmployes.rechercherParMatricule("EMP0001").has_value() &&
                     gestionnaireEmployes.rechercherParNom("ala").size() == 1,
                 "recherche d'un employe par CIN, matricule et nom");

        gestionnaireEmployes.affecterDepartement(idEmploye, idDepartement);
        gestionnaireEmployes.modifierStatut(idEmploye, "En conge");
        gestionnaireEmployes.modifierCoordonnees(
            idEmploye, "sara.alaoui@example.com", "0698765432");
        verifier(gestionnaireEmployes.trouver(idEmploye).getStatut() == "En conge" &&
                     gestionnaireEmployes.effectifDepartement(idDepartement) == 1 &&
                     gestionnaireEmployes.employesDuDepartement(idDepartement).size() == 1,
                 "affectation, statut et consultation par departement");

        donnees.statut = "Actif";
        donnees.email = "sara.modifiee@example.com";
        donnees.salaireBase = 9000.0;
        gestionnaireEmployes.modifier(idEmploye, donnees);
        verifier(gestionnaireEmployes.lister().size() == 1 &&
                     gestionnaireEmployes.historique(idEmploye).size() > 1 &&
                     gestionnaireEmployes.trouver(idEmploye)
                             .getSalaire()
                             .getSalaireBase() == 9000.0,
                 "modification complete, salaire et historique");

        gestionnaireEmployes.trouver(idEmploye).afficherInfos();
        gestionnaireDepartements.trouver(idDepartement).afficherInfos();

        gestionnaireEmployes.affecterDepartement(idEmploye, 0);
        gestionnaireEmployes.supprimer(idEmploye);
        gestionnaireDepartements.supprimer(idDepartement, gestionnaireEmployes);
        verifier(gestionnaireEmployes.nombre() == 0 &&
                     gestionnaireDepartements.nombre() == 0,
                 "suppression d'un employe et d'un departement inutilise");

        if (!toutEstCorrect) {
            std::cerr << "Au moins une verification a echoue.\n";
            return 1;
        }

        std::cout << "Toutes les verifications sont reussies.\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Erreur pendant la verification : " << ex.what() << '\n';
        return 1;
    }
}
