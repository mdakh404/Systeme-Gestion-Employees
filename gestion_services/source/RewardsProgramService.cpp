#include "RewardsProgramService.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <utility>

namespace {

constexpr int STOCK_MAXIMUM = 1000000;

// --- Date & String validation helpers ---
bool anneeBissextile(int annee) {
    return (annee % 400 == 0) || (annee % 4 == 0 && annee % 100 != 0);
}

bool dateIsoValide(const std::string& date) {
    if (date.size() != 10) return false;
    if (date[4] != '-' || date[7] != '-') return false;

    for (std::size_t i = 0; i < date.size(); ++i) {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(static_cast<unsigned char>(date[i]))) return false;
    }

    const int annee = std::stoi(date.substr(0, 4));
    const int mois = std::stoi(date.substr(5, 2));
    const int jour = std::stoi(date.substr(8, 2));

    if (annee < 1900 || mois < 1 || mois > 12 || jour < 1) return false;

    static const int joursMois[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int jourMaximum = joursMois[mois - 1];
    if (mois == 2 && anneeBissextile(annee)) jourMaximum = 29;

    return jour <= jourMaximum;
}

int comparerDates(const std::string& gauche, const std::string& droite) {
    if (gauche < droite) return -1;
    if (droite < gauche) return 1;
    return 0;
}

bool contientSeparateur(const std::string& valeur) {
    return valeur.find(';') != std::string::npos ||
           valeur.find('\n') != std::string::npos ||
           valeur.find('\r') != std::string::npos;
}

void verifierTexteObligatoire(const std::string& valeur, const std::string& champ) {
    if (valeur.empty()) {
        throw std::invalid_argument(champ + " ne doit pas etre vide.");
    }
    if (contientSeparateur(valeur)) {
        throw std::invalid_argument(champ + " ne doit pas contenir ';' ni de saut de ligne.");
    }
    if (valeur.size() > 200) {
        throw std::invalid_argument(champ + " est trop long (200 caracteres maximum).");
    }
}

void verifierTexteOptionnel(const std::string& valeur, const std::string& champ) {
    if (contientSeparateur(valeur)) {
        throw std::invalid_argument(champ + " ne doit pas contenir ';' ni de saut de ligne.");
    }
    if (valeur.size() > 200) {
        throw std::invalid_argument(champ + " est trop long (200 caracteres maximum).");
    }
}

std::string champ(std::istringstream& flux) {
    std::string valeur;
    std::getline(flux, valeur, ';');
    return valeur;
}

int entierDepuisTexte(const std::string& texte) {
    std::size_t position = 0;
    const int valeur = std::stoi(texte, &position);
    if (position != texte.size()) {
        throw std::runtime_error("entier invalide : " + texte);
    }
    return valeur;
}

bool booleenDepuisTexte(const std::string& texte) {
    if (texte == "1") return true;
    if (texte == "0") return false;
    throw std::runtime_error("booleen invalide : " + texte);
}

bool lireLigne(const std::string& invite, std::string& sortie) {
    std::cout << invite;
    return static_cast<bool>(std::getline(std::cin, sortie));
}

bool lireEntier(const std::string& invite, int& sortie) {
    std::string ligne;
    if (!lireLigne(invite, ligne)) return false;
    std::istringstream flux(ligne);
    if (!(flux >> sortie)) return false;
    flux >> std::ws;
    return flux.eof();
}

} // namespace

// ---------------------------------------------------------------------------
// TypeProgramme
// ---------------------------------------------------------------------------

std::string toString(TypeProgramme type) {
    switch (type) {
        case TypeProgramme::Performance:    return "performance";
        case TypeProgramme::Fidelite:       return "fidelite";
        case TypeProgramme::Reconnaissance: return "reconnaissance";
    }
    return "performance";
}

TypeProgramme typeDepuisTexte(const std::string& texte) {
    if (texte == "performance")    return TypeProgramme::Performance;
    if (texte == "fidelite")       return TypeProgramme::Fidelite;
    if (texte == "reconnaissance") return TypeProgramme::Reconnaissance;
    throw std::invalid_argument("Type de programme inconnu : " + texte);
}

// ---------------------------------------------------------------------------
// ProgrammeRecompense
// ---------------------------------------------------------------------------

