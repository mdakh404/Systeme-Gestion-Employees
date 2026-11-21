#include "MenuAbsences.h"
#include "GestionnaireFichiersAbsences.h"
#include "Utf8.h"

#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

namespace {

template<typename Operation>
auto appliquerEtSauvegarder(GestionnaireAbsences& gestionnaire,
                            const GestionnaireFichiersAbsences& fichier,
                            Operation operation, int idSupprime = 0) {
    auto candidat = gestionnaire;
    auto resultat = operation(candidat);
    if (resultat) {
        if (idSupprime == 0) {
            fichier.sauvegarderAbsences(candidat, gestionnaire);
        } else {
            fichier.sauvegarderSuppression(candidat, gestionnaire, idSupprime);
        }
        gestionnaire = std::move(candidat);
    }
    return resultat;
}

std::size_t largeurMotifConsole(std::string_view motif) {
    std::size_t largeur = 0;
    for (std::size_t i = 0; i < motif.size();) {
        char32_t point;
        if (!utf8::lirePoint(motif, i, point)) {
            ++i;
            ++largeur;
        } else if (point < 0x0300 || point > 0x036F) {
            ++largeur;
        }
    }
    return largeur;
}

bool lireLigne(const char* invite, std::string& ligne) {
    std::cout << invite;
    return static_cast<bool>(std::getline(std::cin, ligne));
}

template<typename T>
bool lireValeur(const std::string& ligne, T& valeur) {
    std::istringstream saisie(ligne);
    if (!(saisie >> valeur)) {
        return false;
    }
    saisie >> std::ws;
    return saisie.eof();
}

std::optional<int> lireIdentifiant(const char* invite) {
    std::string ligne;
    if (!lireLigne(invite, ligne)) {
        return std::nullopt;
    }
    int identifiant;
    if (!lireValeur(ligne, identifiant) || identifiant <= 0) {
        std::cout << "Identifiant invalide. Saisissez un entier strictement positif.\n";
        return std::nullopt;
    }
    return identifiant;
}

const char* ouiNon(bool valeur) {
    return valeur ? "Oui" : "Non";
}

std::optional<bool> lireOuiNon(const char* invite, std::optional<bool> valeurParDefaut = std::nullopt) {
    std::string ligne;
    if (lireLigne(invite, ligne)) {
        if (ligne.empty() && valeurParDefaut.has_value()) {
            return valeurParDefaut;
        }
        char reponse;
        if (lireValeur(ligne, reponse)) {
            if (reponse == 'O' || reponse == 'o') {
                return true;
            }
            if (reponse == 'N' || reponse == 'n') {
                return false;
            }
        }
        std::cout << "Réponse invalide. Saisissez O ou N.\n";
    }
    return std::nullopt;
}

} // namespace

MenuAbsences::MenuAbsences(const GestionnaireAbsences& gestionnaire, Enregistrement enregistrer,
                           Modification modifier, Annulation annuler, Suppression supprimer)
    : gestionnaire(gestionnaire), enregistrer(std::move(enregistrer)),
      modifier(std::move(modifier)), annuler(std::move(annuler)), supprimer(std::move(supprimer)) {}

MenuAbsences::MenuAbsences(GestionnaireAbsences& gestionnaire,
                           const GestionnaireFichiersAbsences& fichier)
    : MenuAbsences(gestionnaire,
          [&gestionnaire, &fichier](int id, const std::string& date, const std::string& motif, bool justifiee) {
              return appliquerEtSauvegarder(gestionnaire, fichier, [&](auto& candidat) -> std::optional<int> {
                  if (!candidat.enregistrerAbsence(id, date, motif, justifiee)) {
                      return std::nullopt;
                  }
                  return candidat.consulterAbsences().back().getId();
              });
          },
          [&gestionnaire, &fichier](int id, const std::string& date, const std::string& motif, bool justifiee) {
              return appliquerEtSauvegarder(gestionnaire, fichier, [&](auto& candidat) {
                  return candidat.modifierAbsence(id, date, motif, justifiee);
              });
          },
          [&gestionnaire, &fichier](int id) {
              appliquerEtSauvegarder(gestionnaire, fichier, [&](auto& candidat) {
                  candidat.annulerAbsence(id);
                  return true;
              });
          },
          [&gestionnaire, &fichier](int id) {
              return appliquerEtSauvegarder(gestionnaire, fichier,
                  [id](auto& candidat) { return candidat.supprimerAbsence(id); }, id);
          }) {}

