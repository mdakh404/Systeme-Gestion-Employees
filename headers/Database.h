#pragma once
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

// Une ligne de resultat SQL : un champ vaut std::nullopt quand la valeur SQL est NULL.
using LigneSql = std::vector<std::optional<std::string>>;

// Acces MySQL / MariaDB (singleton).
//  - Compile avec -DUSE_MYSQL : vraie connexion (libmariadb ou libmysqlclient).
//  - Compile sans l'option    : connecter() renvoie toujours false et l'application
//    continue avec les fichiers data/*.txt, comme avant.
// Parametres de connexion : fichier data/db.cfg (voir data/db.cfg.example), puis
// variables d'environnement EMS_DB_HOST, EMS_DB_PORT, EMS_DB_USER, EMS_DB_PASSWORD, EMS_DB_NAME.
class Database {
public:
    static Database& instance();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    bool connecter();
    void deconnecter();
    bool estConnectee() const { return connexion_ != nullptr; }
    const std::string& derniereErreur() const { return erreur_; }
    std::string description() const;   // "utilisateur@hote:port/base"

    // Toutes les erreurs SQL sont signalees par std::runtime_error (message du serveur inclus).
    void executer(const std::string& sql);
    std::vector<LigneSql> requete(const std::string& sql);
    std::string echapper(const std::string& valeur) const;

private:
    Database() = default;
    ~Database() { deconnecter(); }

    void* connexion_ = nullptr;
    std::string erreur_;
    std::string hote_ = "127.0.0.1";
    std::string utilisateur_ = "root";
    std::string motDePasse_;
    std::string base_ = "gestion_employes";
    unsigned int port_ = 3306;
};

// Transaction RAII : ROLLBACK automatique si commit() n'a pas ete appele.
class Transaction {
    Database& db_;
    bool active_ = true;

public:
    explicit Transaction(Database& db) : db_(db) { db_.executer("START TRANSACTION"); }
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;
    void commit() {
        db_.executer("COMMIT");
        active_ = false;
    }
    ~Transaction() {
        if (active_) {
            try { db_.executer("ROLLBACK"); } catch (...) {}
        }
    }
};
