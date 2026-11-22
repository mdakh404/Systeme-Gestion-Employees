# Employee Management System

This guide is for students who will extend the project. It explains the current classes, their methods, how the application is organized, and what remains to be implemented.

## Current project status

The project is written in C++17. Employee and department data are stored in memory only and are lost when the program exits. There is no file storage or database. The current `source/main.cpp` is a simple test driver: it creates the managers and domain objects, calls their methods, and prints whether each check succeeds.

- The administrator space can add, edit, delete, search, view, and list employees.
- Employees can sign in using a matricule or CIN, view their profile, and change their email or telephone.
- The HR manager space is not implemented; its main-menu entry is a placeholder.
- `GestionnaireDepartements` manages departments in memory and employee assignment checks department IDs.
- `Salaire` stores base salary and its change history; payroll calculations are not implemented.

## Build and run the test driver on Windows

```powershell
g++ -std=c++17 -Wall -Wextra -Wpedantic -Iheaders -Iheaders\admin -Iheaders\espaceEmploye source\Date.cpp source\Departement.cpp source\Employe.cpp source\GestionnaireDepartements.cpp source\GestionnaireEmployes.cpp source\Personne.cpp source\Validation.cpp source\admin\EspaceAdmin.cpp source\espaceEmploye\EspaceEmploye.cpp source\main.cpp -o GestionEmployes.exe
.\GestionEmployes.exe
```

The test driver checks dates, departments, employee operations, assignments, salary updates, and history using temporary in-memory records. It exits with a non-zero code if a check fails. No CMake, CTest, separate test source, or test framework is required.

## How the code is organized

The current entry point is `source/main.cpp`, which serves as a lightweight test driver rather than an interactive menu. It creates `GestionnaireDepartements` first, then creates `GestionnaireEmployes` with a reference to it. This order matters: the employee manager uses the department registry to validate assignments. `EspaceAdmin` and `EspaceEmploye` remain in the project as console-space classes, but they are not launched by this test driver. The HR space is not implemented.

Keep console input and display in the `Espace...` classes. Put reusable employee rules and data changes in `GestionnaireEmployes` and the domain classes. This prevents different menu spaces from keeping separate copies of employees or applying different validation rules.

## Main classes and their methods

### `GestionnaireEmployes`

Declared in `headers/GestionnaireEmployes.h` and implemented in `source/GestionnaireEmployes.cpp`. Construct it with a `GestionnaireDepartements` reference. This in-memory service owns employee records and is the main API for other project groups. Lookups return employee values (or `std::optional<Employe>`) rather than references into its internal map, so a caller's result remains valid if a record is later erased.

- `ajouter(donnees)` validates input, rejects duplicate CIN, assigns the next ID and a generated matricule, records the creation in the history, and returns the new ID.
- `modifier(id, donnees)` finds and validates the employee, checks CIN uniqueness, then updates personal and professional details.
- `supprimer(id)` removes an employee. It throws `EmployeIntrouvable` if the ID does not exist.
- `rechercher(id)` returns `std::optional<Employe>`, empty if absent.
- `trouver(id)` returns an employee copy. It throws `EmployeIntrouvable` if absent.
- `rechercherParNom(motif)` returns employee copies whose first or last name contains the motif; matching ignores ASCII letter case.
- `rechercherParCin(cin)` and `rechercherParMatricule(matricule)` return `std::optional<Employe>`, empty if absent.
- `modifierStatut(id, statut)` changes only the status. Valid values are `Actif`, `Inactif`, `En conge`, and `Suspendu`.
- `modifierCoordonnees(id, email, telephone)` validates and changes email and telephone without replacing the rest of the employee record.
- `lister()` returns copies of all employees ordered by ID.
- `nombre()` returns the number of employees currently in memory.
- `historique(id)` returns a copy of the employee's change history. It throws if the employee is absent.
- `affecterDepartement(idEmploye, idDepartement)` changes the employee's department ID after checking the registry. ID `0` means unassigned; negative or unknown positive IDs are rejected.
- `employesDuDepartement(idDepartement)` returns employee copies with that department ID.
- `effectifDepartement(idDepartement)` counts employees with that department ID.