ProgrammeRecompense::ProgrammeRecompense(int id,
                                         std::string nom,
                                         std::string description,
                                         TypeProgramme type,
                                         int pointsMaxParAttribution,
                                         std::string dateDebut,
                                         std::string dateFin,
                                         bool actif)
    : id_(id),
      nom_(std::move(nom)),
      description_(std::move(description)),
      type_(type),
      pointsMaxParAttribution_(pointsMaxParAttribution),
      dateDebut_(std::move(dateDebut)),
      dateFin_(std::move(dateFin)),
      actif_(actif) {
    if (id_ <= 0) {
        throw std::invalid_argument("Identifiant de programme invalide.");
    }
    verifierTexteObligatoire(nom_, "nom du programme");
    verifierTexteOptionnel(description_, "description du programme");
    if (pointsMaxParAttribution_ <= 0) {
        throw std::invalid_argument("Le maximum de points doit etre strictement positif.");
    }
    validerPeriode();
}

void ProgrammeRecompense::validerPeriode() const {
    if (!dateIsoValide(dateDebut_)) {
        throw std::invalid_argument("Date de debut invalide (format attendu : AAAA-MM-JJ).");
    }
    if (dateFin_.empty()) return;
    if (!dateIsoValide(dateFin_)) {
        throw std::invalid_argument("Date de fin invalide (format attendu : AAAA-MM-JJ).");
    }
    if (comparerDates(dateDebut_, dateFin_) > 0) {
        throw std::invalid_argument("La date de fin precede la date de debut.");
    }
}

void ProgrammeRecompense::setNom(const std::string& nom) {
    verifierTexteObligatoire(nom, "nom du programme");
    nom_ = nom;
}

void ProgrammeRecompense::setDescription(const std::string& description) {
    verifierTexteOptionnel(description, "description du programme");
    description_ = description;
}

void ProgrammeRecompense::setType(TypeProgramme type) {
    type_ = type;
}

void ProgrammeRecompense::setPointsMaxParAttribution(int points) {
    if (points <= 0) {
        throw std::invalid_argument("Le maximum de points doit etre strictement positif.");
    }
    pointsMaxParAttribution_ = points;
}

void ProgrammeRecompense::setPeriode(const std::string& dateDebut, const std::string& dateFin) {
    const std::string ancienneDebut = dateDebut_;
    const std::string ancienneFin = dateFin_;
    dateDebut_ = dateDebut;
    dateFin_ = dateFin;
    try {
        validerPeriode();
    } catch (...) {
        dateDebut_ = ancienneDebut;
        dateFin_ = ancienneFin;
        throw;
    }
}

bool ProgrammeRecompense::estDisponible(const std::string& date) const {
    if (!actif_) return false;
    if (!dateIsoValide(date)) {
        throw std::invalid_argument("Date de reference invalide (format attendu : AAAA-MM-JJ).");
    }
    if (comparerDates(date, dateDebut_) < 0) return false;
    if (!dateFin_.empty() && comparerDates(date, dateFin_) > 0) return false;
    return true;
}

void ProgrammeRecompense::afficher() const {
    std::cout << "  #" << id_ << " - " << nom_
              << " [" << toString(type_) << "]"
              << (actif_ ? "" : " (desactive)") << "\n"
              << "      " << (description_.empty() ? "(sans description)" : description_) << "\n"
              << "      Points max / attribution : " << pointsMaxParAttribution_ << "\n"
              << "      Periode : " << dateDebut_ << " -> "
              << (dateFin_.empty() ? "illimitee" : dateFin_) << "\n";
}

// ---------------------------------------------------------------------------
// Recompense
// ---------------------------------------------------------------------------

Recompense::Recompense(int id,
                       std::string nom,
                       std::string description,
                       int coutPoints,
                       int stock)
    : id_(id),
      nom_(std::move(nom)),
      description_(std::move(description)),
      coutPoints_(coutPoints),
      stock_(stock) {
    if (id_ <= 0) {
        throw std::invalid_argument("Identifiant de recompense invalide.");
    }
    verifierTexteObligatoire(nom_, "nom de la recompense");
    verifierTexteOptionnel(description_, "description de la recompense");
    if (coutPoints_ <= 0) {
        throw std::invalid_argument("Le cout en points doit etre strictement positif.");
    }
    if (stock_ < 0 || stock_ > STOCK_MAXIMUM) {
        throw std::invalid_argument("Stock invalide.");
    }
}

void Recompense::setNom(const std::string& nom) {
    verifierTexteObligatoire(nom, "nom de la recompense");
    nom_ = nom;
}

void Recompense::setDescription(const std::string& description) {
    verifierTexteOptionnel(description, "description de la recompense");
    description_ = description;
}

