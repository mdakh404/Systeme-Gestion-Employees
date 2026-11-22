#include "DemandeAdministrative.h"
#include "ExceptionsDemandes.h"
#include <iostream>

DemandeAdministrative::DemandeAdministrative(int id, const DonneesDemande& d)
    : id_(id),
      idEmploye_(d.idEmploye),
      type_(d.type),
      description_(d.description),
      dateDepot_(d.dateDepot),
      statut_(STATUT_EN_ATTENTE) {}

DemandeAdministrative::DemandeAdministrative(int id, int idEmploye, const std::string& type,
                                             const std::string& description, const Date& dateDepot,
                                             const std::string& statut)
    : id_(id),
      idEmploye_(idEmploye),
      type_(type),
      description_(description),
      dateDepot_(dateDepot),
      statut_(statut) {}

void DemandeAdministrative::afficherInfos() const {
    std::cout << "  Identifiant   : " << id_ << "\n"
              << "  Employe       : " << idEmploye_ << "\n"
              << "  Type          : " << type_ << "\n"
              << "  Date de depot : " << dateDepot_.toString() << "\n"
              << "  Statut        : " << statut_ << "\n"
              << "  Description   : " << description_ << "\n";
}

bool DemandeAdministrative::statutValide(const std::string& statut) {
    return statut == STATUT_EN_ATTENTE || statut == STATUT_ACCEPTEE ||
           statut == STATUT_REFUSEE;
}

void DemandeAdministrative::definirStatut(const std::string& statut) {
    if (!statutValide(statut))
        throw DonneesDemandeInvalides({"statut invalide (En attente, Acceptee, Refusee)"});

    // Une decision deja prise est definitive
    if (!estEnAttente()) throw DemandeDejaTraitee(id_, statut_);

    statut_ = statut;
}

// Rollback apres un echec de sauvegarde : restaure le statut precedent.
void DemandeAdministrative::restaurerStatut(const std::string& statut) {
    if (statutValide(statut)) statut_ = statut;
}

std::vector<std::string> DemandeAdministrative::typesProposes() {
    return {"Attestation de travail",
            "Changement de poste",
            "Changement de departement",
            "Demande de prime",
            "Formation",
            "Autre"};
}