void MenuAbsences::executer() const {
    std::string ligne;
    while (lireLigne("\n1. Enregistrer une absence\n"
                     "2. Consulter toutes les absences actives\n"
                     "3. Consulter l'historique d'un employé\n"
                     "4. Rechercher une absence\n"
                     "5. Modifier une absence\n"
                     "6. Annuler une absence\n"
                     "7. Supprimer une absence\n"
                     "0. Retour\n\nVotre choix : ", ligne)) {
        int choix;
        if (!lireValeur(ligne, choix) || choix < 0 || choix > 7) {
            std::cout << "Choix invalide. Saisissez un nombre de 0 à 7.\n";
            continue;
        }

        try {
            switch (choix) {
            case 1: saisirAbsence(); break;
            case 2: afficherAbsences(); break;
            case 3: afficherHistorique(); break;
            case 4: rechercherAbsence(); break;
            case 5: corrigerAbsence(); break;
            case 6: annulerAbsence(); break;
            case 7: supprimerAbsence(); break;
            case 0: return;
            }
        } catch (const std::exception& erreur) {
            std::cout << "Erreur : " << erreur.what() << '\n';
        }
        if (!std::cin) {
            break;
        }
    }
}

void MenuAbsences::saisirAbsence() const {
    const auto employe = lireIdentifiant("Identifiant de l'employé : ");
    std::string date, motif;
    if (!employe || !lireLigne("Date (JJ/MM/AAAA) : ", date)
        || !lireLigne("Motif de l'absence : ", motif)) {
        return;
    }
    const auto justifiee = lireOuiNon("Absence justifiée (O/N) : ");
    if (!justifiee.has_value()) {
        return;
    }
    const auto id = enregistrer(*employe, date, motif, *justifiee);
    if (id) {
        std::cout << "Absence enregistrée avec succès !\nIdentifiant : " << *id << '\n';
        return;
    }
    std::cout << "Erreur : une absence existe déjà pour cet employé à cette date.\n";
}

void MenuAbsences::afficherAbsences() const {
    const auto absences = gestionnaire.consulterAbsencesActives();
    if (absences.empty()) {
        std::cout << "Aucune absence active.\n";
        return;
    }

    std::size_t largeurMotif = 5;
    for (const auto& absence : absences) {
        const auto taille = largeurMotifConsole(absence.getMotif());
        if (taille > largeurMotif) {
            largeurMotif = taille;
        }
    }
    std::cout << std::left << std::setw(10) << "ID" << " | "
              << std::setw(10) << "ID Employe" << " | " << std::setw(10) << "Date" << " | "
              << "Motif" << std::string(largeurMotif - 5, ' ') << " | Justifiee\n";
    for (const auto& absence : absences) {
        std::cout << std::setw(10) << absence.getId() << " | "
                  << std::setw(10) << absence.getIdEmploye() << " | "
                  << std::setw(10) << absence.getDateAbsence() << " | "
                  << absence.getMotif()
                  << std::string(largeurMotif - largeurMotifConsole(absence.getMotif()), ' ') << " | "
                  << ouiNon(absence.estJustifiee()) << '\n';
    }
}

