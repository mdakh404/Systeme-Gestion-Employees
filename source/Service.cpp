#include "Service.h"

Service::Service(int id, std::string name, std::string description) {
    this->id = id;
    this->name = name;
    this->description = description;
}

int Service::getServiceID() {
    return this->id;
}

std::string Service::getServiceName() {
    return this->name;
}

std::string Service::getServiceDescription() {
    return this->description;
}

void Service::setServiceID(int id) {
    this->id = id;
}

void Service::setServiceName(std::string name) {
    this->name = name;
}

void Service::setServiceDescription(std::string description) {
    this->description = description;
}