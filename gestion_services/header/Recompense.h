#ifndef RECOMPENSE_H
#define RECOMPENSE_H

#include <string>

// Article du catalogue : ce que les points permettent d'obtenir
// (jour de conge bonus, bon cadeau, formation...).
class Recompense {
public:
    Recompense() = default;

    Recompense(int id,
               std::string nom,
               std::string description,
               int coutPoints,
               int stock);

    int getId() const { return id_; }
    const std::string& getNom() const { return nom_; }
    const std::string& getDescription() const { return description_; }
    int getCoutPoints() const { return coutPoints_; }
    int getStock() const { return stock_; }

    void setNom(const std::string& nom);
    void setDescription(const std::string& description);
    void setCoutPoints(int coutPoints);
    void setStock(int stock);

    bool estDisponible() const { return stock_ > 0; }

    // Retire une unite du stock. Leve une exception si le stock est vide.
    void retirerDuStock();
    void reapprovisionner(int quantite);

    void afficher() const;

private:
    int id_ = 0;
    std::string nom_;
    std::string description_;
    int coutPoints_ = 0;
    int stock_ = 0;
};

#endif
