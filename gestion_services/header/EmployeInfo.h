#ifndef EMPLOYEINFO_H
#define EMPLOYEINFO_H

#include <functional>
#include <optional>
#include <string>

// Vue minimale d'un employe, fournie par le module Employes.
//
// Le service de recompenses ne depend PAS de la classe Employe : il recoit ce
// petit adaptateur. Cela permet de compiler et de tester ce module seul, sans
// attendre la fusion du module Employes.
struct EmployeInfo {
    int id = 0;
    std::string matricule;
    std::string nom;
    std::string prenom;

    std::string nomComplet() const {
        return prenom.empty() ? nom : prenom + " " + nom;
    }
};

// Retourne std::nullopt si l'employe n'existe pas.
using FournisseurEmployes = std::function<std::optional<EmployeInfo>(int)>;

#endif
