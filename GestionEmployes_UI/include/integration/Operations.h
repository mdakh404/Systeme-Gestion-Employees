#pragma once
// ---------------------------------------------------------------------------
// Operations — adaptation entre la saisie console et les regles du back-end.
//
// Aucune regle metier ici : uniquement
//   - conversion de formats (dates JJ/MM/AAAA <-> AAAA-MM-JJ pour les conges),
//   - libelles et listes de valeurs imposees par le back-end,
//   - sauvegarde apres chaque modification, en reproduisant ce que font les
//     menus du back-end (EspaceAdmin, MenuDemandes, MenuAbsences).
// ---------------------------------------------------------------------------
#include <optional>
#include <string>
#include <vector>

struct BackendServices;

namespace integration {

// ---------------- Formats ----------------
// Les conges sont stockes en AAAA-MM-JJ par GestionnaireConges ; l'UI saisit
// et affiche toujours JJ/MM/AAAA.
std::string dateFrVersIso(const std::string& jjmmaaaa);
std::string dateIsoVersFr(const std::string& aaaammjj);

// Valeurs acceptees par le back-end
const std::vector<std::string>& typesConge();      // Annuel, Maladie, Sans solde, Exceptionnel
const std::vector<std::string>& statutsEmploye();  // Actif, Inactif, En conge, Suspendu

// ---------------- Employes ----------------
void sauvegarderEmployes(const BackendServices& s);
// Verifie que le departement existe (0 = retrait) puis affecte et sauvegarde.
void affecterDepartement(const BackendServices& s, int idEmploye, int idDepartement);
std::string nomDepartement(const BackendServices& s, int idDepartement);

// ---------------- Departements ----------------
void sauvegarderDepartements(const BackendServices& s);

// ---------------- Conges ----------------
// Convertit les dates, depose la demande (le back-end sauvegarde lui-meme)
// et renvoie l'identifiant attribue.
int deposerConge(const BackendServices& s, int idEmploye, const std::string& debutFr,
                 const std::string& finFr, const std::string& type);
void traiterConge(const BackendServices& s, int idConge, bool accepter);

// ---------------- Absences ----------------
// Tout ou rien : la modification n'est conservee que si l'ecriture reussit.
// nullopt = doublon (meme employe, meme date).
std::optional<int> enregistrerAbsence(const BackendServices& s, int idEmploye,
                                      const std::string& dateFr, const std::string& motif,
                                      bool justifiee);
bool modifierAbsence(const BackendServices& s, int idAbsence, const std::string& dateFr,
                     const std::string& motif, bool justifiee);
void annulerAbsence(const BackendServices& s, int idAbsence);
bool supprimerAbsence(const BackendServices& s, int idAbsence);

// ---------------- Demandes administratives ----------------
int deposerDemande(const BackendServices& s, int idEmploye, const std::string& type,
                   const std::string& description);
void traiterDemande(const BackendServices& s, int idDemande, bool accepter);

} // namespace integration
