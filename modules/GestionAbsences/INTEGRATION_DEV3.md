# Intégration de GestionAbsences — Dev 3

GestionAbsences est une bibliothèque C++17 autonome. Dev 3 gère les absences, leurs règles, leur historique et leur stockage. L’application RH assemble les modules et conserve le point d’entrée commun.

## API publique et fichiers

| Classe | Contrat |
|---|---|
| `Absence` | Objet d’absence avec deux identifiants entiers, date `JJ/MM/AAAA`, motif, justification et annulation. `idEmploye` est l’unique lien avec l’annuaire. |
| `GestionnaireAbsences` | Enregistre, modifie, annule, supprime et consulte. Constructeur avec `VerificationEmploye = std::function<bool(int)>`. Un second constructeur avec date fixe et vérificateur est disponible pour les tests. |
| `GestionnaireFichiersAbsences` | Prend un chemin de stockage. `chargerAbsences(vérificateur)` restitue la collection et configure le contrôle des futurs ajouts. Fournit aussi les opérations de sauvegarde. |
| `MenuAbsences` | `MenuAbsences(gestionnaire, stockage)` relie les opérations à la sauvegarde. `executer() const` exécute le sous-menu. Le choix **0** retourne à l’appelant. Le constructeur à callbacks reste disponible pour injecter un autre flux de commande. |

Intégrer le dossier du module via son `CMakeLists.txt`, ses en-têtes publics dans `include/` et les quatre fichiers d’implémentation `src/Absence.cpp`, `src/GestionnaireAbsences.cpp`, `src/GestionnaireFichiersAbsences.cpp` et `src/MenuAbsences.cpp`. Conserver l’en-tête interne `src/Utf8.h` avec les sources. L’ensemble requiert un compilateur C++17, la bibliothèque standard et CMake 3.20 ou plus récent.

`src/main.cpp` est réservé à la démonstration autonome. Les sources sous `tests/`, les dossiers de construction et les données de démonstration restent hors de l’exécutable RH final. Prévoir séparément le transfert des données réelles si l’équipe souhaite les conserver.

## Connexion du référentiel des employés

Le vérificateur reçoit un `int` strictement positif et retourne `true` uniquement pour un employé reconnu par le référentiel. `false` signifie que l’employé est introuvable. Une panne doit provoquer une exception. Le métier rejette un identifiant non positif avant d’appeler le vérificateur, rejette un résultat négatif et bloque l’ajout si le vérificateur est absent ou lève une exception. Le détail d’une exception standard est conservé dans le diagnostic. `enregistrerAbsence` retourne `false` pour un doublon d’absence et lève une exception pour les autres refus.

L’adaptateur doit capturer la même instance logique du gestionnaire d’employés que celle utilisée dans le menu RH. Utiliser une capture par référence si l’annuaire vit plus longtemps que le gestionnaire d’absences, ou une propriété partagée adaptée au cycle de vie. Les copies transactionnelles du gestionnaire d’absences copient le callback. Éviter un annuaire mutable capturé par valeur. Dev 1 doit confirmer que ses identifiants sont compatibles avec `int`, que `rechercher(int)` distingue l’absence de résultat et que ses erreurs remontent. Si sa recherche retourne un pointeur ou un `std::optional`, convertir le résultat en booléen. Adapter cette conversion au contrat définitif.

Le chargement historique, les consultations et la modification d’une absence existante ne demandent pas de relier à nouveau l’employé. La modification conserve son identifiant. Pour un employé supprimé ou archivé, conserver ses anciennes absences et considérer l’identifiant comme indisponible pour les nouvelles absences selon la politique de Dev 1. L’identifiant employé ne doit pas être réattribué à une autre personne si l’historique doit rester interprétable. L’équipe RH décide si l’archivage remplace la suppression et comment afficher un employé archivé dans le menu général.

## CMake dans le projet RH

```cmake
cmake_minimum_required(VERSION 3.20)
project(ApplicationRH LANGUAGES CXX)

add_subdirectory(modules/GestionAbsences)
add_executable(application_rh src/main.cpp)
target_link_libraries(application_rh PRIVATE GestionAbsences::module)
```