void Recompense::setCoutPoints(int coutPoints) {
    if (coutPoints <= 0) {
        throw std::invalid_argument("Le cout en points doit etre strictement positif.");
    }
    coutPoints_ = coutPoints;
}

void Recompense::setStock(int stock) {
    if (stock < 0 || stock > STOCK_MAXIMUM) {
        throw std::invalid_argument("Stock invalide.");
    }
    stock_ = stock;
}

void Recompense::retirerDuStock() {
    if (stock_ <= 0) {
        throw std::invalid_argument("Recompense en rupture de stock.");
    }
    --stock_;
}

void Recompense::reapprovisionner(int quantite) {
    if (quantite <= 0) {
        throw std::invalid_argument("La quantite de reapprovisionnement doit etre positive.");
    }
    if (stock_ + quantite > STOCK_MAXIMUM) {
        throw std::invalid_argument("Le stock depasserait le maximum autorise.");
    }
    stock_ += quantite;
}

void Recompense::afficher() const {
    std::cout << "  #" << id_ << " - " << nom_
              << " (" << coutPoints_ << " points)"
              << (estDisponible() ? "" : " [rupture de stock]") << "\n"
              << "      " << (description_.empty() ? "(sans description)" : description_) << "\n"
              << "      Stock : " << stock_ << "\n";
}

// ---------------------------------------------------------------------------
// RewardsProgramService
// ---------------------------------------------------------------------------

RewardsProgramService::RewardsProgramService(int id,
                                             FournisseurEmployes fournisseur,
                                             std::string cheminFichier,
                                             std::string dateReference)
    : Service(id,
              "Programme de recompenses",
              "Programmes, catalogue, attribution et utilisation de points"),
      fournisseur_(std::move(fournisseur)),
      cheminFichier_(std::move(cheminFichier)),
      dateReference_(std::move(dateReference)) {
    if (id <= 0) {
        throw std::invalid_argument("Identifiant de service invalide.");
    }
    if (!dateReference_.empty() && !dateIsoValide(dateReference_)) {
        throw std::invalid_argument("Date de reference invalide (format attendu : AAAA-MM-JJ).");
    }
    charger();
}

// --- Programmes ---

int RewardsProgramService::ajouterProgramme(const std::string& nom,
                                            const std::string& description,
                                            TypeProgramme type,
                                            int pointsMaxParAttribution,
                                            const std::string& dateDebut,
                                            const std::string& dateFin) {
    ProgrammeRecompense programme(prochainIdProgramme_, nom, description, type,
                                  pointsMaxParAttribution, dateDebut, dateFin, true);
    programmes_.push_back(programme);
    ++prochainIdProgramme_;
    sauvegarder();
    return programme.getId();
}

void RewardsProgramService::modifierProgramme(int idProgramme,
                                              const std::string& nom,
                                              const std::string& description,
                                              TypeProgramme type,
                                              int pointsMaxParAttribution,
                                              const std::string& dateDebut,
                                              const std::string& dateFin) {
    const ProgrammeRecompense& existant = programmeModifiable(idProgramme);
    ProgrammeRecompense nouveau(idProgramme, nom, description, type,
                                pointsMaxParAttribution, dateDebut, dateFin, existant.estActif());
    programmeModifiable(idProgramme) = nouveau;
    sauvegarder();
}

void RewardsProgramService::desactiverProgramme(int idProgramme) {
    programmeModifiable(idProgramme).desactiver();
    sauvegarder();
}

void RewardsProgramService::activerProgramme(int idProgramme) {
    programmeModifiable(idProgramme).activer();
    sauvegarder();
}

const ProgrammeRecompense* RewardsProgramService::trouverProgramme(int idProgramme) const {
    for (const auto& programme : programmes_) {
        if (programme.getId() == idProgramme) return &programme;
    }
    return nullptr;
}

ProgrammeRecompense& RewardsProgramService::programmeModifiable(int idProgramme) {
    for (auto& programme : programmes_) {
        if (programme.getId() == idProgramme) return programme;
    }
    throw std::invalid_argument("Programme introuvable (id = " + std::to_string(idProgramme) + ").");
}

// --- Catalogue ---

int RewardsProgramService::ajouterRecompense(const std::string& nom,
                                             const std::string& description,
                                             int coutPoints,
                                             int stock) {
    Recompense recompense(prochainIdRecompense_, nom, description, coutPoints, stock);
    catalogue_.push_back(recompense);
    ++prochainIdRecompense_;
    sauvegarder();
    return recompense.getId();
}

