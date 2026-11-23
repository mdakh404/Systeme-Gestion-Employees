#include "TrainingService.h"
#include "DbPersistence.h"
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <limits>

TrainingService::TrainingService(int id)
    : Service(id, "Training Service",
              "Professional training: subscriptions and courses for employees"),
      nextTrainingId(1) {}

std::vector<Training>::const_iterator TrainingService::findTraining(int trainingId) const {
    return std::find_if(trainings.begin(), trainings.end(),
        [trainingId](const Training& t) { return t.id == trainingId; });
}

void TrainingService::addTraining(const std::string& title, const std::string& type,
                                  double price, int durationHours, const std::string& startDate) {
    if (title.empty()) throw std::invalid_argument("Title is empty");
    if (type != "Course" && type != "Subscription")
        throw std::invalid_argument("Type must be Course or Subscription");
    if (price < 0 || durationHours <= 0)
        throw std::invalid_argument("Invalid price or duration");
    trainings.push_back({nextTrainingId++, title, type, price, durationHours, startDate});
}

void TrainingService::removeTraining(int trainingId) {
    auto it = findTraining(trainingId);
    if (it == trainings.end()) throw std::runtime_error("Training not found");
    trainings.erase(trainings.begin() + (it - trainings.begin()));
    for (auto& entry : enrollments) {
        auto& v = entry.second;
        v.erase(std::remove(v.begin(), v.end(), trainingId), v.end());
    }
}

std::vector<Training> TrainingService::searchTraining(const std::string& keyword) const {
    std::vector<Training> result;
    std::copy_if(trainings.begin(), trainings.end(), std::back_inserter(result),
        [&keyword](const Training& t) { return t.title.find(keyword) != std::string::npos; });
    return result;
}

void TrainingService::listTrainings() const {
    if (trainings.empty()) { std::cout << "No training available.\n"; return; }
    for (const auto& t : trainings)
        std::cout << "[" << t.id << "] " << t.title << " (" << t.type << ") - "
                  << t.price << " MAD, " << t.durationHours << "h, start " << t.startDate << "\n";
}

void TrainingService::enrollEmployee(int employeeId, int trainingId) {
    if (findTraining(trainingId) == trainings.end())
        throw std::runtime_error("Training not found");
    auto& v = enrollments[employeeId];
    if (std::find(v.begin(), v.end(), trainingId) != v.end())
        throw std::runtime_error("Employee already enrolled");
    v.push_back(trainingId);
}

void TrainingService::unenrollEmployee(int employeeId, int trainingId) {
    auto it = enrollments.find(employeeId);
    if (it == enrollments.end()) throw std::runtime_error("No enrollment for this employee");
    auto& v = it->second;
    auto pos = std::find(v.begin(), v.end(), trainingId);
    if (pos == v.end()) throw std::runtime_error("Enrollment not found");
    v.erase(pos);
}

void TrainingService::showEmployeeTrainings(int employeeId) const {
    auto it = enrollments.find(employeeId);
    if (it == enrollments.end() || it->second.empty()) {
        std::cout << "No training for employee " << employeeId << ".\n"; return;
    }
    for (int tid : it->second) {
        auto t = findTraining(tid);
        if (t != trainings.end())
            std::cout << "- " << t->title << " (" << t->type << ", " << t->price << " MAD)\n";
    }
}

double TrainingService::getTotalCost() const {
    double total = 0;
    for (const auto& entry : enrollments)
        for (int tid : entry.second) {
            auto t = findTraining(tid);
            if (t != trainings.end()) total += t->price;
        }
    return total;
}


void TrainingService::saveToFile(const std::string& filename) const {
    if (Database::instance().estConnectee()) {   // mode MySQL
        Database& base = Database::instance();
        Transaction tx(base);
        base.executer("DELETE FROM inscription_formation");
        base.executer("DELETE FROM formation");
        for (const auto& t : trainings) {
            std::ostringstream prix;
            prix << std::fixed << std::setprecision(2) << t.price;
            base.executer("INSERT INTO formation (id, titre, type_formation, prix, duree_heures, "
                          "date_debut) VALUES (" + std::to_string(t.id) + ", " + db::q(t.title) + ", "
                          + db::q(t.type) + ", " + prix.str() + ", " + std::to_string(t.durationHours)
                          + ", " + db::q(t.startDate) + ")");
        }
        // Les inscriptions d'employes absents de la base sont ignorees (cle etrangere).
        for (const auto& entry : enrollments)
            for (int tid : entry.second)
                base.executer("INSERT IGNORE INTO inscription_formation (employe_id, formation_id) "
                              "SELECT id, " + std::to_string(tid) + " FROM employe WHERE id = "
                              + std::to_string(entry.first));
        tx.commit();
        return;
    }
    std::ofstream out(filename);
    if (!out) throw std::runtime_error("Cannot write " + filename);
    for (const auto& t : trainings)
        out << "T;" << t.id << ";" << t.title << ";" << t.type << ";"
            << t.price << ";" << t.durationHours << ";" << t.startDate << "\n";
    for (const auto& entry : enrollments)
        for (int tid : entry.second) out << "E;" << entry.first << ";" << tid << "\n";
}

