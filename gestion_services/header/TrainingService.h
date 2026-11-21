#ifndef TRAININGSERVICE_H
#define TRAININGSERVICE_H

#include "Service.h"
#include <string>
#include <vector>
#include <map>

// Une formation : cours ou abonnement
struct Training {
    int id;
    std::string title;
    std::string type;       
    double price;
    int durationHours;
    std::string startDate;  
};

class TrainingService : public Service {
    private:
        std::vector<Training> trainings;
        std::map<int, std::vector<int>> enrollments; 
        int nextTrainingId;

        std::vector<Training>::const_iterator findTraining(int trainingId) const;

    public:
        TrainingService(int id = 0);
        ~TrainingService() override = default;

        // Catalogue
        void addTraining(const std::string& title, const std::string& type,
                         double price, int durationHours, const std::string& startDate);
        void removeTraining(int trainingId);
        std::vector<Training> searchTraining(const std::string& keyword) const;
        void listTrainings() const;

        // Inscriptions des employes
        void enrollEmployee(int employeeId, int trainingId);
        void unenrollEmployee(int employeeId, int trainingId);
        void showEmployeeTrainings(int employeeId) const;
        double getTotalCost() const;

        // Persistance
        void saveToFile(const std::string& filename) const;
        void loadFromFile(const std::string& filename);

        // Redefinitions de Service
        void display() const override;
        void execute() override; 
};

#endif