void RewardsProgramService::modifierStock(int idRecompense, int nouveauStock) {
    recompenseModifiable(idRecompense).setStock(nouveauStock);
    sauvegarder();
}

const Recompense* RewardsProgramService::trouverRecompense(int idRecompense) const {
    for (const auto& recompense : catalogue_) {
        if (recompense.getId() == idRecompense) return &recompense;
    }
    return nullptr;
}

Recompense& RewardsProgramService::recompenseModifiable(int idRecompense) {
    for (auto& recompense : catalogue_) {
        if (recompense.getId() == idRecompense) return recompense;
    }
    throw std::invalid_argument("Recompense introuvable (id = " + std::to_string(idRecompense) + ").");
}

// --- Attribution / utilisation ---

void RewardsProgramService::verifierEmploye(int idEmploye) const {
    if (idEmploye <= 0) {
        throw std::invalid_argument("Identifiant employe invalide.");
    }
    if (!fournisseur_) {
        throw std::logic_error("Fournisseur d'employes non configure.");
    }
    if (!fournisseur_(idEmploye)) {
        throw std::invalid_argument("Employe introuvable (id = " + std::to_string(idEmploye) + ").");
    }
}

int RewardsProgramService::attribuerPoints(int idEmploye,
                                           int idProgramme,
                                           int points,
                                           const std::string& motif) {
    verifierEmploye(idEmploye);

    ProgrammeRecompense& programme = programmeModifiable(idProgramme);
    if (!programme.estActif()) {
        throw std::invalid_argument("Ce programme est desactive.");
    }

    const std::string date = dateDuJour();
    if (!programme.estDisponible(date)) {
        throw std::invalid_argument("Ce programme n'est pas valide a la date du jour.");
    }

    if (points <= 0) {
        throw std::invalid_argument("Le nombre de points doit etre strictement positif.");
    }
    if (points > programme.getPointsMaxParAttribution()) {
        throw std::invalid_argument("Le nombre de points depasse le maximum du programme.");
    }

    verifierTexteObligatoire(motif, "motif");

    for (const auto& attribution : attributions_) {
        if (attribution.idEmploye == idEmploye &&
            attribution.idProgramme == idProgramme &&
            attribution.date == date) {
            throw std::invalid_argument(
                "Une attribution existe deja aujourd'hui pour cet employe et ce programme.");
        }
    }

    AttributionRecompense attribution;
    attribution.id = prochainIdAttribution_++;
    attribution.idEmploye = idEmploye;
    attribution.idProgramme = idProgramme;
    attribution.points = points;
    attribution.motif = motif;
    attribution.date = date;
    attributions_.push_back(attribution);

    sauvegarder();
    return attribution.id;
}

int RewardsProgramService::utiliserRecompense(int idEmploye, int idRecompense) {
    verifierEmploye(idEmploye);

    Recompense& recompense = recompenseModifiable(idRecompense);
    if (!recompense.estDisponible()) {
        throw std::invalid_argument("Cette recompense est en rupture de stock.");
    }

    const int disponible = solde(idEmploye);
    if (disponible < recompense.getCoutPoints()) {
        throw std::invalid_argument("Solde de points insuffisant.");
    }

    recompense.retirerDuStock();

    UtilisationRecompense utilisation;
    utilisation.id = prochainIdUtilisation_++;
    utilisation.idEmploye = idEmploye;
    utilisation.idRecompense = idRecompense;
    utilisation.points = recompense.getCoutPoints();
    utilisation.date = dateDuJour();
    utilisations_.push_back(utilisation);

    sauvegarder();
    return utilisation.id;
}

// --- Consultation ---

int RewardsProgramService::solde(int idEmploye) const {
    if (idEmploye <= 0) {
        throw std::invalid_argument("Identifiant employe invalide.");
    }

    long long total = 0;
    for (const auto& attribution : attributions_) {
        if (attribution.idEmploye == idEmploye) total += attribution.points;
    }
    for (const auto& utilisation : utilisations_) {
        if (utilisation.idEmploye == idEmploye) total -= utilisation.points;
    }

    if (total < 0) {
        throw std::logic_error("Solde negatif : le registre de points est incoherent.");
    }
    return static_cast<int>(total);
}

