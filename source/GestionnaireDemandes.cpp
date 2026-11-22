#include "GestionnaireDemandes.h"
#include "ExceptionsDemandes.h"
#include "Validation.h"
#include <algorithm>

void GestionnaireDemandes::valider(const DonneesDemande& d) const {
    std::vector<std::string> err;

    if (d.idEmploye <= 0) err.push_back("identifiant d'employe invalide");

    if (d.type.size() < 2 || d.type.size() > 50)
        err.push_back("type de demande invalide (2-50 caracteres)");
    else if (!Validation::sansSeparateur(d.type))
        err.push_back("type de demande : caracteres ';' et '|' interdits");

    if (d.description.size() < 3 || d.description.size() > 200)
        err.push_back("description invalide (3-200 caracteres)");
    else if (!Validation::sansSeparateur(d.description))
        err.push_back("description : caracteres ';' et '|' interdits");

    if (!d.dateDepot.estValide()) err.push_back("date de depot invalide");

    if (!err.empty()) throw DonneesDemandeInvalides(err);
}

int GestionnaireDemandes::creerDemande(const DonneesDemande& d) {
    valider(d);

    int id = prochainId_;
    demandes_.emplace(id, DemandeAdministrative(id, d));
    ++prochainId_;
    return id;
}

void GestionnaireDemandes::traiterDemande(int id, const std::string& decision) {
    auto it = demandes_.find(id);
    if (it == demandes_.end()) throw DemandeIntrouvable(id);

    if (decision != DemandeAdministrative::STATUT_ACCEPTEE &&
        decision != DemandeAdministrative::STATUT_REFUSEE)
        throw DonneesDemandeInvalides({"decision invalide (Acceptee ou Refusee)"});

    it->second.definirStatut(decision);
}

const DemandeAdministrative* GestionnaireDemandes::rechercher(int id) const {
    auto it = demandes_.find(id);
    return it == demandes_.end() ? nullptr : &it->second;
}

std::vector<DemandeAdministrative> GestionnaireDemandes::lister() const {
    std::vector<DemandeAdministrative> res;
    res.reserve(demandes_.size());
    for (const auto& [id, d] : demandes_) res.push_back(d);   // map => deja trie par id
    return res;
}

std::vector<DemandeAdministrative> GestionnaireDemandes::listerParEmploye(int idEmploye) const {
    std::vector<DemandeAdministrative> res;
    for (const auto& [id, d] : demandes_)
        if (d.getIdEmploye() == idEmploye) res.push_back(d);
    return res;
}

std::vector<DemandeAdministrative> GestionnaireDemandes::listerEnAttente() const {
    std::vector<DemandeAdministrative> res;
    for (const auto& [id, d] : demandes_)
        if (d.estEnAttente()) res.push_back(d);
    return res;
}

void GestionnaireDemandes::supprimerDemande(int id) {
    demandes_.erase(id);
}

// Rollback uniquement : restaure le statut precedent apres un echec d'ecriture.
void GestionnaireDemandes::restaurerStatut(int id, const std::string& statut) {
    auto it = demandes_.find(id);
    if (it == demandes_.end()) return;
    it->second.restaurerStatut(statut);
}

void GestionnaireDemandes::charger(const std::vector<DemandeAdministrative>& donnees) {
    demandes_.clear();
    prochainId_ = 1;

    for (const auto& d : donnees) {
        demandes_.emplace(d.getId(), d);
        if (d.getId() >= prochainId_) prochainId_ = d.getId() + 1;
    }
}
