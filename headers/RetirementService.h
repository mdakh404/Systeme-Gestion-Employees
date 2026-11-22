#ifndef RETIREMENT_SERVICE_H
#define RETIREMENT_SERVICE_H

#include <string>
#include "Service.h"
#include "Employe.h"
#include "Date.h"

class GestionnaireEmployes;

class RetirementService : public Service {
    private:
        int retirementAge;
        int minimumYearsOfService;
        double pensionRate;
        double maximumPensionRate;
        GestionnaireEmployes* gestion_ = nullptr;

    public:
        RetirementService(
            int id,
            std::string name,
            std::string description,
            int retirementAge = 60,
            int minimumYearsOfService = 15,
            double pensionRate = 0.02,
            double maximumPensionRate = 0.80,
            GestionnaireEmployes* gestion = nullptr
        );

    void setGestionnaire(GestionnaireEmployes* g) { gestion_ = g; }

    // Getters
    int getRetirementAge() const;
    int getMinimumYearsOfService() const;
    double getPensionRate() const;
    double getMaximumPensionRate() const;

    // Setters
    void setRetirementAge(int retirementAge);
    void setMinimumYearsOfService(int minimumYearsOfService);
    void setPensionRate(double pensionRate);
    void setMaximumPensionRate(double maximumPensionRate);

    // Retirement calculations
    int calculateAge(const Employe& e) const;
    int calculateYearsOfService(const Employe& e) const;

    bool isEligibleForRetirement(const Employe& e) const;

    Date calculateRetirementDate(const Employe& e) const;

    double estimateMonthlyPension(const Employe& e) const;

    // Service interface
    void display() const override;
    void execute() override;
};

#endif