std::vector<AttributionRecompense> RewardsProgramService::historique(int idEmploye) const {
    if (idEmploye <= 0) {
        throw std::invalid_argument("Identifiant employe invalide.");
    }

    std::vector<AttributionRecompense> resultat;
    for (const auto& attribution : attributions_) {
        if (attribution.idEmploye == idEmploye) resultat.push_back(attribution);
    }

    std::sort(resultat.begin(), resultat.end(),
              [](const AttributionRecompense& gauche, const AttributionRecompense& droite) {
                  if (gauche.date != droite.date) return gauche.date < droite.date;
                  return gauche.id < droite.id;
              });
    return resultat;
}

std::vector<UtilisationRecompense> RewardsProgramService::utilisations(int idEmploye) const {
    if (idEmploye <= 0) {
        throw std::invalid_argument("Identifiant employe invalide.");
    }

    std::vector<UtilisationRecompense> resultat;
    for (const auto& utilisation : utilisations_) {
        if (utilisation.idEmploye == idEmploye) resultat.push_back(utilisation);
    }

    std::sort(resultat.begin(), resultat.end(),
              [](const UtilisationRecompense& gauche, const UtilisationRecompense& droite) {
                  if (gauche.date != droite.date) return gauche.date < droite.date;
                  return gauche.id < droite.id;
              });
    return resultat;
}

StatistiquesRecompenses RewardsProgramService::statistiques() const {
    StatistiquesRecompenses stats;

    stats.totalProgrammes = static_cast<int>(programmes_.size());
    for (const auto& programme : programmes_) {
        if (programme.estActif()) ++stats.programmesActifs;
    }

    stats.totalAttributions = static_cast<int>(attributions_.size());
    for (const auto& attribution : attributions_) {
        stats.pointsDistribues += attribution.points;
    }

    stats.totalUtilisations = static_cast<int>(utilisations_.size());
    for (const auto& utilisation : utilisations_) {
        stats.pointsUtilises += utilisation.points;
    }

    std::map<int, int> pointsNets;
    for (const auto& attribution : attributions_) {
        pointsNets[attribution.idEmploye] += attribution.points;
    }
    for (const auto& utilisation : utilisations_) {
        pointsNets[utilisation.idEmploye] -= utilisation.points;
    }

    for (const auto& entree : pointsNets) {
        stats.topEmployes.emplace_back(entree.first, entree.second);
    }

    std::sort(stats.topEmployes.begin(), stats.topEmployes.end(),
              [](const std::pair<int, int>& gauche, const std::pair<int, int>& droite) {
                  if (gauche.second != droite.second) return gauche.second > droite.second;
                  return gauche.first < droite.first;
              });

    return stats;
}

void RewardsProgramService::afficherStatistiques() const {
    const StatistiquesRecompenses stats = statistiques();

    std::cout << "\n--- Statistiques ---\n"
              << "Programmes      : " << stats.totalProgrammes
              << " (actifs : " << stats.programmesActifs << ")\n"
              << "Attributions    : " << stats.totalAttributions
              << " | points distribues : " << stats.pointsDistribues << "\n"
              << "Utilisations    : " << stats.totalUtilisations
              << " | points utilises   : " << stats.pointsUtilises << "\n";

    if (stats.topEmployes.empty()) return;

    std::cout << "Top employes (points nets) :\n";
    const std::size_t limite = stats.topEmployes.size() < 5 ? stats.topEmployes.size() : 5;
    for (std::size_t i = 0; i < limite; ++i) {
        std::cout << "  " << (i + 1) << ". employe #" << stats.topEmployes[i].first
                  << " : " << stats.topEmployes[i].second << " points\n";
    }
}

// --- Persistance ---

std::string RewardsProgramService::dateDuJour() const {
    if (!dateReference_.empty()) return dateReference_;

    const std::time_t maintenant = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &maintenant);
#else
    localtime_r(&maintenant, &local);
#endif

    char tampon[11] = {};
    std::strftime(tampon, sizeof(tampon), "%Y-%m-%d", &local);
    return tampon;
}

