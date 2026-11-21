#ifndef RETIREMENT_SERVICE_H
#define RETIREMENT_SERVICE_H

#include <string>
#include "Service.h"
#include "../../gestion-des-employ-s/headers/Employe.h"
#include "../../gestion-des-employ-s/headers/Date.h"

class RetirementService : public Service {
    private:
        int retirementAge;
        int minimumYearsOfService;
        double pensionRate;
        double maximumPensionRate;

    public:
        RetirementService(
            int id,
            std::string name,
            std::string description,
            int retirementAge = 60,
            int minimumYearsOfService = 15,
            double pensionRate = 0.02,
            double maximumPensionRate = 0.80
        );

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
