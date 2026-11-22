#pragma once
#include <stdexcept>
#include <string>
#include <vector>

// Hierarchie d'exceptions du module Employes
class ExceptionEmploye : public std::runtime_error {
public:
    explicit ExceptionEmploye(const std::string& msg) : std::runtime_error(msg) {}
};

class EmployeIntrouvable : public ExceptionEmploye {
public:
    explicit EmployeIntrouvable(int id)
        : ExceptionEmploye("Employe introuvable (id = " + std::to_string(id) + ")") {}
};

class EmployeExistant : public ExceptionEmploye {
public:
    explicit EmployeExistant(const std::string& cin)
        : ExceptionEmploye("Un employe avec ce CIN existe deja: " + cin) {}
};

class DonneesInvalides : public ExceptionEmploye {
    std::vector<std::string> erreurs_;
    static std::string joindre(const std::vector<std::string>& e) {
        std::string s = "Donnees invalides: ";
        for (size_t i = 0; i < e.size(); ++i) s += (i ? "; " : "") + e[i];
        return s;
    }
public:
    explicit DonneesInvalides(const std::vector<std::string>& e)
        : ExceptionEmploye(joindre(e)), erreurs_(e) {}
    const std::vector<std::string>& erreurs() const { return erreurs_; }
};

class ExceptionDepartement : public std::runtime_error {
public:
    explicit ExceptionDepartement(const std::string& msg) : std::runtime_error(msg) {}
};

class DepartementIntrouvable : public ExceptionDepartement {
public:
    explicit DepartementIntrouvable(int id)
        : ExceptionDepartement("Departement introuvable (id = " + std::to_string(id) + ")") {}
};

class DepartementExistant : public ExceptionDepartement {
public:
    explicit DepartementExistant(const std::string& nom)
        : ExceptionDepartement("Un departement avec ce nom existe deja: " + nom) {}
};

class DepartementUtilise : public ExceptionDepartement {
public:
    explicit DepartementUtilise(int id)
        : ExceptionDepartement("Impossible de supprimer un departement affecte (id = " +
                               std::to_string(id) + ")") {}
};
