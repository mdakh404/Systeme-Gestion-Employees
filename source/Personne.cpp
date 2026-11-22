#include "Personne.h"
#include <iostream>

Personne::Personne(int id, const std::string& nom, const std::string& prenom,
                   const std::string& cin, const Date& dateNaissance,
                   const std::string& email, const std::string& telephone)
    : id_(id), nom_(nom), prenom_(prenom), cin_(cin), email_(email),
      telephone_(telephone), dateNaissance_(dateNaissance) {}

void Personne::afficherInfos() const {
    std::cout << "[" << id_ << "] " << prenom_ << " " << nom_
              << " | CIN: " << cin_
              << " | Ne(e) le " << dateNaissance_.toString()
              << " | " << email_ << " | " << telephone_ << "\n";
}
