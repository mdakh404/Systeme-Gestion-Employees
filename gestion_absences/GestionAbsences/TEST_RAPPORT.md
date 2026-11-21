# Documentation des Tests: "Module de Gestion des Absences"


---

## Introduction

Ce rapport présente les protocoles de compilation, d'exécution et de validation réalisés sur le module C++ de **Gestion des Absences**. L'objectif de cette campagne est de garantir le bon fonctionnement de l'ensemble des opérations CRUD (Création, Lecture, Mise à jour, Suppression), le respect des règles métier (validation des entrées, contrôle des dates futures ou invalides, détection des doublons) et la persistance des données sur fichier.

La démarche de validation s'articule autour de deux axes complémentaires :
1. **Validation fonctionnelle manuelle en terminal :** simulation des cas nominaux et d'erreur via le menu interactif CLI.
2. **Tests unitaires automatisés avec GoogleTest :** exécution automatisée de 11 cas de test unitaires et d'intégration couvrant la logique métier et la persistance.

---

## 1. Tests Manuels dans le Terminal

### 1.1 Configuration de l'environnement et compilation

Les manipulations ont été exécutées sous **Fedora Linux 44**. Après vérification de la présence de `g++` et installation du gestionnaire de build `CMake`, le projet a été configuré et compilé dans le dossier `gestion_absences/GestionAbsences`.

```bash
# 1. Installation des dépendances système (Fedora 44)
sudo dnf install gcc-c++ cmake make

# 2. Vérification des versions installées
g++ --version     # g++ (GCC) 16.1.1 20260515
cmake --version   # cmake version 4.3.0

# 3. Compilation du projet
cd gestion_absences/GestionAbsences
cmake -S . -B build
cmake --build build -j2

# 4. Lancement de l'application interactive
./build/gestion_absences
```

**Résultat de la compilation :**  
Génération sans avertissement ni erreur de la bibliothèque statique `libgestion_absences_lib.a` (`Absence.cpp`, `GestionnaireAbsences.cpp`, `GestionnaireFichiersAbsences.cpp`, `MenuAbsences.cpp`) et du binaire `./build/gestion_absences`.

---

### 1.2 Synthèse des scénarios fonctionnels testés

| Opération / Fonctionnalité | Actions réalisées | Comportement attendu & Observé | Statut |
| :--- | :--- | :--- | :---: |
| **1. Enregistrement & Contrôle de saisie** | Choix `1` : Employé `15`, Date `21/11/2026`, Motif `Maladie`, Justifiée `0` puis `O`. | Rejet immédiat de `0` (`Réponse invalide. Saisissez O ou N.`). Validation avec `O` et création de l'ID `1`. | **VALIDÉ** |
| **2. Consultation des actives** | Choix `2` à vide, puis après ajout de l'ID `1`. | Message `Aucune absence active.` initialement, puis affichage du tableau formaté avec l'absence active ID `1`. | **VALIDÉ** |
| **3. Historique employé** | Choix `3` pour l'employé `15` avant et après ajout. | Message `Aucune absence enregistrée...` initialement, puis historique complet avec le statut `Active`. | **VALIDÉ** |
| **4. Recherche par ID** | Choix `4` avec l'ID `15` (inexistant) puis ID `1` (existant). | Retourne `Absence introuvable.` pour `15`, puis affiche les détails complets pour l'ID `1`. | **VALIDÉ** |
| **5. Modification & Date future** | Choix `5` sur ID `1` : date future (`22/11/2026`), puis date courante (`21/11/2026`), motif `Retard`, justifiée `N`. | Rejet de la date future (`Erreur : impossible d'enregistrer une absence future.`). Mise à jour réussie après confirmation. | **VALIDÉ** |
| **6. Annulation d'absence** | Choix `6` sur l'absence ID `1`. | Confirmation `Absence annulée.`. L'absence passe au statut `Annulée`. | **VALIDÉ** |
| **7. Suppression définitive** | Choix `7` sur ID `1` avec confirmation `O`. | Avertissement et suppression définitive (`Absence supprimée avec succès !`). Options 2 et 3 confirment le retrait. | **VALIDÉ** |
| **8. Sauvegarde & Persistance** | Ajout de l'ID `2` (Employé `15`, `21/11/2026`, `Maladie`, `O`), arrêt `^C`, relance de `./build/gestion_absences`. | Restitution automatique de l'absence ID `2` rechargée depuis le fichier au redémarrage. | **VALIDÉ** |

