# GestionEmployes_UI — interfaces console

Interfaces console du projet, écrites pour l'API **réelle** du back-end
([Systeme-Gestion-Employees](https://github.com/mdakh404/Systeme-Gestion-Employees), branche `main`).
Il n'y a plus de stubs.

## Compilation (depuis la racine du dépôt Systeme-Gestion-Employees)

Ce dossier se place à la racine du dépôt, à côté de `headers/` et `source/`.
On compile l'UI avec les sources **métier** du back-end, sans son `main.cpp` ni ses menus `Menu*` / `Espace*` :

```bash
g++ -std=c++17 -IGestionEmployes_UI -IGestionEmployes_UI/include -Iheaders \
  GestionEmployes_UI/main.cpp GestionEmployes_UI/include/integration/*.cpp GestionEmployes_UI/UI/*/*.cpp \
  source/Absence.cpp source/Conge.cpp source/Database.cpp source/Date.cpp source/DbPersistence.cpp \
  source/DemandeAdministrative.cpp source/Departement.cpp source/Employe.cpp source/GestionnaireAbsences.cpp \
  source/GestionnaireConges.cpp source/GestionnaireDemandes.cpp source/GestionnaireEmployes.cpp \
  source/GestionnaireEmployesBD.cpp source/GestionnaireFichiersAbsences.cpp source/GestionnaireFichiersConge.cpp \
  source/GestionnaireFichiersDemandes.cpp source/Personne.cpp source/PrimeService.cpp source/RetirementService.cpp \
  source/Service.cpp source/TrainingService.cpp source/Validation.cpp \
  -o GestionEmployes_UI.exe
```

Testé sur le commit `cba61be` de `main` : compilation sans erreur ni warning, et parcours admin / RH / employé validé.
Le `main.cpp` du back-end compile toujours de son côté : ce dossier n'y touche pas.

## Contenu

- `UI/` : vues console (affichage, saisie, try/catch).
- `include/BackendTypes.h` : inclut les headers du back-end.
- `include/integration/Operations.*` : conversion des dates de congé (`JJ/MM/AAAA` ↔ `AAAA-MM-JJ`) et sauvegarde après chaque opération.

## Deux éléments absents du back-end `main` (à reprendre par le back-end)

- `include/integration/GestionnaireDepartements.*` : copie de la branche `gestion_employee`, plus une sauvegarde dans `data/departements.txt`.
- `include/integration/AuthService.*` :
  - admin et RH se connectent via `data/comptes_ui.txt` (créé avec `admin/admin` et `rh/rh`) ;
  - l'employé se connecte par matricule ou CIN.

Quand le back-end fournira ces deux éléments, supprimer ces fichiers.
