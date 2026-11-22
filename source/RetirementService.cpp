#include "RetirementService.h"
#include "GestionnaireEmployes.h"
#include <iostream>

RetirementService::RetirementService(
    int id,
    std::string name,
    std::string description,
    int retirementAge,
    int minimumYearsOfService,
    double pensionRate,
    double maximumPensionRate,
    GestionnaireEmployes* gestion
)
    : Service(id, name, description),
      retirementAge(retirementAge),
      minimumYearsOfService(minimumYearsOfService),
      pensionRate(pensionRate),
      maximumPensionRate(maximumPensionRate),
      gestion_(gestion)
{
}

// Getters
int RetirementService::getRetirementAge() const {
    return this->retirementAge;
}

int RetirementService::getMinimumYearsOfService() const {
    return this->minimumYearsOfService;
}

double RetirementService::getPensionRate() const {
    return this->pensionRate;
}

double RetirementService::getMaximumPensionRate() const {
    return this->maximumPensionRate;
}

// Setters
void RetirementService::setRetirementAge(int retirementAge) {
    this->retirementAge = retirementAge;
}

void RetirementService::setMinimumYearsOfService(int minimumYearsOfService) {
    this->minimumYearsOfService = minimumYearsOfService;
}

void RetirementService::setPensionRate(double pensionRate) {
    this->pensionRate = pensionRate;
}

void RetirementService::setMaximumPensionRate(double maximumPensionRate) {
    this->maximumPensionRate = maximumPensionRate;
}

// Calculate employee's age
int RetirementService::calculateAge(const Employe& e) const {
    return Date::anneesEntre(
        e.getDateNaissance(),
        Date::aujourdhui()
    );
}

// Check years of service
int RetirementService::calculateYearsOfService(const Employe& e) const {
    return e.anciennete();
}

// Check retirement eligibility
bool RetirementService::isEligibleForRetirement(const Employe& e) const {
    int age = this->calculateAge(e);
    int yearsOfService = this->calculateYearsOfService(e);

    return (age >= this->retirementAge) &&
           (yearsOfService >= this->minimumYearsOfService);
}

// Calculate theoretical retirement date
Date RetirementService::calculateRetirementDate(const Employe& e) const {
    const Date& dateNaissance = e.getDateNaissance();

    return Date(
        dateNaissance.jour,
        dateNaissance.mois,
        dateNaissance.annee + this->retirementAge
    );
}

// Estimate monthly pension
double RetirementService::estimateMonthlyPension(const Employe& e) const {
    const Salaire& salary = e.getSalaire();

    int yearsOfService = this->calculateYearsOfService(e);

    double pensionPercentage = yearsOfService * this->pensionRate;

    if (pensionPercentage > this->maximumPensionRate) {
        pensionPercentage = this->maximumPensionRate;
    }

    return salary.getSalaireBase() * pensionPercentage;
}

// Display service information
void RetirementService::display() const {
    std::cout << "\n========== " << name << " ==========\n";
    std::cout << "ID: " << id << '\n';
    std::cout << "Description: " << description << '\n';
    std::cout << "Retirement age: " << retirementAge << '\n';
    std::cout << "Minimum years of service: "
              << minimumYearsOfService << '\n';
    std::cout << "Pension rate: " << pensionRate * 100 << "%\n";
    std::cout << "Maximum pension rate: "
              << maximumPensionRate * 100 << "%\n";
}

// Execute service
void RetirementService::execute() {
    std::cout << "--- " << name << " ---\n";

    if (!gestion_) {
        std::cout << "Gestionnaire d'employes non configure.\n";
        return;
    }

    for (const Employe& employee : gestion_->lister()) {
        std::cout << employee.getId() << " - "
                  << employee.getPrenom() << ' '
                  << employee.getNom() << " ("
                  << employee.getMatricule() << ")\n";
    }

    std::cout << "Id de l'employe : ";
    int id;
    if (!(std::cin >> id)) return;

    const Employe* employee = gestion_->rechercher(id);

    if (employee == nullptr) {
        std::cout << "Employe non trouve.\n";
        return;
    }

    if (this->isEligibleForRetirement(*employee)) {
        Date retirementDate = this->calculateRetirementDate(*employee);
        double monthlyPension = this->estimateMonthlyPension(*employee);

        std::cout << "L'employe est eligible a la retraite.\n"
                  << "Date de retraite : "
                  << retirementDate.jour << '/'
                  << retirementDate.mois << '/'
                  << retirementDate.annee << '\n'
                  << "Estimation de la pension mensuelle : "
                  << monthlyPension << '\n';
    } else {
        std::cout << "L'employe n'est pas eligible a la retraite.\n";
    }
}