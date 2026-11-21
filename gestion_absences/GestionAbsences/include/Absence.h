#ifndef GESTION_ABSENCES_ABSENCE_H
#define GESTION_ABSENCES_ABSENCE_H

#include <string>

class Absence {
public:
    Absence(int id, int idEmploye, const std::string& dateAbsence,
            const std::string& motif, bool justifiee, bool annulee = false);

    int getId() const;
    int getIdEmploye() const;
    const std::string& getDateAbsence() const;
    const std::string& getMotif() const;
    bool estJustifiee() const;

    void setDateAbsence(const std::string& dateAbsence);
    void setMotif(const std::string& motif);
    void setJustifiee(bool justifiee);

    bool estAnnulee() const;
    void annuler();
    void afficher() const;

private:
    friend class GestionnaireAbsences;
    static int numeroJour(const std::string& date);
    int id;
    int idEmploye;
    std::string dateAbsence;
    std::string motif;
    bool justifiee;
    bool annulee;
};

#endif
