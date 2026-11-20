#ifndef GESTIONNAIRE_FICHIERS_CONGE_H
#define GESTIONNAIRE_FICHIERS_CONGE_H

#include <string>
#include <vector>
#include "Conge.h"

class GestionnaireFichiersConge {
private:
    std::string cheminFichier;
    char separateur;

    // Méthodes utilitaires internes
    void assurerExistenceDossierEtFichier() const;
    Conge deserialiserLigne(const std::string& ligne) const;
    std::string serialiserConge(const Conge& conge) const;

public:
    // Constructeur avec chemin par défaut
    explicit GestionnaireFichiersConge(
        const std::string& chemin = "data/conges.txt",
        char sep = ';'
    );

    // Lecture et sauvegarde complètes
    std::vector<Conge> chargerTousLesConges() const;
    void sauvegarderTousLesConges(const std::vector<Conge>& conges) const;

    // Opérations ciblées (utiles pour Ibtissam et Chaima)
    void ajouterConge(const Conge& nouveauConge) const;
    bool mettreAJourStatut(int idConge, const std::string& nouveauStatut) const;
    std::vector<Conge> rechercherParEmploye(int idEmploye) const;
};

#endif // GESTIONNAIRE_FICHIERS_CONGE_H
