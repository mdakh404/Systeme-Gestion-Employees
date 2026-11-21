#include "PrimeService.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

PrimeService::PrimeService(int id,GestionnaireEmployes& gestion,const std::string& name,const std::string& description,double tauxParAnnee,double tauxMaximum,int ancienneteMinimum): 
	Service(id, name, description),gestion_(gestion),tauxParAnnee_(tauxParAnnee),tauxMaximum_(tauxMaximum),ancienneteMinimum_(ancienneteMinimum) {
    validerTaux(tauxParAnnee_);
    validerTaux(tauxMaximum_);
    if (tauxParAnnee_ > tauxMaximum_) {
        throw std::invalid_argument("taux par annee superieur au taux maximum");
    }
    if (ancienneteMinimum_ < 0) {
        throw std::invalid_argument("anciennete minimum negative");
    }
}

void PrimeService::validerTaux(double taux) {
    if (!std::isfinite(taux) || taux < 0.0 || taux > 1.0) {
        throw std::invalid_argument("le taux doit etre compris entre 0 et 1");
    }
}

double PrimeService::arrondirCentimes(double montant) {
    return std::round(montant * 100.0) / 100.0;
}

double PrimeService::calculerTaux(int anneesAnciennete) const {
    if (anneesAnciennete < 0) {
        throw std::invalid_argument("anciennete negative");
    }
    if (anneesAnciennete < ancienneteMinimum_) {
        return 0.0;
    }
    const double taux = tauxParAnnee_ * static_cast<double>(anneesAnciennete);
    return taux < tauxMaximum_ ? taux : tauxMaximum_;
}

double PrimeService::calculerMontant(const Employe& employe) const {
    return arrondirCentimes(
        employe.getSalaire().getSalaireBase() * calculerTaux(employe.anciennete()));
}

PrimeService::Bonification PrimeService::bonifier(const Employe& employe) {
    Bonification resultat;
    resultat.idEmploye = employe.getId();
    resultat.matricule = employe.getMatricule();
    resultat.salaireBase = employe.getSalaire().getSalaireBase();
    resultat.anneesAnciennete = employe.anciennete();
    resultat.taux = calculerTaux(resultat.anneesAnciennete);
    resultat.montantPrime = arrondirCentimes(resultat.salaireBase * resultat.taux);
    return resultat;
}

void PrimeService::setTauxParAnnee(double tauxParAnnee) {
    validerTaux(tauxParAnnee);
    if (tauxParAnnee > tauxMaximum_) {
        throw std::invalid_argument("taux par annee superieur au taux maximum");
    }
    tauxParAnnee_ = tauxParAnnee;
}

void PrimeService::setTauxMaximum(double tauxMaximum) {
    validerTaux(tauxMaximum);
    if (tauxMaximum < tauxParAnnee_) {
        throw std::invalid_argument("taux maximum inferieur au taux par annee");
    }
    tauxMaximum_ = tauxMaximum;
}

void PrimeService::setAncienneteMinimum(int ancienneteMinimum) {
    if (ancienneteMinimum < 0) {
        throw std::invalid_argument("anciennete minimum negative");
    }
    ancienneteMinimum_ = ancienneteMinimum;
}

void PrimeService::display() const {
    std::cout << "Service #" << id << " - " << name << '\n'
              << "  Description : " << description << '\n'
              << "  Parametres  : " << (tauxParAnnee_ * 100.0) << " % par annee, "
              << "plafond " << (tauxMaximum_ * 100.0) << " %, "
              << "anciennete minimum " << ancienneteMinimum_ << " ans\n";
}

void PrimeService::execute() {
    std::cout << "--- " << name << " ---\n";

    for (const Employe& employe : gestion_.lister()) {
        std::cout << employe.getId() << " - " << employe.getPrenom() << ' '<< employe.getNom() << " (" << employe.getMatricule() << ")\n";
    }

    std::cout << "Id de l'employe : ";
    int idEmploye = 0;
    if (!(std::cin >> idEmploye)) {
        std::cout << "Saisie invalide.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }

    const Employe* employe = gestion_.rechercher(idEmploye);
    if (employe == nullptr) {
        std::cout << "Employe introuvable.\n";
        return;
    }

    const Bonification resultat = bonifier(*employe);

    const std::ios_base::fmtflags flags = std::cout.flags();
    const std::streamsize precision = std::cout.precision();
    std::cout << std::fixed << std::setprecision(2)
              << "Anciennete     : " << resultat.anneesAnciennete << " an(s)\n"
              << "Taux applique  : " << (resultat.taux * 100.0) << " %\n"
              << "Prime accordee : " << resultat.montantPrime << '\n';
    std::cout.flags(flags);
    std::cout.precision(precision);
}
