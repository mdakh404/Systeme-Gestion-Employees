# Gestion des absences

Application console autonome en C++17 destinée au suivi des absences. Elle utilise la bibliothèque standard C++ et se compile indépendamment des autres modules.

## Responsabilités de Dev 3

Dev 3 prend en charge la saisie, la validation, la consultation et la persistance des absences. Le module conserve un identifiant employé et délègue la vérification d’existence au référentiel fourni par l’application. Il se compile indépendamment du module de Dev 1.

## Fonctionnalités

- Enregistrer une absence, consulter les absences actives ou afficher l’historique d’un employé.
- Rechercher une absence active ou annulée par son identifiant.
- Corriger une absence active, l’annuler en conservant sa trace ou supprimer définitivement une absence.
- Sauvegarder chaque création, modification, annulation ou suppression et recharger les données au démarrage.

## Règles métier

- L’identifiant employé doit être un entier strictement positif et reconnu par le vérificateur fourni à l’initialisation. L’ajout est refusé si le vérificateur est absent ou indisponible. Le menu refuse aussi une saisie non numérique. La démonstration simule uniquement les identifiants **1, 2, 15 et 22**.
- La date suit le format `JJ/MM/AAAA`. Les jours, les mois et les années bissextiles sont vérifiés selon le calendrier grégorien. Une absence future est refusée.
- La création et la modification sont autorisées jusqu’à 30 jours calendaires après la date de l’absence, d’après la date locale du système. Au-delà de 7 jours, l’absence doit être déclarée justifiée. Une justification déclarée indique un statut booléen et ne représente aucun document.
- Les seuils de 30 et 7 jours sont des choix de configuration RH pour ce projet universitaire. Les constantes `delaiEnregistrement` et `delaiJustification` dans `src/GestionnaireAbsences.cpp` centralisent leur configuration.
- Le motif doit être en UTF-8 valide et contenir entre 3 et 200 points de code après retrait des blancs Unicode aux extrémités. Un motif uniquement blanc, un point-virgule ou un saut de ligne est refusé à la saisie, en création comme en correction. Les accents et les espaces à l’intérieur du motif sont conservés.
- Un seul enregistrement actif est autorisé pour un employé à une date donnée, quel que soit le motif. Une correction conserve l’identifiant de l’absence et celui de l’employé. Elle applique les mêmes validations de date, de motif et de doublon que la création.
- L’annulation conserve l’enregistrement et son statut de justification. L’absence disparaît de la liste active, reste consultable dans l’historique avec le statut « Annulée » et libère la date pour un nouvel enregistrement. Une absence annulée est conservée telle quelle.
- La suppression définitive retire l’absence du fichier, des listes et de l’historique après confirmation. Elle est possible pour une absence active ou annulée.
- Les identifiants restent uniques. L’attribution reprend après le dernier identifiant attribué, même si cette absence a été supprimée ou si la collection est vide.
- Le menu applique la mise à jour en mémoire et confirme l’opération après la sauvegarde. Une erreur d’écriture ou un changement concurrent du fichier est signalé et la collection en mémoire reste inchangée.

## Architecture et structure

| Emplacement | Responsabilité |
|---|---|
| `include/Absence.h` et `src/Absence.cpp` | Modèle encapsulé et validation calendaire |
| `include/GestionnaireAbsences.h` et `src/GestionnaireAbsences.cpp` | Collection en mémoire, règles métier, identifiants et consultations |
| `include/GestionnaireFichiersAbsences.h` et `src/GestionnaireFichiersAbsences.cpp` | Lecture, contrôle du format et sauvegarde sûre |
| `include/MenuAbsences.h` et `src/MenuAbsences.cpp` | Saisie et affichage console |
| `src/Utf8.h` | Décodage UTF-8 utilisé pour la validation du motif et l’alignement du tableau |
| `src/main.cpp` | Chargement initial et coordination entre les gestionnaires et le menu |
| `data/absences.txt` | Données locales persistantes |
| `tests/test_absences.cpp` | Tests C++ des règles métier et de la persistance en répertoire temporaire |
| `tests/test_fichiers_absences.cpp` | Tests complémentaires de reprise et de fichiers corrompus |
| `CMakeLists.txt` et `README.md` | Configuration de compilation, CTest et documentation |

## Compilation et tests

