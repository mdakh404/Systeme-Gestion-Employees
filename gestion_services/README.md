# Rewards Program Service

Service de **programmes de recompenses** : le RH cree des programmes, attribue
des points aux employes, et les employes echange ces points contre des articles
d'un catalogue.

Ce service est **distinct** de `PrimeService` (bonification des primes) : il ne
calcule pas de montant en dirhams, il tient un registre de points.

## Contenu

| Fichier | Role |
|---|---|
| `header/RewardsProgramService.h` / `source/RewardsProgramService.cpp` | Le service : programmes, catalogue, attributions, utilisations, statistiques, persistance |
| `header/ProgrammeRecompense.h` / `source/ProgrammeRecompense.cpp` | Un programme : type, points maximum par attribution, periode de validite |
| `header/Recompense.h` / `source/Recompense.cpp` | Un article du catalogue : cout en points, stock |
| `header/EmployeInfo.h` | Vue minimale d'un employe + `FournisseurEmployes` |
| `header/DateIso.h` | Validation et comparaison de dates `AAAA-MM-JJ` |
| `header/ValidationTexte.h` | Regles communes sur les champs texte |
| `tests/test_rewards.cpp` | Tests automatiques, sans framework |
| `demo/main.cpp` | Demonstration autonome (menu console) |
| `CMakeLists.txt` | Bibliotheque + tests + demo |

## API publique

| Methode | Role | Entree | Sortie |
|---|---|---|---|
| `RewardsProgramService(id, fournisseur, fichier, dateReference)` | Initialise le service et charge le fichier | `int`, `FournisseurEmployes`, chemin, date `AAAA-MM-JJ` (vide = date systeme) | instance |
| `ajouterProgramme` | Cree un programme | nom, description, `TypeProgramme`, points max, debut, fin | `int` id |
| `modifierProgramme` | Remplace les reglages d'un programme | id + memes champs | `void` |
| `desactiverProgramme` / `activerProgramme` | Active ou desactive un programme | id | `void` |
| `programmes` / `trouverProgramme` | Consulte les programmes | — / id | `vector` / `const ProgrammeRecompense*` |
| `ajouterRecompense` | Ajoute un article au catalogue | nom, description, cout, stock | `int` id |
| `modifierStock` | Fixe le stock d'un article | id, nouveau stock | `void` |
| `catalogue` / `trouverRecompense` | Consulte le catalogue | — / id | `vector` / `const Recompense*` |
| `attribuerPoints` | Attribue des points a un employe | idEmploye, idProgramme, points, motif | `int` id attribution |
| `utiliserRecompense` | Echange des points contre un article | idEmploye, idRecompense | `int` id utilisation |
| `solde` | Points disponibles d'un employe | idEmploye | `int` |
| `historique` / `utilisations` | Registre d'un employe | idEmploye | `vector` |
| `statistiques` / `afficherStatistiques` | Totaux et classement | — | `StatistiquesRecompenses` / console |
| `charger` / `sauvegarder` | Persistance | — | `void` |
| `display` / `execute` | Interface `Service` | — | console |

## Regles metier

- Un programme est **disponible** si son drapeau est actif **et** si la date du
  jour tombe dans `[dateDebut, dateFin]`. Une date de fin vide signifie
  « pas de limite ».
- `attribuerPoints` exige : employe existant, programme actif et disponible,
  `0 < points <= pointsMaxParAttribution`, motif non vide.
- **Une seule attribution par employe et par programme et par jour** (evite les
  doubles saisies). Deux programmes differents le meme jour restent possibles.
- `utiliserRecompense` exige : stock disponible et `solde >= coutPoints`.
- Le solde est toujours `somme(attributions) - somme(utilisations)` et ne peut
  jamais devenir negatif.
- Les points sont des entiers : aucune erreur d'arrondi.
- Le registre est en **ajout seul** : une attribution ou une utilisation n'est
  jamais modifiee ni supprimee.

## Compilation et tests

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Sans CMake, depuis `gestion_services/` :

```sh
g++ -std=c++17 -Wall -Wextra -pedantic -Iheader \
    source/Service.cpp source/ProgrammeRecompense.cpp source/Recompense.cpp \
    source/RewardsProgramService.cpp tests/test_rewards.cpp -o test_rewards
./test_rewards
```

## Format de `data/recompenses.txt`

Une ligne par enregistrement, champs separes par `;`. Les lignes commencant par
`#` sont des commentaires.

```text
#recompenses v1
P;id;nom;description;type;pointsMax;dateDebut;dateFin;actif
R;id;nom;description;coutPoints;stock
A;id;idEmploye;idProgramme;points;motif;date
U;id;idEmploye;idRecompense;points;date
C;prochainIdProgramme;prochainIdRecompense;prochainIdAttribution;prochainIdUtilisation
```

Les champs texte ne peuvent pas contenir `;` ni de saut de ligne. Le fichier est
ecrit dans un fichier temporaire puis renomme, afin de ne pas le corrompre en cas
d'interruption. Un fichier absent demarre sur une collection vide ; une ligne
invalide leve une exception indiquant son numero.

## Integration au module Employes

Le service ne depend pas de la classe `Employe`. Il recoit un
`FournisseurEmployes` (voir `EmployeInfo.h`) :

```cpp
#include "RewardsProgramService.h"

RewardsProgramService recompenses(
    1,
    [&employes](int id) -> std::optional<EmployeInfo> {
        const Employe* e = employes.rechercher(id);
        if (e == nullptr) return std::nullopt;
        return EmployeInfo{e->getId(), e->getMatricule(), e->getNom(), e->getPrenom()};
    },
    "data/recompenses.txt");
```

C'est le meme principe que le module Absences (`std::function<bool(int)>`) : le
service compile et se teste seul, et le raccordement se fait en une ligne.

## Limites

- La persistance est un fichier texte ; le passage a MySQL se fait en remplacant
  `charger()` / `sauvegarder()` par un depot base sur le schema
  `docs/gestion_employes.sql`.
- Ce module ne compile pas les autres services : `PrimeService`,
  `RetirementService` et `TrainingService` dependent du module Employes.