---

### 1.3 Détail des sorties console du terminal

#### Opération 1 : Enregistrement et contrôle de saisie
```text
Votre choix : 1
Identifiant de l'employé : 15
Date (JJ/MM/AAAA) : 21/11/2026
Motif de l'absence : Maladie
Absence justifiée (O/N) : 0
Réponse invalide. Saisissez O ou N.

Votre choix : 1
Identifiant de l'employé : 15
Date (JJ/MM/AAAA) : 21/11/2026
Motif de l'absence : Maladie
Absence justifiée (O/N) : O
Absence enregistrée avec succès !
Identifiant : 1
```

#### Opérations 2 & 3 : Consultation des actives et historique employé
```text
Votre choix : 2
ID         | ID Employe | Date       | Motif   | Justifiee
1          | 15         | 21/11/2026 | Maladie | Oui

Votre choix : 3
Identifiant de l'employé : 15
Historique de l'employé 15
ID | Date | Motif | Justifiee | Statut
1 | 21/11/2026 | Maladie | Oui | Active
```

#### Opération 4 : Recherche par identifiant
```text
Votre choix : 4
Identifiant de l'absence : 15
Absence introuvable.

Votre choix : 4
Identifiant de l'absence : 1
ID absence : 1
ID employe : 15
Date : 21/11/2026
Motif : Maladie
Justifiee : Oui
Statut : Active
```

#### Opération 5 : Modification avec contrôle des dates futures
```text
Votre choix : 5
========== MODIFIER UNE ABSENCE ==========
Identifiant de l'absence : 1
Nouvelle date (JJ/MM/AAAA, Entrée pour conserver) : 22/11/2026
Nouveau motif (Entrée pour conserver) : Retard
Justifiée (O/N, Entrée pour conserver) : N
Récapitulatif de la modification
Date : 21/11/2026 -> 22/11/2026
Motif : Maladie -> Retard
Justifiée : Oui -> Non
Confirmer la modification (O/N) : O
Erreur : impossible d'enregistrer une absence future.

Votre choix : 5
========== MODIFIER UNE ABSENCE ==========
Identifiant de l'absence : 1
Nouvelle date (JJ/MM/AAAA, Entrée pour conserver) : 
Nouveau motif (Entrée pour conserver) : Retard
Justifiée (O/N, Entrée pour conserver) : N
Récapitulatif de la modification
Date : 21/11/2026 -> 21/11/2026
Motif : Maladie -> Retard
Justifiée : Oui -> Non
Confirmer la modification (O/N) : O
Absence modifiée avec succès !
```

#### Opération 6 & 7 : Annulation et suppression définitive
```text
Votre choix : 6
Identifiant de l'absence : 1
Absence annulée.

Votre choix : 7
========== SUPPRIMER UNE ABSENCE ==========
Identifiant de l'absence : 1
Informations
ID absence : 1
ID employe : 15
Date : 21/11/2026
Motif : Retard
Justifiee : Non
Statut : Annulée

ATTENTION : cette suppression est définitive.
Confirmer la suppression (O/N) : O
Absence supprimée avec succès !
```

#### Opération 8 : Sauvegarde et persistance après redémarrage
```text
Votre choix : 1
Identifiant de l'employé : 15
Date (JJ/MM/AAAA) : 21/11/2026
Motif de l'absence : Maladie
Absence justifiée (O/N) : O
Absence enregistrée avec succès !
Identifiant : 2

Votre choix : ^C

Sofyane_HB @ GestionAbsences $ ./build/gestion_absences
Votre choix : 2
ID         | ID Employe | Date       | Motif   | Justifiee
2          | 15         | 21/11/2026 | Maladie | Oui
```

---

## 2. Tests Unitaires Automatisés avec GoogleTest

### 2.1 Configuration CMake et commandes d'exécution

Pour isoler et vérifier automatiquement la logique du code C++, la suite de tests unitaires basée sur GoogleTest 1.17.0 (`gtest-devel`) a été configurée dans un répertoire de build dédié (`build-gtest`).

```bash
# 1. Installation du paquet GoogleTest sous Fedora 44
sudo dnf install gtest-devel

# 2. Application du patch de test
unzip -o ~/Downloads/GestionAbsences_GTest_patch.zip -d ~/Downloads

# 3. Configuration CMake avec activation des tests
cmake -S . -B build-gtest \
  -DGESTION_ABSENCES_BUILD_GTEST=ON \
  -DGESTION_ABSENCES_BUILD_DEMO=OFF

# 4. Compilation des tests
cmake --build build-gtest -j2

# 5. Exécution automatisée via CTest et le binaire dédié
ctest --test-dir build-gtest --output-on-failure
./build-gtest/test_gestion_absences_gtest
```

