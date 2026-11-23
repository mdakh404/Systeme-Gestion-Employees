#pragma once
// ---------------------------------------------------------------------------
// AuthService — authentification cote UI.
//
// Le back-end (main) ne fournit PAS de service d'authentification C++ :
//  - Employe : son propre EspaceEmploye identifie l'employe par MATRICULE ou
//    CIN, sans mot de passe. On reprend exactement cette regle ici, en
//    s'appuyant sur le vrai GestionnaireEmployes.
//  - Administrateur / RH : les comptes (login + hash) n'existent que dans le
//    script SQL (vue v_comptes_actifs), aucun code C++ ne les lit. En
//    attendant, les comptes sont lus dans data/comptes_ui.txt
//    (login;motDePasse;role). Le fichier est cree avec admin/admin et rh/rh
//    s'il n'existe pas.
//
// => A remplacer quand l'equipe back-end fournira l'authentification reelle.
// ---------------------------------------------------------------------------
#include <optional>
#include <string>

class GestionnaireEmployes;

struct SessionAuth {
    int employeId = 0;   // 0 pour les comptes administrateur / RH
    std::string login;
    std::string role;    // "administrateur" | "responsable_rh" | "employe"
};

class AuthService {
    const GestionnaireEmployes& employes_;
    std::string cheminComptes_;

public:
    explicit AuthService(const GestionnaireEmployes& employes,
                         std::string cheminComptes = "data/comptes_ui.txt");

    // Cree le fichier de comptes par defaut s'il n'existe pas. Renvoie true si cree.
    bool initialiserComptesParDefaut() const;

    // Administrateur ou RH : login + mot de passe, le role doit correspondre.
    std::optional<SessionAuth> authentifierCompte(const std::string& login,
                                                  const std::string& motDePasse,
                                                  const std::string& roleAttendu) const;

    // Employe : matricule ou CIN (meme regle que EspaceEmploye du back-end).
    // Un employe au statut "Inactif" est refuse.
    std::optional<SessionAuth> authentifierEmploye(const std::string& matriculeOuCin) const;
};
