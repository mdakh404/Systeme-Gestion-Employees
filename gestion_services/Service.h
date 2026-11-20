#ifndef SERVICE_H
#define SERVICE_H
#include <string>

class Service {
    protected:
        int id;
        std::string name;
        std::string description;
    
    public:
        Service(int id, std::string name, std::string description);
        int getServiceID();
        std::string getServiceName();
        std::string getServiceDescription();
        void setServiceID(int id);
        void setServiceName(std::string name);
        void setServiceDescription(std::string description);
        virtual void display() const = 0;
        virtual void execute() = 0;
        virtual ~Service() = default;
};

#endif