---

### 2.2 Table des 11 tests unitaires exécutés

| N° | Suite de tests | Nom du test unitaire | Objectif de vérification | Durée | Statut |
| :- | :--- | :--- | :--- | :---: | :---: |
| **1** | `GestionnaireAbsencesTest` | `AjouterAbsenceValide` | Ajout nominal d'une absence et génération de l'ID. | 0 ms | **PASS** |
| **2** | `GestionnaireAbsencesTest` | `RefuserEmployeInexistant` | Rejet de la création si l'employé n'existe pas dans le système. | 0 ms | **PASS** |
| **3** | `GestionnaireAbsencesTest` | `RefuserDateInvalide` | Rejet des dates mal formatées ou invalides dans le calendrier. | 0 ms | **PASS** |
| **4** | `GestionnaireAbsencesTest` | `RefuserDateFuture` | Rejet systématique des absences positionnées dans le futur. | 0 ms | **PASS** |
| **5** | `GestionnaireAbsencesTest` | `RefuserDoublonEmployeEtDate` | Interdiction d'avoir deux absences le même jour pour un employé. | 0 ms | **PASS** |
| **6** | `GestionnaireAbsencesTest` | `RechercherAbsenceExistante` | Récupération exacte des données d'une absence via son ID. | 0 ms | **PASS** |
| **7** | `GestionnaireAbsencesTest` | `ConsulterHistoriqueEmploye` | Restitution complète de l'historique lié à un employé. | 0 ms | **PASS** |
| **8** | `GestionnaireAbsencesTest` | `ModifierAbsence` | Validation de la mise à jour des attributs d'une absence. | 0 ms | **PASS** |
| **9** | `GestionnaireAbsencesTest` | `AnnulerAbsenceLaRetireDesActivesMaisGardeHistorique` | Passage au statut annulé sans suppression de l'historique. | 0 ms | **PASS** |
| **10** | `GestionnaireAbsencesTest` | `SupprimerAbsenceDefinitivement` | Suppression physique d'une absence des structures mémoire. | 0 ms | **PASS** |
| **11** | `GestionnaireFichiersAbsencesTest` | `SauvegarderEtRechargerAbsence` | Sérialisation et chargement fidèle des données depuis le fichier. | 0 ms | **PASS** |

---

### 2.3 Sorties d'exécution GoogleTest & CTest

#### Rapport de CTest
```text
Test project /home/sofyane_hb/Downloads/Systeme-Gestion-Employees-gestion-absences/gestion_absences/GestionAbsences/build-gtest
      Start  1: GestionnaireAbsencesTest.AjouterAbsenceValide
 1/11 Test  #1: GestionnaireAbsencesTest.AjouterAbsenceValide ..................................   Passed    0.01 sec
      Start  2: GestionnaireAbsencesTest.RefuserEmployeInexistant
 2/11 Test  #2: GestionnaireAbsencesTest.RefuserEmployeInexistant ..............................   Passed    0.01 sec
      Start  3: GestionnaireAbsencesTest.RefuserDateInvalide
 3/11 Test  #3: GestionnaireAbsencesTest.RefuserDateInvalide ...................................   Passed    0.01 sec
      Start  4: GestionnaireAbsencesTest.RefuserDateFuture
 4/11 Test  #4: GestionnaireAbsencesTest.RefuserDateFuture .....................................   Passed    0.01 sec
      Start  5: GestionnaireAbsencesTest.RefuserDoublonEmployeEtDate
 5/11 Test  #5: GestionnaireAbsencesTest.RefuserDoublonEmployeEtDate ...........................   Passed    0.01 sec
      Start  6: GestionnaireAbsencesTest.RechercherAbsenceExistante
 6/11 Test  #6: GestionnaireAbsencesTest.RechercherAbsenceExistante ............................   Passed    0.01 sec
      Start  7: GestionnaireAbsencesTest.ConsulterHistoriqueEmploye
 7/11 Test  #7: GestionnaireAbsencesTest.ConsulterHistoriqueEmploye ............................   Passed    0.01 sec
      Start  8: GestionnaireAbsencesTest.ModifierAbsence
 8/11 Test  #8: GestionnaireAbsencesTest.ModifierAbsence .......................................   Passed    0.01 sec
      Start  9: GestionnaireAbsencesTest.AnnulerAbsenceLaRetireDesActivesMaisGardeHistorique
 9/11 Test  #9: GestionnaireAbsencesTest.AnnulerAbsenceLaRetireDesActivesMaisGardeHistorique ...   Passed    0.01 sec
      Start 10: GestionnaireAbsencesTest.SupprimerAbsenceDefinitivement
10/11 Test #10: GestionnaireAbsencesTest.SupprimerAbsenceDefinitivement ........................   Passed    0.01 sec
      Start 11: GestionnaireFichiersAbsencesTest.SauvegarderEtRechargerAbsence
11/11 Test #11: GestionnaireFichiersAbsencesTest.SauvegarderEtRechargerAbsence .................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 11
Total Test time (real) = 0.11 sec
```