void MenuAbsences::afficherHistorique() const {
    const auto employe = lireIdentifiant("Identifiant de l'employé : ");
    if (!employe) {
        return;
    }
    const auto historique = gestionnaire.consulterHistorique(*employe);
    std::cout << "Historique de l'employé " << *employe << '\n';
    if (historique.empty()) {
        std::cout << "Aucune absence enregistrée pour cet employé.\n";
        return;
    }
    std::cout << "ID | Date | Motif | Justifiee | Statut\n";
    for (const auto& absence : historique) {
        std::cout << absence.getId() << " | " << absence.getDateAbsence() << " | "
                  << absence.getMotif() << " | " << ouiNon(absence.estJustifiee()) << " | "
                  << (absence.estAnnulee() ? "Annulée" : "Active") << '\n';
    }
}

void MenuAbsences::rechercherAbsence() const {
    const auto identifiant = lireIdentifiant("Identifiant de l'absence : ");
    if (!identifiant) {
        return;
    }
    const auto absence = gestionnaire.rechercherAbsence(*identifiant);
    if (absence) {
        absence->afficher();
    } else {
        std::cout << "Absence introuvable.\n";
    }
}

void MenuAbsences::corrigerAbsence() const {
    std::cout << "\n========== MODIFIER UNE ABSENCE ==========\n";
    const auto id = lireIdentifiant("Identifiant de l'absence : ");
    if (!id) {
        return;
    }
    const auto absence = gestionnaire.rechercherAbsence(*id);
    if (!absence) {
        std::cout << "Erreur : absence introuvable.\n";
        return;
    }
    if (absence->estAnnulee()) {
        std::cout << "Erreur : une absence annulée ne peut pas être modifiée.\n";
        return;
    }
    std::cout << "Informations actuelles\n";
    absence->afficher();

    std::string date, motif;
    if (!lireLigne("Nouvelle date (JJ/MM/AAAA, Entrée pour conserver) : ", date)
        || !lireLigne("Nouveau motif (Entrée pour conserver) : ", motif)) {
        return;
    }
    if (date.empty()) {
        date = absence->getDateAbsence();
    }
    if (motif.empty()) {
        motif = absence->getMotif();
    }
    const auto justifiee = lireOuiNon("Justifiée (O/N, Entrée pour conserver) : ", absence->estJustifiee());
    if (!justifiee.has_value()) {
        return;
    }

    std::cout << "\nRécapitulatif de la modification\n"
              << "Date : " << absence->getDateAbsence() << " -> " << date << '\n'
              << "Motif : " << absence->getMotif() << " -> " << motif << '\n'
              << "Justifiée : " << ouiNon(absence->estJustifiee()) << " -> " << ouiNon(*justifiee) << '\n';
    if (!lireOuiNon("Confirmer la modification (O/N) : ").value_or(false)) {
        std::cout << "Modification abandonnée.\n";
        return;
    }
    if (!modifier(*id, date, motif, *justifiee)) {
        std::cout << "Erreur : une absence existe déjà pour cet employé à cette date.\n";
        return;
    }
    std::cout << "Absence modifiée avec succès !\n";
}

void MenuAbsences::annulerAbsence() const {
    const auto id = lireIdentifiant("Identifiant de l'absence : ");
    if (id) {
        annuler(*id);
        std::cout << "Absence annulée.\n";
    }
}

void MenuAbsences::supprimerAbsence() const {
    std::cout << "\n========== SUPPRIMER UNE ABSENCE ==========\n";
    const auto id = lireIdentifiant("Identifiant de l'absence : ");
    if (!id) {
        return;
    }
    const auto absence = gestionnaire.rechercherAbsence(*id);
    if (!absence) {
        std::cout << "Erreur : absence introuvable.\n";
        return;
    }
    std::cout << "Informations\n";
    absence->afficher();
    std::cout << "\nATTENTION : cette suppression est définitive.\n";
    if (!lireOuiNon("Confirmer la suppression (O/N) : ").value_or(false)) {
        std::cout << "Suppression annulée. Aucune donnée modifiée.\n";
        return;
    }
    if (!supprimer(*id)) {
        std::cout << "Erreur : absence introuvable.\n";
        return;
    }
    std::cout << "Absence supprimée avec succès !\n";
}
