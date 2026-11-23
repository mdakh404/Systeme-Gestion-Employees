#include "Database.h"

#include <cstdlib>
#include <fstream>
#include <map>

#ifdef USE_MYSQL
#include <mysql.h>
#endif

#ifdef USE_MYSQL
namespace {

std::string nettoyer(const std::string& s) {
    const char* blancs = " \t\r\n";
    std::size_t debut = s.find_first_not_of(blancs);
    if (debut == std::string::npos) return "";
    std::size_t fin = s.find_last_not_of(blancs);
    return s.substr(debut, fin - debut + 1);
}

// Fichier de configuration : lignes "cle=valeur", '#' = commentaire.
std::map<std::string, std::string> lireConfiguration(const std::string& chemin) {
    std::map<std::string, std::string> cfg;
    std::ifstream in(chemin);
    std::string ligne;
    while (std::getline(in, ligne)) {
        ligne = nettoyer(ligne);
        if (ligne.empty() || ligne[0] == '#') continue;
        std::size_t egal = ligne.find('=');
        if (egal == std::string::npos) continue;
        cfg[nettoyer(ligne.substr(0, egal))] = nettoyer(ligne.substr(egal + 1));
    }
    return cfg;
}

void surchargerParEnvironnement(std::map<std::string, std::string>& cfg) {
    const struct { const char* variable; const char* cle; } liens[] = {
        {"EMS_DB_HOST", "host"}, {"EMS_DB_PORT", "port"}, {"EMS_DB_USER", "user"},
        {"EMS_DB_PASSWORD", "password"}, {"EMS_DB_NAME", "database"}};
    for (const auto& lien : liens) {
        if (const char* valeur = std::getenv(lien.variable)) cfg[lien.cle] = valeur;
    }
}

// Base sur la machine locale (ex. XAMPP) : pas de risque d'ecoute reseau.
bool estHoteLocal(const std::string& hote) {
    return hote == "127.0.0.1" || hote == "localhost" || hote == "::1";
}

} // namespace
#endif

Database& Database::instance() {
    static Database db;
    return db;
}

std::string Database::description() const {
    return utilisateur_ + "@" + hote_ + ":" + std::to_string(port_) + "/" + base_;
}

#ifdef USE_MYSQL

bool Database::connecter() {
    if (connexion_) return true;

    auto cfg = lireConfiguration("data/db.cfg");
    surchargerParEnvironnement(cfg);
    if (cfg.count("host")) hote_ = cfg["host"];
    if (cfg.count("user")) utilisateur_ = cfg["user"];
    if (cfg.count("password")) motDePasse_ = cfg["password"];
    if (cfg.count("database")) base_ = cfg["database"];
    if (cfg.count("port")) {
        try { port_ = static_cast<unsigned int>(std::stoul(cfg["port"])); } catch (...) {}
    }

    MYSQL* m = mysql_init(nullptr);
    if (!m) {
        erreur_ = "mysql_init a echoue";
        return false;
    }
    unsigned int delai = 5;
    mysql_options(m, MYSQL_OPT_CONNECT_TIMEOUT, &delai);
    mysql_options(m, MYSQL_SET_CHARSET_NAME, "utf8mb4");

    if (estHoteLocal(hote_)) {
        char off = 0;
        mysql_options(m, MYSQL_OPT_SSL_ENFORCE, &off);
        mysql_options(m, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &off);
    }

    if (!mysql_real_connect(m, hote_.c_str(), utilisateur_.c_str(), motDePasse_.c_str(),
                            base_.c_str(), port_, nullptr, 0)) {
        erreur_ = mysql_error(m);
        mysql_close(m);
        return false;
    }
    mysql_set_character_set(m, "utf8mb4");
    connexion_ = m;
    erreur_.clear();
    return true;
}

void Database::deconnecter() {
    if (connexion_) {
        mysql_close(static_cast<MYSQL*>(connexion_));
        connexion_ = nullptr;
    }
}

std::vector<LigneSql> Database::requete(const std::string& sql) {
    if (!connexion_) throw std::runtime_error("Pas de connexion a la base de donnees.");
    MYSQL* c = static_cast<MYSQL*>(connexion_);

    if (mysql_real_query(c, sql.c_str(), static_cast<unsigned long>(sql.size())) != 0) {
        throw std::runtime_error(std::string("Erreur SQL : ") + mysql_error(c));
    }

    std::vector<LigneSql> lignes;
    MYSQL_RES* res = mysql_store_result(c);
    if (!res) {
        if (mysql_field_count(c) != 0) {
            throw std::runtime_error(std::string("Erreur SQL : ") + mysql_error(c));
        }
        return lignes;   // INSERT / UPDATE / DELETE : pas de resultat
    }

    const unsigned int nbChamps = mysql_num_fields(res);
    while (MYSQL_ROW row = mysql_fetch_row(res)) {
        unsigned long* longueurs = mysql_fetch_lengths(res);
        LigneSql ligne;
        ligne.reserve(nbChamps);
        for (unsigned int i = 0; i < nbChamps; ++i) {
            if (row[i]) ligne.emplace_back(std::string(row[i], longueurs[i]));
            else ligne.emplace_back(std::nullopt);
        }
        lignes.push_back(std::move(ligne));
    }
    mysql_free_result(res);
    return lignes;
}

std::string Database::echapper(const std::string& valeur) const {
    if (!connexion_) throw std::runtime_error("Pas de connexion a la base de donnees.");
    std::string sortie(valeur.size() * 2 + 1, '\0');
    unsigned long n = mysql_real_escape_string(static_cast<MYSQL*>(connexion_), &sortie[0],
                                               valeur.c_str(),
                                               static_cast<unsigned long>(valeur.size()));
    sortie.resize(n);
    return sortie;
}

#else   // ---- compilation sans MySQL : mode fichiers uniquement ----

bool Database::connecter() {
    erreur_ = "programme compile sans -DUSE_MYSQL";
    return false;
}
void Database::deconnecter() {}
std::vector<LigneSql> Database::requete(const std::string&) {
    throw std::runtime_error("Base de donnees indisponible (compilation sans -DUSE_MYSQL).");
}
std::string Database::echapper(const std::string&) const {
    throw std::runtime_error("Base de donnees indisponible (compilation sans -DUSE_MYSQL).");
}

#endif

void Database::executer(const std::string& sql) {
    requete(sql);
}