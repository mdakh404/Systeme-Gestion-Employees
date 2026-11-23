#include "Operations.h"
#include "../BackendTypes.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace {

// Certaines methodes du back-end ecrivent directement sur std::cout
// (ex. GestionnaireConges::deposerDemande). L'affichage etant le role de
// l'UI, on neutralise ces messages pendant l'appel.
class SilenceCout {
    std::ostringstream tampon_;
    std::streambuf* ancien_;
public:
    SilenceCout() : ancien_(std::cout.rdbuf(tampon_.rdbuf())) {}
    ~SilenceCout() { std::cout.rdbuf(ancien_); }
    SilenceCout(const SilenceCout&) = delete;
    SilenceCout& operator=(const SilenceCout&) = delete;
};

template <typename T>
T& exiger(T* service, const char* nom) {
    if (!service) throw std::logic_error(std::string("Service non initialise : ") + nom);
    return *service;
}

// Meme principe que appliquerEtSauvegarder() dans MenuAbsences.cpp du back-end.
template <typename Operation>
auto appliquerAbsences(const BackendServices& s, Operation operation, int idSupprime = 0) {
    GestionnaireAbsences& ga = exiger(s.absences, "absences");
    const GestionnaireFichiersAbsences& fichier = exiger(s.fichiersAbsences, "fichiers absences");
    GestionnaireAbsences candidat = ga;
    auto resultat = operation(candidat);
    if (resultat) {
        if (idSupprime == 0) fichier.sauvegarderAbsences(candidat, ga);
        else fichier.sauvegarderSuppression(candidat, ga, idSupprime);
        ga = std::move(candidat);
    }
    return resultat;
}

template <typename Operation>
auto appliquerDemandes(const BackendServices& s, Operation operation) {
    GestionnaireDemandes& gd = exiger(s.demandes, "demandes");
    const GestionnaireFichiersDemandes& fichier = exiger(s.fichiersDemandes, "fichiers demandes");
    const std::vector<DemandeAdministrative> avant = gd.lister();
    auto resultat = operation(gd);
    try {
        fichier.ecrireDemandes(gd.lister());
    } catch (...) {
        gd.charger(avant);   // annule en memoire si l'ecriture echoue
        throw;
    }
    return resultat;
}

} // namespace

namespace integration {

// ---------------- Formats ----------------
std::string dateFrVersIso(const std::string& fr) {
    const Date d = Date::fromString(fr); // valide le format et le calendrier
    std::ostringstream os;
    os << d.annee << '-' << (d.mois < 10 ? "0" : "") << d.mois << '-'
       << (d.jour < 10 ? "0" : "") << d.jour;
    return os.str();
}

std::string dateIsoVersFr(const std::string& iso) {
    if (iso.size() != 10 || iso[4] != '-' || iso[7] != '-') return iso; // deja lisible
    return iso.substr(8, 2) + "/" + iso.substr(5, 2) + "/" + iso.substr(0, 4);
}

const std::vector<std::string>& typesConge() {
    static const std::vector<std::string> types = {"Annuel", "Maladie", "Sans solde", "Exceptionnel"};
    return types;
}

const std::vector<std::string>& statutsEmploye() {
    static const std::vector<std::string> statuts = {"Actif", "Inactif", "En conge", "Suspendu"};
    return statuts;
}

// ---------------- Employes ----------------
void sauvegarderEmployes(const BackendServices& s) {
    exiger(s.employes, "employes").sauvegarderDansFichier();
}

void affecterDepartement(const BackendServices& s, int idEmploye, int idDepartement) {
    if (idDepartement != 0 && !exiger(s.departements, "departements").contient(idDepartement))
        throw DepartementIntrouvable(idDepartement);
    exiger(s.employes, "employes").affecterDepartement(idEmploye, idDepartement);
    sauvegarderEmployes(s);
}

std::string nomDepartement(const BackendServices& s, int idDepartement) {
    if (idDepartement == 0) return "-";
    if (s.departements) {
        if (auto d = s.departements->rechercher(idDepartement)) return d->getNom();
    }
    return "#" + std::to_string(idDepartement);
}

// ---------------- Departements ----------------
void sauvegarderDepartements(const BackendServices& s) {
    exiger(s.departements, "departements").sauvegarderDansFichier();
}

// ---------------- Conges ----------------
int deposerConge(const BackendServices& s, int idEmploye, const std::string& debutFr,
                 const std::string& finFr, const std::string& type) {
    GestionnaireConges& gc = exiger(s.conges, "conges");
    const std::string debut = dateFrVersIso(debutFr);
    const std::string fin = dateFrVersIso(finFr);
    {
        SilenceCout silence;
        gc.deposerDemande(idEmploye, debut, fin, type); // sauvegarde incluse
    }
    // deposerDemande ne renvoie pas l'id : c'est le plus grand id existant.
    int id = 0;
    for (const Conge& c : gc.tousLesConges()) id = std::max(id, c.Getid());
    return id;
}

void traiterConge(const BackendServices& s, int idConge, bool accepter) {
    SilenceCout silence;
    exiger(s.conges, "conges").traiterDemande(idConge, accepter); // sauvegarde incluse
}

// ---------------- Absences ----------------
std::optional<int> enregistrerAbsence(const BackendServices& s, int idEmploye,
                                      const std::string& dateFr, const std::string& motif,
                                      bool justifiee) {
    return appliquerAbsences(s, [&](GestionnaireAbsences& candidat) -> std::optional<int> {
        if (!candidat.enregistrerAbsence(idEmploye, dateFr, motif, justifiee)) return std::nullopt;
        return candidat.consulterAbsences().back().getId();
    });
}

bool modifierAbsence(const BackendServices& s, int idAbsence, const std::string& dateFr,
                     const std::string& motif, bool justifiee) {
    return appliquerAbsences(s, [&](GestionnaireAbsences& candidat) {
        return candidat.modifierAbsence(idAbsence, dateFr, motif, justifiee);
    });
}

void annulerAbsence(const BackendServices& s, int idAbsence) {
    appliquerAbsences(s, [&](GestionnaireAbsences& candidat) {
        candidat.annulerAbsence(idAbsence);
        return true;
    });
}

bool supprimerAbsence(const BackendServices& s, int idAbsence) {
    return appliquerAbsences(s, [&](GestionnaireAbsences& candidat) {
        return candidat.supprimerAbsence(idAbsence);
    }, idAbsence);
}

// ---------------- Demandes administratives ----------------
int deposerDemande(const BackendServices& s, int idEmploye, const std::string& type,
                   const std::string& description) {
    return appliquerDemandes(s, [&](GestionnaireDemandes& gd) {
        DonneesDemande d;
        d.idEmploye = idEmploye;
        d.type = type;
        d.description = description;
        d.dateDepot = Date::aujourdhui();
        return gd.creerDemande(d);
    });
}

void traiterDemande(const BackendServices& s, int idDemande, bool accepter) {
    appliquerDemandes(s, [&](GestionnaireDemandes& gd) {
        gd.traiterDemande(idDemande, accepter ? DemandeAdministrative::STATUT_ACCEPTEE
                                              : DemandeAdministrative::STATUT_REFUSEE);
        return true;
    });
}

} // namespace integration
