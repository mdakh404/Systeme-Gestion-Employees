#ifndef VALIDATIONTEXTE_H
#define VALIDATIONTEXTE_H

#include <stdexcept>
#include <string>

// Regles communes aux champs texte enregistres dans le fichier de donnees.
// Le point-virgule est le separateur du fichier : on l'interdit donc dans les
// valeurs, ainsi que les sauts de ligne.
namespace rewards {

inline bool contientSeparateur(const std::string& valeur) {
    return valeur.find(';') != std::string::npos ||
           valeur.find('\n') != std::string::npos ||
           valeur.find('\r') != std::string::npos;
}

inline void verifierTexteObligatoire(const std::string& valeur, const std::string& champ) {
    if (valeur.empty()) {
        throw std::invalid_argument(champ + " ne doit pas etre vide.");
    }
    if (contientSeparateur(valeur)) {
        throw std::invalid_argument(champ + " ne doit pas contenir ';' ni de saut de ligne.");
    }
    if (valeur.size() > 200) {
        throw std::invalid_argument(champ + " est trop long (200 caracteres maximum).");
    }
}

inline void verifierTexteOptionnel(const std::string& valeur, const std::string& champ) {
    if (contientSeparateur(valeur)) {
        throw std::invalid_argument(champ + " ne doit pas contenir ';' ni de saut de ligne.");
    }
    if (valeur.size() > 200) {
        throw std::invalid_argument(champ + " est trop long (200 caracteres maximum).");
    }
}

} // namespace rewards

#endif
