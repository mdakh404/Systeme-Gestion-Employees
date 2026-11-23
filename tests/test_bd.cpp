// Smoke test de la persistance MySQL.
//   - se connecte, ajoute un employe + un conge + une demande + une absence,
//   - "redemarre" (recharge tout depuis la base) et verifie que les donnees sont revenues.
// Code de retour 0 = OK. Voir docs/README_BD.md pour la compilation.
#include <ctime>
#include <iostream>
#include <string>

#include "Database.h"
#include "GestionnaireAbsences.h"
#include "GestionnaireConges.h"
#include "GestionnaireDemandes.h"
#include "GestionnaireEmployes.h"
#include "GestionnaireFichiersAbsences.h"
#include "GestionnaireFichiersDemandes.h"

namespace {
int echecs = 0;
void verifier(bool condition, const std::string& message) {
    std::cout << (condition ? "[OK]    " : "[ECHEC] ") << message << "\n";
    if (!condition) ++echecs;
}
} // namespace

int main() {
    if (!Database::instance().connecter()) {
        std::cout << "Connexion impossible : " << Database::instance().derniereErreur() << "\n"
                  << "(compiler avec -DUSE_MYSQL et renseigner data/db.cfg)\n";
        return 2;
    }
    std::cout << "Connecte : " << Database::instance().description() << "\n";

    try {
        const std::string suffixe = std::to_string(std::time(nullptr) % 1000000);

        // ---- Employe ----
        GestionnaireEmployes employes;
        employes.chargerDepuisFichier();
        DonneesEmploye d;
        d.nom = "Testeur";
        d.prenom = "Bd";
        d.cin = "T" + suffixe;
        d.email = "test" + suffixe + "@company.ma";
        d.telephone = "0612345678";
        d.dateNaissance = Date(15, 5, 1990);
        d.dateEmbauche = Date(1, 9, 2018);
        d.salaireBase = 8500.0;
        int id = employes.ajouter(d);
        employes.sauvegarderDansFichier();

        GestionnaireEmployes relu;
        relu.chargerDepuisFichier();
        const Employe* e = relu.rechercher(id);
        verifier(e != nullptr, "employe retrouve apres rechargement (id " + std::to_string(id) + ")");
        if (e) {
            verifier(e->getNom() == "Testeur" && e->getCin() == d.cin, "nom et CIN identiques");
            verifier(e->getDateEmbauche() == Date(1, 9, 2018), "date d'embauche identique");
            verifier(e->getSalaire().getSalaireBase() == 8500.0, "salaire de base identique");
            verifier(e->getStatut() == "Actif", "statut Actif");
        }

        // ---- Conge ----
        GestionnaireConges conges;   // charge depuis la base
        conges.deposerDemande(id, "2027-04-01", "2027-04-05", "Annuel");   // sauvegarde auto
        GestionnaireConges conges2;
        bool congeTrouve = false;
        for (const auto& c : conges2.tousLesConges())
            if (c.GetidEmploye() == id && c.GetdateDebut() == "2027-04-01") congeTrouve = true;
        verifier(congeTrouve, "conge retrouve apres rechargement");

        // ---- Demande administrative ----
        GestionnaireDemandes demandes;
        GestionnaireFichiersDemandes fichiersDemandes;
        demandes.charger(fichiersDemandes.lireDemandes());
        DonneesDemande dd;
        dd.idEmploye = id;
        dd.type = "Attestation de travail";
        dd.description = "Pour dossier bancaire";
        dd.dateDepot = Date::aujourdhui();
        int idDemande = demandes.creerDemande(dd);
        fichiersDemandes.ecrireDemandes(demandes.lister());
        GestionnaireDemandes demandes2;
        demandes2.charger(fichiersDemandes.lireDemandes());
        const DemandeAdministrative* dem = demandes2.rechercher(idDemande);
        verifier(dem != nullptr && dem->getType() == "Attestation de travail"
                     && dem->getDescription() == "Pour dossier bancaire" && dem->estEnAttente(),
                 "demande retrouvee (type, description, statut En attente)");

        // ---- Absence ----
        GestionnaireFichiersAbsences fichiersAbsences;
        GestionnaireAbsences absences = fichiersAbsences.chargerAbsences(
            [&relu](int i) { return relu.rechercher(i) != nullptr; });
        absences.enregistrerAbsence(id, Date::aujourdhui().toString(), "Test base de donnees", true);
        fichiersAbsences.sauvegarderAbsences(absences);
        GestionnaireAbsences absences2 = fichiersAbsences.chargerAbsences();
        verifier(!absences2.consulterHistorique(id).empty(), "absence retrouvee apres rechargement");

        // ---- Desactivation (pas de suppression physique) ----
        relu.supprimer(id);
        relu.sauvegarderDansFichier();
        GestionnaireEmployes apresSuppression;
        apresSuppression.chargerDepuisFichier();
        verifier(apresSuppression.rechercher(id) == nullptr,
                 "employe supprime : n'est plus charge (marque 'supprime' en base)");
    } catch (const std::exception& ex) {
        std::cout << "[ECHEC] exception : " << ex.what() << "\n";
        ++echecs;
    }

    std::cout << (echecs == 0 ? "\nTOUS LES TESTS SONT PASSES\n" : "\nDES TESTS ONT ECHOUE\n");
    Database::instance().deconnecter();
    return echecs == 0 ? 0 : 1;
}