Il faut CMake 3.20 ou plus récent et un compilateur C++17. Depuis le dossier `GestionAbsences` :

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build -C Debug --output-on-failure
```

Si vous sélectionnez une autre configuration avec un générateur multiconfiguration, utilisez le même nom pour `cmake --build build --config …` et `ctest -C …`.

Les tests sont conservés dans une sauvegarde séparée. La livraison sans `tests/` construit la bibliothèque et la démonstration. Pour relancer CTest, recopiez les tests dans un espace de développement puis reconfigurez CMake.

Les tests métier injectent une date fixe avec le constructeur `GestionnaireAbsences(dateReference)` pour vérifier les limites de 7 et 30 jours, le passage d’année et le 29 février. En production, le constructeur ordinaire laisse le gestionnaire consulter la date locale du système.

## Exécution

| Environnement | Depuis `GestionAbsences` | Depuis `build` |
|---|---|---|
| Windows, générateur monoconfiguration | `.\build\gestion_absences.exe` | `.\gestion_absences.exe` |
| Windows, Visual Studio en Debug | `.\build\Debug\gestion_absences.exe` | `.\Debug\gestion_absences.exe` |
| Linux ou macOS, générateur monoconfiguration | `./build/gestion_absences` | `./gestion_absences` |

La configuration CMake de la démonstration prépare le dossier `data/`. Sans argument, la démonstration utilise `data/absences.txt` dans le dossier source quel que soit le répertoire de lancement. Avec une livraison neuve, elle y démarre sur une collection vierge. Après déplacement du projet, configurez et compilez à nouveau. Si votre console Windows affiche mal les accents, activez UTF-8 avec `chcp 65001`.

Pour utiliser un fichier isolé, passez son chemin à l’exécutable de démonstration. La cible CMake réutilisable et le raccordement au menu RH sont décrits dans [INTEGRATION_DEV3.md](INTEGRATION_DEV3.md).

Pour une démonstration avec des données fictives, enregistrez l’exemple de format ci-dessous dans un dossier temporaire et passez le chemin de ce fichier à l’exécutable. Pour consulter les données locales sauvegardées hors de la livraison, passez explicitement le chemin de cette sauvegarde. Conservez son éventuel repère à côté du fichier principal.

## Exemple d’utilisation

1. Choisissez **1**, saisissez l’employé **15**, la date du jour au format `JJ/MM/AAAA`, le motif **Maladie**, puis **O**.
2. Notez l’identifiant affiché après la sauvegarde. Choisissez **2** pour voir le tableau complet.
3. Choisissez **3**, puis saisissez **15** pour voir son historique. Choisissez **4** pour retrouver l’absence par son identifiant.
4. Choisissez **5** pour corriger une absence. Laissez un champ vide avec **Entrée** pour conserver sa valeur, puis relisez le récapitulatif. Saisissez **O** pour confirmer ou **N** pour abandonner la correction.
5. Choisissez **6** pour annuler une absence tout en la conservant dans l’historique.
6. Choisissez **7** pour supprimer une absence. Vérifiez les informations affichées, puis saisissez **O** pour la retirer définitivement ou **N** pour conserver les données.
7. Choisissez **0** pour quitter le sous-menu. Dans la démonstration autonome, ce retour termine le programme. L’état sauvegardé sera disponible au prochain lancement.

## Format de `absences.txt`

Le fichier contient une absence par ligne selon le format `idAbsence;idEmploye;dateAbsence;motif;justifiee;annulee`. Les deux derniers champs valent `0` ou `1`. Une ligne historique à cinq champs est lue comme une absence active. La sauvegarde écrit les six champs. Lors d’une suppression, les lignes des autres absences sont conservées telles quelles.

```text
1;15;20/11/2026;Maladie;1;0
2;22;19/11/2026;Raison personnelle;0;1
```

Un fichier absent ou vide sans repère donne une collection vide. Une ligne invalide déclenche un diagnostic avec son numéro. Le chargement conserve les identifiants et les données historiques, même si une ancienne ligne dépasse aujourd’hui les délais ou ne remplit plus les nouvelles exigences de longueur ou d’encodage du motif.

Si le dernier identifiant attribué dépasse tous ceux des absences restantes, la première ligne contient une métadonnée telle que `#dernierId;8`. Elle reste présente si la collection est vide. Le compteur et les absences partagent le même fichier et sont remplacés ensemble lors de la sauvegarde. Les fichiers historiques sans cette ligne sont toujours lisibles.

Le fichier voisin `absences.txt.compteur` mémorise une borne déjà publiée. Si le fichier principal disparaît ou revient à un identifiant inférieur à cette borne, le chargement s’arrête avec une erreur et laisse les fichiers en place. Restaurer une copie cohérente du fichier principal avec un compteur au moins égal au repère permet de reprendre. Conservez les deux fichiers dans les sauvegardes et préservez le repère le plus récent lors d’une restauration. Le format d’`absences.txt` est inchangé et les fichiers historiques sans repère sont lisibles. Le repère est créé lors d’une sauvegarde.

Le repère est avancé après publication du fichier principal. Si cette étape échoue ou si le programme s’arrête entre les deux remplacements, le fichier principal reste la référence et une sauvegarde ultérieure rattrape le repère. Entre-temps, certaines anciennes versions peuvent ne pas être détectées. Le repère ne détecte pas un contenu restauré avec le même compteur et ne permet pas de reconstruire les lignes perdues. La perte simultanée des fichiers et de leurs sauvegardes rend la récupération du compteur impossible. Une coupure électrique n’est pas couverte par une garantie de durabilité.

## Limites du module autonome

La gestion des employés, des congés, des salaires, des départements et des demandes administratives est hors périmètre.

La sauvegarde utilise un fichier temporaire et refuse d’écraser une collection plus récente. Si un arrêt brutal laisse un dossier `.sauvegarde`, retirez ce dossier uniquement après avoir vérifié qu’aucune sauvegarde n’est en cours. L’alignement des motifs tient compte des accents français. La largeur d’affichage des emoji et des caractères à double largeur dépend du terminal.
