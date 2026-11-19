#pragma once
#include <string>
#include "Date.h"

// Classe de base : informations communes a toute personne
class Personne {
protected:
    int id_;
    std::string nom_, prenom_, cin_, email_, telephone_;
    Date dateNaissance_;

public:
    Personne(int id, const std::string& nom, const std::string& prenom,
             const std::string& cin, const Date& dateNaissance,
             const std::string& email, const std::string& telephone);
    virtual ~Personne() = default;

    virtual void afficherInfos() const;   // polymorphisme

    int getId() const { return id_; }
    const std::string& getNom() const { return nom_; }
    const std::string& getPrenom() const { return prenom_; }
    const std::string& getCin() const { return cin_; }
    const Date& getDateNaissance() const { return dateNaissance_; }
    const std::string& getEmail() const { return email_; }
    const std::string& getTelephone() const { return telephone_; }

    void setNom(const std::string& v) { nom_ = v; }
    void setPrenom(const std::string& v) { prenom_ = v; }
    void setCin(const std::string& v) { cin_ = v; }
    void setDateNaissance(const Date& v) { dateNaissance_ = v; }
    void setEmail(const std::string& v) { email_ = v; }
    void setTelephone(const std::string& v) { telephone_ = v; }
};