void RewardsProgramService::charger() {
    programmes_.clear();
    catalogue_.clear();
    attributions_.clear();
    utilisations_.clear();
    prochainIdProgramme_ = 1;
    prochainIdRecompense_ = 1;
    prochainIdAttribution_ = 1;
    prochainIdUtilisation_ = 1;

    if (cheminFichier_.empty()) return;

    std::ifstream flux(cheminFichier_);
    if (!flux) return;

    std::string ligne;
    int numero = 0;
    while (std::getline(flux, ligne)) {
        ++numero;
        if (ligne.empty() || ligne[0] == '#') continue;

        try {
            std::istringstream champs(ligne);
            const std::string type = champ(champs);

            if (type == "P") {
                const int id = entierDepuisTexte(champ(champs));
                const std::string nom = champ(champs);
                const std::string description = champ(champs);
                const TypeProgramme nature = typeDepuisTexte(champ(champs));
                const int pointsMax = entierDepuisTexte(champ(champs));
                const std::string debut = champ(champs);
                const std::string fin = champ(champs);
                const bool actif = booleenDepuisTexte(champ(champs));
                programmes_.emplace_back(id, nom, description, nature, pointsMax, debut, fin, actif);
            } else if (type == "R") {
                const int id = entierDepuisTexte(champ(champs));
                const std::string nom = champ(champs);
                const std::string description = champ(champs);
                const int cout = entierDepuisTexte(champ(champs));
                const int stock = entierDepuisTexte(champ(champs));
                catalogue_.emplace_back(id, nom, description, cout, stock);
            } else if (type == "A") {
                AttributionRecompense attribution;
                attribution.id = entierDepuisTexte(champ(champs));
                attribution.idEmploye = entierDepuisTexte(champ(champs));
                attribution.idProgramme = entierDepuisTexte(champ(champs));
                attribution.points = entierDepuisTexte(champ(champs));
                attribution.motif = champ(champs);
                attribution.date = champ(champs);
                attributions_.push_back(attribution);
            } else if (type == "U") {
                UtilisationRecompense utilisation;
                utilisation.id = entierDepuisTexte(champ(champs));
                utilisation.idEmploye = entierDepuisTexte(champ(champs));
                utilisation.idRecompense = entierDepuisTexte(champ(champs));
                utilisation.points = entierDepuisTexte(champ(champs));
                utilisation.date = champ(champs);
                utilisations_.push_back(utilisation);
            } else if (type == "C") {
                prochainIdProgramme_ = entierDepuisTexte(champ(champs));
                prochainIdRecompense_ = entierDepuisTexte(champ(champs));
                prochainIdAttribution_ = entierDepuisTexte(champ(champs));
                prochainIdUtilisation_ = entierDepuisTexte(champ(champs));
            } else {
                throw std::runtime_error("type d'enregistrement inconnu : " + type);
            }
        } catch (const std::exception& erreur) {
            throw std::runtime_error("Fichier de recompenses corrompu, ligne " +
                                     std::to_string(numero) + " : " + erreur.what());
        }
    }

    for (const auto& programme : programmes_) {
        prochainIdProgramme_ = std::max(prochainIdProgramme_, programme.getId() + 1);
    }
    for (const auto& recompense : catalogue_) {
        prochainIdRecompense_ = std::max(prochainIdRecompense_, recompense.getId() + 1);
    }
    for (const auto& attribution : attributions_) {
        prochainIdAttribution_ = std::max(prochainIdAttribution_, attribution.id + 1);
    }
    for (const auto& utilisation : utilisations_) {
        prochainIdUtilisation_ = std::max(prochainIdUtilisation_, utilisation.id + 1);
    }
}

void RewardsProgramService::sauvegarder() const {
    if (cheminFichier_.empty()) return;

    const std::filesystem::path cible(cheminFichier_);
    if (cible.has_parent_path()) {
        std::filesystem::create_directories(cible.parent_path());
    }
    const std::filesystem::path temporaire = cible.string() + ".tmp";

    std::ofstream flux(temporaire, std::ios::trunc);
    if (!flux) {
        throw std::runtime_error("Impossible d'ecrire le fichier de recompenses : " + cheminFichier_);
    }

    flux << "#recompenses v1\n";

    for (const auto& programme : programmes_) {
        flux << "P;" << programme.getId()
             << ';' << programme.getNom()
             << ';' << programme.getDescription()
             << ';' << toString(programme.getType())
             << ';' << programme.getPointsMaxParAttribution()
             << ';' << programme.getDateDebut()
             << ';' << programme.getDateFin()
             << ';' << (programme.estActif() ? 1 : 0)
             << '\n';
    }

    for (const auto& recompense : catalogue_) {
        flux << "R;" << recompense.getId()
             << ';' << recompense.getNom()
             << ';' << recompense.getDescription()
             << ';' << recompense.getCoutPoints()
             << ';' << recompense.getStock()
             << '\n';
    }

    for (const auto& attribution : attributions_) {
        flux << "A;" << attribution.id
             << ';' << attribution.idEmploye
             << ';' << attribution.idProgramme
             << ';' << attribution.points
             << ';' << attribution.motif
             << ';' << attribution.date
             << '\n';
    }

    for (const auto& utilisation : utilisations_) {
        flux << "U;" << utilisation.id
             << ';' << utilisation.idEmploye
             << ';' << utilisation.idRecompense
             << ';' << utilisation.points
             << ';' << utilisation.date
             << '\n';
    }

    flux << "C;" << prochainIdProgramme_
         << ';' << prochainIdRecompense_
         << ';' << prochainIdAttribution_
         << ';' << prochainIdUtilisation_
         << '\n';

    flux.close();
    if (!flux) {
        throw std::runtime_error("Erreur lors de l'ecriture du fichier de recompenses.");
    }

    std::error_code erreur;
    std::filesystem::rename(temporaire, cible, erreur);
    if (erreur) {
        std::filesystem::remove(cible, erreur);
        std::filesystem::rename(temporaire, cible, erreur);
        if (erreur) {
            throw std::runtime_error("Impossible de remplacer le fichier de recompenses : " +
                                     erreur.message());
        }
    }
}