Private helpers `valider` and `verifierCinUnique` centralize employee validation and CIN uniqueness. Validation covers names (which must contain a letter), CIN, email, telephone, status, finite non-negative salary, valid non-future birth and hiring dates, and minimum hiring age. Do not duplicate these checks in menu classes.

### `EspaceAdmin`

Declared in `headers/admin/EspaceAdmin.h` and implemented in `source/admin/EspaceAdmin.cpp`. This class handles administrator prompts and display, then delegates employee changes to `GestionnaireEmployes`.

- `EspaceAdmin(gestion)` connects the console space to the shared employee manager.
- `menuAdmin()` displays the administrator menu and dispatches choices.
- `ajouterEmploye()` collects employee data and calls `GestionnaireEmployes::ajouter`.
- `modifierEmploye()` collects a full replacement record and calls `modifier`.
- `supprimerEmploye()` confirms and calls `supprimer`.
- `rechercherEmploye()` searches by name and displays matches.
- `consulterEmploye()` finds an employee by ID and displays details.
- `afficherEmployes()` displays the result of `lister()`.

### `EspaceEmploye`

Declared in `headers/espaceEmploye/EspaceEmploye.h` and implemented in `source/espaceEmploye/EspaceEmploye.cpp`. It provides self-service for one employee during the current program run.

- `EspaceEmploye(gestion, gestionnaireDepartements)` connects the space to the shared employee manager and department registry.
- `executer()` starts the sign-in and return menu loop.
- `connecter()` *(private)* looks up an employee by matricule or CIN. This is identifier lookup, not password authentication.
- `menuConnecte()` *(private)* displays actions for the signed-in employee.
- `afficherProfil()` *(private)* displays personal, professional, and department information.
- `afficherInfosPersonnelles()` *(private)* displays name, CIN, birth date, email, and telephone.
- `afficherInfosProfessionnelles()` *(private)* displays matricule, hiring date, status, and seniority.
- `afficherDepartement()` *(private)* displays the department ID or that the employee is unassigned.
- `modifierCoordonnees()` *(private)* collects email/telephone changes and calls the manager's validated method.
- `deconnecter()` *(private)* clears the current employee ID.

To try this space, first add an employee from the administrator menu, then sign in during the same run. Closing the program clears all employee records.

### `Employe` and `DonneesEmploye`

Declared in `headers/Employe.h` and implemented in `source/Employe.cpp`.

`DonneesEmploye` is the value object used when creating or fully modifying a record. It contains name, first name, CIN, email, telephone, birth date, hiring date, status, and base salary.

`Employe(id, matricule, donnees)` constructs an employee. Its other methods are:

- `afficherInfos()` displays inherited personal information and employee-specific fields.
- `modifierInformations(donnees)` updates editable personal/professional fields and records the change. Normally call it through `GestionnaireEmployes::modifier` to apply validation and CIN uniqueness checks.
- `modifierStatut(statut)` changes status and records the change. Normally call it through `GestionnaireEmployes::modifierStatut` to validate the value.
- `affecterDepartement(id)` and `affecterPoste(id)` set assignment IDs and record the change. Department assignment through `GestionnaireEmployes` also checks that the department ID exists.
- `anciennete(reference)` returns completed years since hiring.
- `ajouterHistorique(evenement)` adds a dated history entry; `getHistorique()` reads the history.
- `getMatricule()`, `getDateEmbauche()`, `getStatut()`, `getIdDepartement()`, `getIdPoste()`, and `getSalaire()` return employee-specific values.
- `operator==` compares employee IDs.

### `Personne`

Declared in `headers/Personne.h`. This base class stores shared identity data.