void TrainingService::loadFromFile(const std::string& filename) {
    if (Database::instance().estConnectee()) {   // mode MySQL
        Database& base = Database::instance();
        auto formations = base.requete(
            "SELECT id, titre, type_formation, prix, duree_heures, date_debut FROM formation "
            "ORDER BY id");
        auto inscriptions = base.requete(
            "SELECT employe_id, formation_id FROM inscription_formation "
            "ORDER BY employe_id, formation_id");
        trainings.clear(); enrollments.clear(); nextTrainingId = 1;
        for (const auto& l : formations) {
            Training t{std::stoi(*l[0]), *l[1], *l[2], std::stod(*l[3]), std::stoi(*l[4]),
                       l[5] ? *l[5] : std::string()};
            trainings.push_back(t);
            nextTrainingId = std::max(nextTrainingId, t.id + 1);
        }
        for (const auto& l : inscriptions) enrollments[std::stoi(*l[0])].push_back(std::stoi(*l[1]));
        return;
    }
    std::ifstream in(filename);
    if (!in) return; 
    trainings.clear(); enrollments.clear(); nextTrainingId = 1;
    std::string line;
    while (std::getline(in, line)) {
        std::stringstream ss(line);
        std::string tag, a, b, c, d, e, f;
        std::getline(ss, tag, ';');
        if (tag == "T") {
            std::getline(ss, a, ';'); std::getline(ss, b, ';'); std::getline(ss, c, ';');
            std::getline(ss, d, ';'); std::getline(ss, e, ';'); std::getline(ss, f, ';');
            Training t{std::stoi(a), b, c, std::stod(d), std::stoi(e), f};
            trainings.push_back(t);
            nextTrainingId = std::max(nextTrainingId, t.id + 1);
        } else if (tag == "E") {
            std::getline(ss, a, ';'); std::getline(ss, b, ';');
            enrollments[std::stoi(a)].push_back(std::stoi(b));
        }
    }
}

void TrainingService::display() const {
    std::cout << "=== " << name << " (ID " << id << ") ===\n"
              << description << "\n"
              << "Trainings: " << trainings.size() << "\n";
    listTrainings();
}

void TrainingService::execute() {
    int choice = -1;
    while (choice != 0) {
        std::cout << "\n--- Training Service ---\n"
                  << "1. List trainings\n2. Add training\n3. Remove training\n"
                  << "4. Search training\n5. Enroll employee\n6. Unenroll employee\n"
                  << "7. Show employee trainings\n8. Total cost\n0. Back\nChoice: ";
        if (!(std::cin >> choice)) { std::cin.clear(); std::cin.ignore(10000, '\n'); choice = -1; continue; }
        std::cin.ignore(10000, '\n');
        try {
            if (choice == 1) listTrainings();
            else if (choice == 2) {
                std::string title, type, date; double price; int hours;
                std::cout << "Title: "; std::getline(std::cin, title);
                std::cout << "Type (Course/Subscription): "; std::getline(std::cin, type);
                std::cout << "Price: "; std::cin >> price;
                std::cout << "Duration (hours): "; std::cin >> hours; std::cin.ignore(10000, '\n');
                std::cout << "Start date (YYYY-MM-DD): "; std::getline(std::cin, date);
                addTraining(title, type, price, hours, date);
                std::cout << "Training added.\n";
            } else if (choice == 3) {
                int tid; std::cout << "Training ID: "; std::cin >> tid; std::cin.ignore(10000, '\n');
                removeTraining(tid); std::cout << "Training removed.\n";
            } else if (choice == 4) {
                std::string k; std::cout << "Keyword: "; std::getline(std::cin, k);
                auto res = searchTraining(k);
                if (res.empty()) std::cout << "No result.\n";
                for (const auto& t : res) std::cout << "[" << t.id << "] " << t.title << "\n";
            } else if (choice == 5 || choice == 6) {
                int eid, tid;
                std::cout << "Employee ID: "; std::cin >> eid;
                std::cout << "Training ID: "; std::cin >> tid; std::cin.ignore(10000, '\n');
                if (choice == 5) { enrollEmployee(eid, tid); std::cout << "Enrolled.\n"; }
                else { unenrollEmployee(eid, tid); std::cout << "Unenrolled.\n"; }
            } else if (choice == 7) {
                int eid; std::cout << "Employee ID: "; std::cin >> eid; std::cin.ignore(10000, '\n');
                showEmployeeTrainings(eid);
            } else if (choice == 8) {
                std::cout << "Total cost: " << getTotalCost() << " MAD\n";
            } else if (choice != 0) std::cout << "Invalid choice.\n";
        } catch (const std::exception& ex) {
            std::cout << "Error: " << ex.what() << "\n";
        }
    }
}