La cible `GestionAbsences::module` est un alias de `gestion_absences_lib`. Elle publie ses en-têtes et son exigence C++17. Elle contient le menu et les quatre implémentations, sans `main.cpp`. Par défaut, une construction autonome ajoute la démonstration et les tests. Comme sous-projet, seules les sources de bibliothèque sont construites par défaut. Les options `GESTION_ABSENCES_BUILD_DEMO` et `GESTION_ABSENCES_BUILD_TESTS` permettent de choisir explicitement. Si le projet RH active les tests, activer aussi CTest à la racine du projet RH.

Utiliser un nom de cible propre à chaque module et un fichier de stockage propre à chaque domaine. Ne pas inclure de fichiers `.cpp` dans les consommateurs. N’ajouter au point d’entrée commun que les adaptateurs et la navigation. Contrôler au moment de la fusion les collisions éventuelles de noms C++ dans l’espace global et les noms de cibles. Aucun code des autres modules n’est nécessaire pour compiler Dev 3.

## Exemple de main.cpp commun

Exemple d’intégration future. Les deux en-têtes Dev 1 et configuration RH, les objets fournis par l’hôte et le type retourné par `rechercher` sont à adapter après accord de l’équipe.

```cpp
#include "GestionnaireFichiersAbsences.h"
#include "MenuAbsences.h"
#include "GestionnaireEmployes.h" // Fourni et validé par Dev 1
#include "ContexteRH.h"            // Fourni par l’application commune

#include <exception>
#include <iostream>
#include <limits>

int main() {
    try {
        ContexteRH rh;
        auto& annuaire = rh.gestionnaireEmployes();
        const auto chemin = rh.dossierDonnees() / "absences.txt";
        const GestionnaireFichiersAbsences stockage(chemin.string());
        auto absences = stockage.chargerAbsences([&annuaire](int id) {
            return static_cast<bool>(annuaire.rechercher(id));
        });
        const MenuAbsences menuAbsences(absences, stockage);

        std::cout << "1. Absences\n0. Quitter\n";
        int choix;
        while (std::cin >> choix && choix != 0) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (choix == 1) {
                menuAbsences.executer();
            }
            std::cout << "1. Absences\n0. Quitter\n";
        }
        return 0;
    } catch (const std::exception& erreur) {
        std::cerr << erreur.what() << '\n';
        return 1;
    }
}
```

Le gestionnaire, le stockage et l’annuaire doivent vivre jusqu’au retour du sous-menu. L’application commune choisit la présentation des erreurs de chargement et la navigation générale.

## Chemin, dates et migration finale

Le chemin transmis à `GestionnaireFichiersAbsences` cible le fichier d’absences dans le dossier commun choisi par l’hôte. Créer le répertoire parent avant la première écriture et s’assurer qu’il est accessible. Un chemin relatif est résolu à partir du dossier de travail. La démonstration autonome conserve son chemin de projet par défaut et accepte en argument un autre fichier pour une exécution isolée.

Lors du transfert, conserver ensemble le fichier principal `absences.txt` et son repère `absences.txt.compteur` s’il existe. Préserver les identifiants, les statuts, l’en-tête facultatif `#dernierId;N` et les lignes historiques. Ne pas réinitialiser le repère pour contourner une restauration incohérente. Les chemins d’absences, de congés, de services et de demandes doivent viser des fichiers distincts.

Le contrat actuel reste une chaîne grégorienne `JJ/MM/AAAA` avec zéros initiaux. Si l’équipe choisit une classe Date commune, placer dans l’application ou l’adaptateur une conversion vers cette chaîne, et éventuellement la conversion inverse pour l’affichage commun. Dev 3 peut continuer à contrôler le format et les règles calendaires sans dépendre d’un en-tête Date externe.

## Vérifications d’intégrabilité avec la sauvegarde des tests

```sh
cmake -S . -B build-integration
cmake --build build-integration
ctest --test-dir build-integration --output-on-failure
cmake -S tests/integration -B build-hote
cmake --build build-hote
```

Ces commandes de test s’appliquent à une copie de développement dans laquelle le dossier `tests/` a été restauré. Le projet de test `tests/integration` possède son propre point d’entrée, ajoute Dev 3 avec `add_subdirectory` et se lie uniquement à `GestionAbsences::module`. Fournir à `hote_absences` un chemin dans un dossier temporaire. Son annuaire de test reconnaît l’identifiant 901. Sans les sources de test, CMake construit la bibliothèque et, en mode autonome, la démonstration.
