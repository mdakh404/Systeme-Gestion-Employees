// Persistance MySQL des employes : tables personne + employe + salaire.
#include "GestionnaireEmployes.h"

#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>

#include "Database.h"
#include "DbPersistence.h"

void GestionnaireEmployes::chargerDepuisBD() {
    Database& base = Database::instance();

    auto lignes = base.requete(
        "SELECT p.id, e.matricule, p.nom, p.prenom, p.cin, p.email, IFNULL(p.telephone, ''), "
        "DATE_FORMAT(p.date_naissance, '%d/%m/%Y'), DATE_FORMAT(e.date_embauche, '%d/%m/%Y'), "
        "e.statut, IFNULL(s.salaire_base, 0), IFNULL(e.departement_id, 0), IFNULL(e.poste_id, 0) "
        "FROM personne p JOIN employe e ON e.id = p.id "
        "LEFT JOIN salaire s ON s.employe_id = e.id "
        "WHERE e.statut <> 'supprime' ORDER BY p.id");
    auto maxId = base.requete("SELECT IFNULL(MAX(id), 0) FROM personne");

    employes_.clear();
    idsBD_.clear();
    prochainId_ = 1;
    if (!maxId.empty() && maxId[0][0]) prochainId_ = std::stoi(*maxId[0][0]) + 1;

    int rejetees = 0;
    for (const auto& l : lignes) {
        try {
            int id = std::stoi(*l[0]);
            DonneesEmploye d;
            d.nom = *l[2];
            d.prenom = *l[3];
            d.cin = *l[4];
            d.email = *l[5];
            d.telephone = l[6] ? *l[6] : "";
            d.dateNaissance = Date::fromString(*l[7]);
            d.dateEmbauche = Date::fromString(*l[8]);
            d.statut = db::statutEmployeDepuisSql(*l[9]);
            d.salaireBase = std::stod(*l[10]);

            Employe e(id, *l[1], d);
            int idDep = std::stoi(*l[11]);
            int idPoste = std::stoi(*l[12]);
            if (idDep > 0) e.affecterDepartement(idDep);
            if (idPoste > 0) e.affecterPoste(idPoste);

            employes_.emplace(id, std::move(e));
            idsBD_.insert(id);
            if (id >= prochainId_) prochainId_ = id + 1;
        } catch (const std::exception& ex) {
            ++rejetees;
            std::cerr << "Employe ignore (donnees invalides en base) : " << ex.what() << "\n";
        }
    }
    if (rejetees > 0) {
        std::cerr << "Attention : " << rejetees << " employe(s) de la base n'ont pas pu etre charges.\n";
    }
}

void GestionnaireEmployes::sauvegarderDansBD() const {
    Database& base = Database::instance();
    Transaction tx(base);
    db::Collecteur col;
    std::set<int> enregistres;

    for (const auto& [id, e] : employes_) {
        const bool ok = col.essayer("employe " + std::to_string(id) + " (" + e.getMatricule() + ")", [&] {
            const std::string idTxt = db::n(id);
            const int idDep = e.getIdDepartement();
            const int idPoste = e.getIdPoste();

            // Le departement / poste doit exister (cle etrangere) : creation minimale si besoin.
            if (idDep > 0) {
                base.executer("INSERT IGNORE INTO departement (id, nom) VALUES (" + db::n(idDep)
                              + ", " + db::q("Departement " + std::to_string(idDep)) + ")");
            }
            if (idPoste > 0) {
                base.executer("INSERT IGNORE INTO poste (id, intitule) VALUES (" + db::n(idPoste)
                              + ", " + db::q("Poste " + std::to_string(idPoste)) + ")");
            }

            // personne : INSERT ou UPDATE selon l'existence (les doublons de CIN / email sont
            // ainsi signales par la base au lieu de modifier la ligne d'un autre employe).
            const std::string tel = e.getTelephone().empty() ? "NULL" : db::q(e.getTelephone());
            const bool existe =
                !base.requete("SELECT 1 FROM personne WHERE id = " + idTxt).empty();
            if (existe) {
                base.executer("UPDATE personne SET nom = " + db::q(e.getNom())
                              + ", prenom = " + db::q(e.getPrenom())
                              + ", cin = " + db::q(e.getCin())
                              + ", date_naissance = " + db::q(db::dateSql(e.getDateNaissance()))
                              + ", email = " + db::q(e.getEmail())
                              + ", telephone = " + tel + " WHERE id = " + idTxt);
            } else {
                base.executer("INSERT INTO personne (id, nom, prenom, cin, date_naissance, email, "
                              "telephone) VALUES (" + idTxt + ", " + db::q(e.getNom()) + ", "
                              + db::q(e.getPrenom()) + ", " + db::q(e.getCin()) + ", "
                              + db::q(db::dateSql(e.getDateNaissance())) + ", "
                              + db::q(e.getEmail()) + ", " + tel + ")");
            }

            // employe : un statut de sortie fixe en base (retraite...) n'est pas ecrase par "Inactif".
            const std::string statut = db::q(db::statutEmployeVersSql(e.getStatut()));
            base.executer(
                "INSERT INTO employe (id, matricule, date_embauche, statut, departement_id, "
                "poste_id) VALUES (" + idTxt + ", " + db::q(e.getMatricule()) + ", "
                + db::q(db::dateSql(e.getDateEmbauche())) + ", " + statut + ", "
                + (idDep > 0 ? db::n(idDep) : "NULL") + ", "
                + (idPoste > 0 ? db::n(idPoste) : "NULL") + ") "
                "ON DUPLICATE KEY UPDATE matricule = VALUES(matricule), "
                "date_embauche = VALUES(date_embauche), "
                "statut = CASE WHEN VALUES(statut) = 'inactif' AND statut IN "
                "('demissionnaire', 'licencie', 'retraite') THEN statut ELSE VALUES(statut) END, "
                "departement_id = VALUES(departement_id), poste_id = VALUES(poste_id)");

            // salaire : le trigger SQL alimente historique_salaire uniquement si la valeur change.
            std::ostringstream montant;
            montant << std::fixed << std::setprecision(2) << e.getSalaire().getSalaireBase();
            base.executer("INSERT INTO salaire (employe_id, salaire_base) VALUES (" + idTxt + ", "
                          + montant.str() + ") ON DUPLICATE KEY UPDATE "
                          "salaire_base = VALUES(salaire_base)");
        });
        if (ok) enregistres.insert(id);
    }

    // Employes retires de la memoire : pas de suppression physique (trigger trg_employe_bd),
    // on les marque 'supprime' ; ils ne sont plus charges au demarrage suivant.
    std::string aDesactiver;
    for (int id : idsBD_) {
        if (employes_.find(id) == employes_.end()) {
            if (!aDesactiver.empty()) aDesactiver += ", ";
            aDesactiver += std::to_string(id);
        }
    }
    if (!aDesactiver.empty()) {
        col.essayer("desactivation des employes supprimes", [&] {
            base.executer("UPDATE employe SET statut = 'supprime' WHERE id IN (" + aDesactiver + ")");
        });
    }

    tx.commit();

    // Memorise les employes presents en base ; un employe dont l'ecriture a echoue mais qui etait
    // deja connu reste suivi.
    std::set<int> connus = enregistres;
    for (int id : idsBD_) {
        if (employes_.find(id) != employes_.end()) connus.insert(id);
    }
    idsBD_ = connus;

    col.lancerSiErreurs("Sauvegarde des employes");
}
