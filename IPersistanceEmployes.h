#pragma once
#include <vector>
#include "Employe.h"

// Contrat (classe abstraite) : GestionnaireEmployes ne depend que de cette
// interface. L'equipe Persistance peut changer fichiers -> base de donnees
// sans toucher a la logique metier.
class IPersistanceEmployes {
public:
    virtual ~IPersistanceEmployes() = default;
    virtual std::vector<Employe> lireEmployes() = 0;
    virtual void sauvegarderEmployes(const std::vector<Employe>& employes) = 0;
};