- `Personne(...)` initializes the ID and personal fields.
- `afficherInfos()` displays those fields; `virtual ~Personne()` supports polymorphic cleanup.
- `getId()`, `getNom()`, `getPrenom()`, `getCin()`, `getDateNaissance()`, `getEmail()`, and `getTelephone()` read values.
- `setNom()`, `setPrenom()`, `setCin()`, `setDateNaissance()`, `setEmail()`, and `setTelephone()` update values.

Prefer manager operations when changing an employee so validation and history are not bypassed.

### `GestionnaireDepartements`

Declared in `headers/GestionnaireDepartements.h` and implemented in `source/GestionnaireDepartements.cpp`. This is the in-memory department registry.

- `ajouter(nom, description)` validates and registers a department with a generated ID; names must be unique.
- `modifier(id, nom, description)` validates and updates a department, preserving its ID.
- `supprimer(id, employes)` removes an unused department. It refuses to delete a department that still has employees assigned to it.
- `rechercher(id)` returns `std::optional<Departement>`; `trouver(id)` returns a value or throws `DepartementIntrouvable`.
- `lister()` returns department copies; `contient(id)` checks whether an ID is registered; `nombre()` returns the registry size.

### `Departement`

Declared in `headers/Departement.h`, this class represents one department.

- `Departement(id, nom, description)` stores the ID and validates the name and description.
- `setNom(nom)` and `setDescription(description)` validate replacements.
- `getId()`, `getNom()`, and `getDescription()` return the fields.
- `afficherInfos()` displays the department.
- `operator==` and `operator!=` compare department IDs.

### `Salaire`

Declared in `headers/Salaire.h`. This is the current minimal salary model.

- `Salaire()` creates a zero-base salary; `Salaire(base)` initializes a base amount without treating the initial value as a revision.
- `getSalaireBase()`, `getPrimeAnciennete()`, and `getHistorique()` read salary values.
- `setSalaireBase(base)` rejects negative values and records the previous base salary.

Advanced payroll and seniority-premium calculations are not implemented.

### `Date`

Declared in `headers/Date.h`. This type stores day, month, and year.

- `Date(jour, mois, annee)` constructs a date.
- `estValide()` checks the date; `toString()` formats it as `JJ/MM/AAAA`.
- `fromString(texte)` parses a date and throws `std::invalid_argument` for invalid input.
- `aujourdhui()` returns today's date.
- `anneesEntre(debut, fin)` calculates completed years between two dates.
- `operator==`, `operator!=`, `operator<`, `operator<=`, `operator>`, and `operator>=` compare dates.

## Validation and exceptions

The functions in `Validation` return whether a field passes its rule:

- `nomValide`, `cinValide`, `emailValide`, and `telephoneValide` check employee fields.
- `nomDepartementValide` checks a department name.
- `sansSeparateur` checks for reserved separators and line breaks in department descriptions.

The domain exception classes are declared in `headers/Exceptions.h`:

- `ExceptionEmploye` is the common base exception.
- `EmployeIntrouvable(id)` signals an unknown employee ID.
- `EmployeExistant(cin)` signals a duplicate CIN.
- `DonneesInvalides(erreurs)` contains one or more validation errors; `erreurs()` returns them.
- `ExceptionDepartement` is the base exception for department errors.
- `DepartementIntrouvable(id)`, `DepartementExistant(nom)`, and `DepartementUtilise(id)` report missing, duplicate, or still-assigned departments.

These exceptions can also be caught as `std::exception`.

## Remaining work for project groups

- Implement the HR console space in the main menu and connect it to `GestionnaireEmployes` and `GestionnaireDepartements`.
- Add department-management prompts where required. The in-memory department service is available, but the current menus do not create or modify departments.
- Employee and department data are intentionally volatile; any future persistence implementation must be a separate, coordinated contribution.
- Employee self-service currently identifies users by CIN or matricule only. Password authentication is not implemented.
- Advanced payroll calculations are outside the current `Salaire` implementation.