// --- Interface Service ---

void RewardsProgramService::display() const {
    int actifs = 0;
    for (const auto& programme : programmes_) {
        if (programme.estActif()) ++actifs;
    }

    std::cout << "Service #" << id << " - " << name << "\n"
              << "  " << description << "\n"
              << "  Programmes   : " << programmes_.size()
              << " (" << actifs << " actifs)\n"
              << "  Catalogue    : " << catalogue_.size() << " article(s)\n"
              << "  Attributions : " << attributions_.size()
              << " | Utilisations : " << utilisations_.size() << "\n";
}

void RewardsProgramService::execute() {
    auto afficherProgrammes = [this]() {
        if (programmes_.empty()) {
            std::cout << "Aucun programme.\n";
            return;
        }
        for (const auto& programme : programmes_) programme.afficher();
    };

    auto afficherCatalogue = [this]() {
        if (catalogue_.empty()) {
            std::cout << "Catalogue vide.\n";
            return;
        }
        for (const auto& recompense : catalogue_) recompense.afficher();
    };

    while (true) {
        std::cout << "\n=== " << name << " ===\n"
                  << "1. Lister les programmes\n"
                  << "2. Ajouter un programme\n"
                  << "3. Activer / desactiver un programme\n"
                  << "4. Lister le catalogue\n"
                  << "5. Ajouter une recompense\n"
                  << "6. Reapprovisionner une recompense\n"
                  << "7. Attribuer des points\n"
                  << "8. Consulter le solde d'un employe\n"
                  << "9. Utiliser une recompense\n"
                  << "10. Statistiques\n"
                  << "0. Retour\n"
                  << "Choix : ";

        std::string ligne;
        if (!std::getline(std::cin, ligne)) return;

        int choix = -1;
        try {
            std::size_t position = 0;
            choix = std::stoi(ligne, &position);
            if (position != ligne.size()) choix = -1;
        } catch (const std::exception&) {
            choix = -1;
        }

        try {
            switch (choix) {
                case 0:
                    return;

                case 1:
                    afficherProgrammes();
                    break;

                case 2: {
                    std::string nom;
                    std::string description;
                    std::string debut;
                    std::string fin;
                    int type = 0;
                    int pointsMax = 0;

                    if (!lireLigne("Nom : ", nom)) return;
                    if (!lireLigne("Description (vide autorisee) : ", description)) return;
                    if (!lireEntier("Type (1=performance, 2=fidelite, 3=reconnaissance) : ", type) ||
                        type < 1 || type > 3) {
                        std::cout << "Type invalide.\n";
                        break;
                    }
                    if (!lireEntier("Points maximum par attribution : ", pointsMax)) {
                        std::cout << "Valeur invalide.\n";
                        break;
                    }
                    if (!lireLigne("Date de debut (AAAA-MM-JJ) : ", debut)) return;
                    if (!lireLigne("Date de fin (vide = illimitee) : ", fin)) return;

                    const TypeProgramme nature = type == 1 ? TypeProgramme::Performance
                                                : type == 2 ? TypeProgramme::Fidelite
                                                            : TypeProgramme::Reconnaissance;
                    const int idProgramme = ajouterProgramme(nom, description, nature,
                                                             pointsMax, debut, fin);
                    std::cout << "Programme cree (id " << idProgramme << ").\n";
                    break;
                }

                case 3: {
                    int idProgramme = 0;
                    if (!lireEntier("Id du programme : ", idProgramme)) {
                        std::cout << "Valeur invalide.\n";
                        break;
                    }
                    const ProgrammeRecompense* programme = trouverProgramme(idProgramme);
                    if (programme == nullptr) {
                        std::cout << "Programme introuvable.\n";
                        break;
                    }
                    const bool etaitActif = programme->estActif();
                    if (etaitActif) {
                        desactiverProgramme(idProgramme);
                    } else {
                        activerProgramme(idProgramme);
                    }
                    std::cout << "Programme " << (etaitActif ? "desactive" : "active") << ".\n";
                    break;
                }

                case 4:
                    afficherCatalogue();
                    break;

                case 5: {
                    std::string nom;
                    std::string description;
                    int cout = 0;
                    int stock = 0;

                    if (!lireLigne("Nom : ", nom)) return;
                    if (!lireLigne("Description (vide autorisee) : ", description)) return;
                    if (!lireEntier("Cout en points : ", cout)) {
                        std::cout << "Valeur invalide.\n";
                        break;
                    }
                    if (!lireEntier("Stock initial : ", stock)) {
                        std::cout << "Valeur invalide.\n";
                        break;
                    }

                    const int idRecompense = ajouterRecompense(nom, description, cout, stock);
                    std::cout << "Recompense ajoutee (id " << idRecompense << ").\n";
                    break;
                }

                case 6: {
                    int idRecompense = 0;
                    int quantite = 0;
                    if (!lireEntier("Id de la recompense : ", idRecompense)) {
                        std::cout << "Valeur invalide.\n";
                        break;
                    }
                    const Recompense* recompense = trouverRecompense(idRecompense);
                    if (recompense == nullptr) {
                        std::cout << "Recompense introuvable.\n";
                        break;
                    }
                    if (!lireEntier("Quantite a ajouter : ", quantite)) {
                        std::cout << "Valeur invalide.\n";
                        break;
                    }
                    modifierStock(idRecompense, recompense->getStock() + quantite);
                    std::cout << "Stock mis a jour.\n";
                    break;
                }

                case 7: {
                    int idEmploye = 0;
                    int idProgramme = 0;
                    int points = 0;
                    std::string motif;

                    if (!lireEntier("Id employe : ", idEmploye)) {
                        std::cout << "Valeur invalide.\n";
                        break;
                    }
                    if (!lireEntier("Id programme : ", idProgramme)) {
                        std::cout << "Valeur invalide.\n";
                        break;
                    }
                    if (!lireEntier("Points a attribuer : ", points)) {
                        std::cout << "Valeur invalide.\n";
                        break;
                    }
                    if (!lireLigne("Motif : ", motif)) return;

                    attribuerPoints(idEmploye, idProgramme, points, motif);
                    std::cout << "Points attribues. Nouveau solde : "
                              << solde(idEmploye) << ".\n";
                    break;
                }

                case 8: {
                    int idEmploye = 0;
                    if (!lireEntier("Id employe : ", idEmploye)) {
                        std::cout << "Valeur invalide.\n";
                        break;
                    }
                    std::cout << "Solde : " << solde(idEmploye) << " points.\n";
                    const std::vector<AttributionRecompense> lignes = historique(idEmploye);
                    if (lignes.empty()) {
                        std::cout << "Aucune attribution.\n";
                        break;
                    }
                    for (const auto& attribution : lignes) {
                        std::cout << "  " << attribution.date
                                  << " | programme #" << attribution.idProgramme
                                  << " | +" << attribution.points
                                  << " | " << attribution.motif << "\n";
                    }
                    break;
                }

                case 9: {
                    int idEmploye = 0;
                    int idRecompense = 0;
                    if (!lireEntier("Id employe : ", idEmploye)) {
                        std::cout << "Valeur invalide.\n";
                        break;
                    }
                    if (!lireEntier("Id recompense : ", idRecompense)) {
                        std::cout << "Valeur invalide.\n";
                        break;
                    }
                    utiliserRecompense(idEmploye, idRecompense);
                    std::cout << "Recompense utilisee. Solde restant : "
                              << solde(idEmploye) << ".\n";
                    break;
                }

                case 10:
                    afficherStatistiques();
                    break;

                default:
                    std::cout << "Choix invalide.\n";
                    break;
            }
        } catch (const std::exception& erreur) {
            std::cout << "Erreur : " << erreur.what() << "\n";
        }
    }
}