#### Rapport détaillé du binaire GoogleTest
```text
Running main() from /builddir/build/BUILD/gtest-1.17.0-build/googletest-1.17.0/googletest/src/gtest_main.cc
[==========] Running 11 tests from 2 test suites.
[----------] Global test environment set-up.
[----------] 10 tests from GestionnaireAbsencesTest
[ RUN      ] GestionnaireAbsencesTest.AjouterAbsenceValide
[       OK ] GestionnaireAbsencesTest.AjouterAbsenceValide (0 ms)
[ RUN      ] GestionnaireAbsencesTest.RefuserEmployeInexistant
[       OK ] GestionnaireAbsencesTest.RefuserEmployeInexistant (0 ms)
[ RUN      ] GestionnaireAbsencesTest.RefuserDateInvalide
[       OK ] GestionnaireAbsencesTest.RefuserDateInvalide (0 ms)
[ RUN      ] GestionnaireAbsencesTest.RefuserDateFuture
[       OK ] GestionnaireAbsencesTest.RefuserDateFuture (0 ms)
[ RUN      ] GestionnaireAbsencesTest.RefuserDoublonEmployeEtDate
[       OK ] GestionnaireAbsencesTest.RefuserDoublonEmployeEtDate (0 ms)
[ RUN      ] GestionnaireAbsencesTest.RechercherAbsenceExistante
[       OK ] GestionnaireAbsencesTest.RechercherAbsenceExistante (0 ms)
[ RUN      ] GestionnaireAbsencesTest.ConsulterHistoriqueEmploye
[       OK ] GestionnaireAbsencesTest.ConsulterHistoriqueEmploye (0 ms)
[ RUN      ] GestionnaireAbsencesTest.ModifierAbsence
[       OK ] GestionnaireAbsencesTest.ModifierAbsence (0 ms)
[ RUN      ] GestionnaireAbsencesTest.AnnulerAbsenceLaRetireDesActivesMaisGardeHistorique
[       OK ] GestionnaireAbsencesTest.AnnulerAbsenceLaRetireDesActivesMaisGardeHistorique (0 ms)
[ RUN      ] GestionnaireAbsencesTest.SupprimerAbsenceDefinitivement
[       OK ] GestionnaireAbsencesTest.SupprimerAbsenceDefinitivement (0 ms)
[----------] 10 tests from GestionnaireAbsencesTest (0 ms total)

[----------] 1 test from GestionnaireFichiersAbsencesTest
[ RUN      ] GestionnaireFichiersAbsencesTest.SauvegarderEtRechargerAbsence
[       OK ] GestionnaireFichiersAbsencesTest.SauvegarderEtRechargerAbsence (0 ms)
[----------] 1 test from GestionnaireFichiersAbsencesTest (0 ms total)

[----------] Global test environment tear-down
[==========] 11 tests from 2 test suites ran. (1 ms total)
[  PASSED  ] 11 tests.
```

---

## 3. Conclusion

La campagne de validation menée sur le module **GestionAbsences** atteste d'un niveau de qualité et de conformité optimal :
* **Ergonomie et robustesse CLI :** Les tests manuels valides et aux limites confirment la stabilité de l'interface utilisateur et l'efficacité des mécanismes de contrôle de saisie.
* **Intégrité et persistance :** La sauvegarde automatique garantit que les données enregistrées restent intégralement conservées après arrêt du programme.
* **Couverture et régression :** Les **11 tests unitaires GoogleTest** s'exécutent avec succès (**100 % PASS**), offrant une garantie de non-régression et assurant une intégration harmonieuse au sein du système global de gestion des employés.