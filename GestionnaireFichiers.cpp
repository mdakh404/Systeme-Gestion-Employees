#include "GestionnaireFichiers.h"
#include "Exceptions.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <iomanip>

static std::vector<std::string> decouper(const std::string& s, char sep) {
    std::vector<std::string> out;
    std::string cur;
    std::istringstream is(s);
    while (std::getline(is, cur, sep)) out.push_back(cur);
    if (!s.empty() && s.back() == sep) out.push_back("");
    return out;
}

// Lit toutes les lignes non vides d'un fichier (liste vide si le fichier n'existe pas)
static std::vector<std::pair<int, std::string>> lireLignes(const std::string& chemin) {
    std::vector<std::pair<int, std::string>> lignes;
    std::ifstream in(chemin);
    if (!in) return lignes;
    std::string ligne;
    int numero = 0;
    while (std::getline(in, ligne)) {
        ++numero;
        if (!ligne.empty() && ligne.back() == '\r') ligne.pop_back();
        if (!ligne.empty()) lignes.emplace_back(numero, ligne);
    }
    return lignes;
}

// Ecriture dans un fichier temporaire puis renommage : le fichier existant n'est
// jamais corrompu si l'ecriture echoue.
static void ecrireFichier(const std::string& chemin, const std::string& contenu) {
    const std::string tmp = chemin + ".tmp";
    {
        std::ofstream out(tmp, std::ios::trunc);
        if (!out) throw ErreurFichier("impossible d'ecrire " + tmp);
        out << contenu;
        out.flush();
        if (!out) throw ErreurFichier("echec d'ecriture dans " + tmp);
    }
    std::remove(chemin.c_str());
    if (std::rename(tmp.c_str(), chemin.c_str()) != 0)
        throw ErreurFichier("renommage impossible: " + chemin);
}

// ------------------------------- Employes -------------------------------

std::vector<Employe> GestionnaireFichiers::lireEmployes() {
    std::vector<Employe> res;
    for (const auto& [numero, ligne] : lireLignes(cheminEmployes_)) {
        auto c = decouper(ligne, ';');
        if (c.size() != 14)
            throw ErreurFichier(cheminEmployes_ + " ligne " + std::to_string(numero) +
                                " : 14 champs attendus, " + std::to_string(c.size()) + " trouves");
        try {
            DonneesEmploye d;
            d.nom = c[1]; d.prenom = c[2]; d.cin = c[3];
            d.dateNaissance = Date::fromString(c[4]);
            d.email = c[5]; d.telephone = c[6];
            d.dateEmbauche = Date::fromString(c[8]);
            d.statut = c[9];
            d.salaireBase = std::stod(c[12]);

            Employe e(std::stoi(c[0]), c[7], d);
            e.setIdDepartementBrut(std::stoi(c[10]));
            e.setIdPosteBrut(std::stoi(c[11]));
            e.setHistorique(decouper(c[13], '|'));
            res.push_back(std::move(e));
        } catch (const std::exception& ex) {
            throw ErreurFichier(cheminEmployes_ + " ligne " + std::to_string(numero) + " : " + ex.what());
        }
    }
    return res;
}

void GestionnaireFichiers::sauvegarderEmployes(const std::vector<Employe>& employes) {
    std::ostringstream out;
    for (const Employe& e : employes) {
        std::string hist;
        for (size_t i = 0; i < e.getHistorique().size(); ++i)
            hist += (i ? "|" : "") + e.getHistorique()[i];

        out << e.getId() << ';' << e.getNom() << ';' << e.getPrenom() << ';'
            << e.getCin() << ';' << e.getDateNaissance().toString() << ';'
            << e.getEmail() << ';' << e.getTelephone() << ';'
            << e.getMatricule() << ';' << e.getDateEmbauche().toString() << ';'
            << e.getStatut() << ';' << e.getIdDepartement() << ';' << e.getIdPoste() << ';'
            << std::fixed << std::setprecision(2) << e.getSalaire().getSalaireBase() << ';'
            << hist << '\n';
    }
    ecrireFichier(cheminEmployes_, out.str());
}

// ------------------------------ Departements ----------------------------

std::vector<Departement> GestionnaireFichiers::lireDepartements() {
    std::vector<Departement> res;
    for (const auto& [numero, ligne] : lireLignes(cheminDepartements_)) {
        auto c = decouper(ligne, ';');
        if (c.size() != 3)
            throw ErreurFichier(cheminDepartements_ + " ligne " + std::to_string(numero) +
                                " : 3 champs attendus, " + std::to_string(c.size()) + " trouves");
        try {
            res.emplace_back(std::stoi(c[0]), c[1], c[2]);
        } catch (const std::exception& ex) {
            throw ErreurFichier(cheminDepartements_ + " ligne " + std::to_string(numero) + " : " + ex.what());
        }
    }
    return res;
}

void GestionnaireFichiers::sauvegarderDepartements(const std::vector<Departement>& departements) {
    std::ostringstream out;
    for (const Departement& d : departements)
        out << d.getId() << ';' << d.getNom() << ';' << d.getDescription() << '\n';
    ecrireFichier(cheminDepartements_, out.str());
}
