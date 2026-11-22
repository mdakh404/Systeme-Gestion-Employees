#include "RewardsProgramService.h"

#include <exception>
#include <iostream>
#include <optional>
#include <string>

namespace {

// Annuaire fictif pour la demonstration autonome.
std::optional<EmployeInfo> annuaireDemo(int id) {
    if (id == 1) return EmployeInfo{1, "EMP0001", "Alaoui", "Karim"};
    if (id == 2) return EmployeInfo{2, "EMP0002", "Benani", "Salma"};
    if (id == 3) return EmployeInfo{3, "EMP0003", "El Idrissi", "Youssef"};
    return std::nullopt;
}

} // namespace

int main(int argc, char** argv) {
    const std::string chemin = argc > 1 ? argv[1] : "data/recompenses.txt";

    try {
        RewardsProgramService service(1, annuaireDemo, chemin);
        std::cout << "Fichier de donnees : " << chemin << "\n";
        service.display();
        service.execute();
        return 0;
    } catch (const std::exception& erreur) {
        std::cerr << "Erreur fatale : " << erreur.what() << "\n";
        return 1;
    }
}
