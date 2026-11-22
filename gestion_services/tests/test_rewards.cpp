#include "RewardsProgramService.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

namespace {

int reussis = 0;
int echoues = 0;

void verifier(bool condition, const std::string& nom) {
    if (condition) {
        ++reussis;
        std::cout << "  [OK]    " << nom << "\n";
    } else {
        ++echoues;
        std::cout << "  [ECHEC] " << nom << "\n";
    }
}

template <typename Action>
void verifierLeve(Action action, const std::string& nom) {
    bool leve = false;
    try {
        action();
    } catch (const std::exception&) {
        leve = true;
    }
    verifier(leve, nom);
}

std::optional<EmployeInfo> annuaire(int id) {
    if (id == 1) return EmployeInfo{1, "EMP0001", "Alaoui", "Karim"};
    if (id == 2) return EmployeInfo{2, "EMP0002", "Benani", "Salma"};
    return std::nullopt;
}

const std::string DATE = "2026-11-22";

} // namespace

int main() {
    const std::filesystem::path dossier = std::filesystem::temp_directory_path();
    const std::filesystem::path fichier = dossier / "recompenses_test_principal.txt";
    const std::filesystem::path fichierCorrompu = dossier / "recompenses_test_corrompu.txt";
    const std::filesystem::path fichierAbsent = dossier / "recompenses_test_absent.txt";

    std::error_code erreur;
    std::filesystem::remove(fichier, erreur);
    std::filesystem::remove(fichierCorrompu, erreur);
    std::filesystem::remove(fichierAbsent, erreur);

    std::cout << "=== Programmes ===\n";
    RewardsProgramService service(1, annuaire, fichier.string(), DATE);

    const int p1 = service.ajouterProgramme("Performance Q4", "Prime de performance",
                                            TypeProgramme::Performance, 100,
                                            "2026-10-01", "2026-12-31");
    verifier(p1 == 1, "creation d'un programme (id = 1)");

    verifierLeve([&] {
        service.ajouterProgramme("", "x", TypeProgramme::Performance, 10, "2026-01-01");
    }, "programme sans nom refuse");

    verifierLeve([&] {
        service.ajouterProgramme("X", "", TypeProgramme::Performance, 0, "2026-01-01");
    }, "points maximum nul refuse");

    verifierLeve([&] {
        service.ajouterProgramme("X", "", TypeProgramme::Performance, 10, "2026-13-01");
    }, "date de debut invalide refusee");

    verifierLeve([&] {
        service.ajouterProgramme("X", "", TypeProgramme::Performance, 10,
                                 "2026-12-31", "2026-01-01");
    }, "date de fin avant le debut refusee");

    const ProgrammeRecompense* programme1 = service.trouverProgramme(p1);
    verifier(programme1 != nullptr && programme1->estDisponible(DATE),
             "programme disponible dans sa periode");
    verifier(programme1 != nullptr && !programme1->estDisponible("2026-09-30"),
             "programme indisponible avant sa periode");
    verifier(programme1 != nullptr && !programme1->estDisponible("2027-01-01"),
             "programme indisponible apres sa periode");

    service.desactiverProgramme(p1);
    verifier(!service.trouverProgramme(p1)->estDisponible(DATE),
             "programme desactive indisponible");
    service.activerProgramme(p1);
    verifier(service.trouverProgramme(p1)->estDisponible(DATE),
             "programme reactive disponible");

    std::cout << "\n=== Catalogue ===\n";
    const int r1 = service.ajouterRecompense("Jour de conge bonus",
                                             "Une journee supplementaire", 80, 5);
    verifier(r1 == 1, "ajout d'une recompense (id = 1)");

    verifierLeve([&] {
        service.ajouterRecompense("X", "", 0, 1);
    }, "cout nul refuse");

    verifierLeve([&] {
        service.ajouterRecompense("X", "", 10, -1);
    }, "stock negatif refuse");

    verifier(service.trouverRecompense(r1)->estDisponible(),
             "recompense disponible en stock");

    std::cout << "\n=== Attribution de points ===\n";
    const int a1 = service.attribuerPoints(1, p1, 100, "Objectifs Q4 atteints");
    verifier(a1 == 1 && service.solde(1) == 100, "attribution de 100 points");

    verifierLeve([&] {
        service.attribuerPoints(99, p1, 10, "x");
    }, "employe inconnu refuse");

    verifierLeve([&] {
        service.attribuerPoints(1, 999, 10, "x");
    }, "programme inconnu refuse");

    const int p2 = service.ajouterProgramme("Fidelite 2026", "Anciennete",
                                            TypeProgramme::Fidelite, 50, "2026-01-01");
    service.desactiverProgramme(p2);
    verifierLeve([&] {
        service.attribuerPoints(2, p2, 10, "x");
    }, "attribution sur programme desactive refusee");

    verifierLeve([&] {
        service.attribuerPoints(1, p1, 0, "x");
    }, "points nuls refuses");

    verifierLeve([&] {
        service.attribuerPoints(1, p1, 101, "x");
    }, "points au-dela du maximum refuses");

    verifierLeve([&] {
        service.attribuerPoints(1, p1, 10, "x");
    }, "double attribution le meme jour refusee");

    const int a2 = service.attribuerPoints(2, p1, 50, "Objectifs Q4 atteints");
    verifier(a2 == 2 && service.solde(2) == 50, "autre employe accepte le meme jour");

    const int p3 = service.ajouterProgramme("Reconnaissance", "Coup de pouce",
                                            TypeProgramme::Reconnaissance, 20, "2026-01-01");
    const int a3 = service.attribuerPoints(1, p3, 10, "Aide a l'equipe");
    verifier(a3 == 3 && service.solde(1) == 110, "autre programme accepte le meme jour");

    verifier(service.solde(3) == 0, "solde nul sans historique");

    std::cout << "\n=== Utilisation des points ===\n";
    verifierLeve([&] {
        service.utiliserRecompense(2, r1);
    }, "solde insuffisant refuse");

    verifierLeve([&] {
        service.utiliserRecompense(1, 999);
    }, "recompense inconnue refusee");

    const int u1 = service.utiliserRecompense(1, r1);
    verifier(u1 == 1 && service.solde(1) == 30, "utilisation : 80 points deduits");
    verifier(service.trouverRecompense(r1)->getStock() == 4,
             "stock decremente apres utilisation");

    const int r2 = service.ajouterRecompense("Bon cadeau", "", 10, 0);
    verifierLeve([&] {
        service.utiliserRecompense(1, r2);
    }, "rupture de stock refusee");

    verifier(service.solde(1) >= 0, "solde jamais negatif");

    std::cout << "\n=== Historique et statistiques ===\n";
    const std::vector<AttributionRecompense> histo = service.historique(1);
    verifier(histo.size() == 2, "historique : 2 attributions pour l'employe 1");
    verifier(!histo.empty() && histo.front().date == DATE, "historique : date enregistree");

    const StatistiquesRecompenses stats = service.statistiques();
    verifier(stats.totalProgrammes == 3, "statistiques : 3 programmes");
    verifier(stats.totalAttributions == 3, "statistiques : 3 attributions");
    verifier(stats.pointsDistribues == 160, "statistiques : 160 points distribues");
    verifier(stats.pointsUtilises == 80, "statistiques : 80 points utilises");
    verifier(!stats.topEmployes.empty() && stats.topEmployes.front().first == 2,
             "statistiques : top employe = #2 (50 points nets)");

    std::cout << "\n=== Persistance ===\n";
    RewardsProgramService recharge(1, annuaire, fichier.string(), DATE);
    verifier(recharge.programmes().size() == 3, "persistance : programmes recharges");
    verifier(recharge.catalogue().size() == 2, "persistance : catalogue recharge");
    verifier(recharge.solde(1) == 30, "persistance : solde conserve");
    verifier(recharge.trouverRecompense(1)->getStock() == 4, "persistance : stock conserve");

    {
        std::ofstream flux(fichierCorrompu, std::ios::trunc);
        flux << "P;1;nom\n";
    }
    verifierLeve([&] {
        RewardsProgramService corrompu(1, annuaire, fichierCorrompu.string(), DATE);
        (void)corrompu;
    }, "fichier corrompu refuse");

    std::filesystem::remove(fichierAbsent, erreur);
    bool demarrageSansFichier = false;
    try {
        RewardsProgramService absent(1, annuaire, fichierAbsent.string(), DATE);
        demarrageSansFichier = absent.programmes().empty() && absent.catalogue().empty();
    } catch (const std::exception&) {
        demarrageSansFichier = false;
    }
    verifier(demarrageSansFichier, "fichier absent : service vide, sans exception");

    std::filesystem::remove(fichier, erreur);
    std::filesystem::remove(fichierCorrompu, erreur);
    std::filesystem::remove(fichierAbsent, erreur);

    std::cout << "\n==============================\n"
              << "Reussis : " << reussis << "   Echoues : " << echoues << "\n";

    return echoues == 0 ? 0 : 1;
}
