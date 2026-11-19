#pragma once
#include <vector>
#include "Departement.h"

// Contrat de persistance des departements (meme principe que pour les employes).
// Le gestionnaire de departements (Membre 3) ne depend que de cette interface.
class IPersistanceDepartements {
public:
    virtual ~IPersistanceDepartements() = default;
    virtual std::vector<Departement> lireDepartements() = 0;
    virtual void sauvegarderDepartements(const std::vector<Departement>& departements) = 0;
};
