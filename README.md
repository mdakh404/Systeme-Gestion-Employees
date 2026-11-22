# Employee Management System

This guide is for students who will extend the project. It explains the current classes, their methods, how the application is organized, and what remains to be implemented.

## Current project status

The application is a C++17 console program. Employee data is stored in memory only and is lost when the program exits. Administrative requests are additionally persisted to `data/demandes.txt`.

- The administrator space can add, edit, delete, search, view, and list employees.
- Employees can sign in using a matricule or CIN, view their profile, change their email or telephone, and deposit or consult their own administrative requests.
- The HR manager space (`EspaceRH`) consults and processes administrative requests.
- Requests follow the lifecycle `En attente` → `Acceptee` / `Refusee`; a decision is final.
- A `Departement` class exists, but there is no department manager or department registry.
- `Salaire` stores base salary and its change history; payroll calculations are not implemented.

## Build and run on Windows

The following command works with GCC/MinGW from PowerShell. It compiles every source file of the project:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic `
  -Iheaders -Iheaders\admin -Iheaders\espaceEmploye `
  source\*.cpp source\admin\*.cpp source\espaceEmploye\*.cpp `
  -o GestionEmployes.exe

.\GestionEmployes.exe
```

Under Linux (Bash):

```bash
g++ -std=c++17 -Wall -Wextra -pedantic \
  -Iheaders -Iheaders/admin -Iheaders/espaceEmploye \
  source/*.cpp source/admin/*.cpp source/espaceEmploye/*.cpp \
  -o GestionEmployes

./GestionEmployes
```

In Visual Studio Code or another IDE, include the same `.cpp` files and add `headers`, `headers\admin`, and `headers\espaceEmploye` to the header search paths. The executable is a build artifact and should not be committed.

## How the code is organized

The program entry point is `source/main.cpp`. It creates one `GestionnaireEmployes`, one `GestionnaireDemandes`, one `GestionnaireFichiersDemandes` and one `MenuDemandes`, then routes choices to the administrator, HR, or employee console space.

Keep console input and display in the `Espace...` and `Menu...` classes. Put reusable rules and data changes in the `Gestionnaire...` classes and the domain classes. This prevents different menu spaces from keeping separate copies of the data or applying different validation rules.

## Main classes and their methods

### `GestionnaireEmployes`

Declared in `headers/GestionnaireEmployes.h` and implemented in `source/GestionnaireEmployes.cpp`. This in-memory service owns the employee records and is the main API for other project groups.

- `ajouter(donnees)` validates input, rejects duplicate CIN, assigns the next ID and a generated matricule, records the creation in the history, and returns the new ID.
- `modifier(id, donnees)` finds and validates the employee, checks CIN uniqueness, then updates personal and professional details.
- `supprimer(id)` removes an employee. It throws `EmployeIntrouvable` if the ID does not exist.
- `rechercher(id)` returns a pointer to an employee or `nullptr` if absent.
- `trouver(id)` returns a mutable employee reference. It throws `EmployeIntrouvable` if absent.
- `rechercherParNom(motif)` returns employees whose first or last name contains the motif; matching ignores ASCII letter case.
- `rechercherParCin(cin)` returns the matching employee or `nullptr`.
- `rechercherParMatricule(matricule)` returns the matching employee or `nullptr`.
- `modifierStatut(id, statut)` changes only the status. Valid values are `Actif`, `Inactif`, `En conge`, and `Suspendu`.
- `modifierCoordonnees(id, email, telephone)` validates and changes email and telephone without replacing the rest of the employee record.
- `lister()` returns copies of all employees ordered by ID.
- `nombre()` returns the number of employees currently in memory.
- `historique(id)` returns an employee's change history. It throws if the employee is absent.
- `affecterDepartement(idEmploye, idDepartement)` changes the employee's department ID. ID `0` means unassigned; negative IDs are rejected.
- `employesDuDepartement(idDepartement)` returns employees with that department ID.
- `effectifDepartement(idDepartement)` counts employees with that department ID.

Private helpers `valider` and `verifierCinUnique` centralize employee validation and CIN uniqueness. Validation covers names, CIN, email, telephone, status, finite non-negative salary, dates, and minimum hiring age. Do not duplicate these checks in menu classes.

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

- `EspaceEmploye(gestion, demandes)` connects the space to the shared employee manager and to the administrative-requests menu.
- `executer()` starts the sign-in and return menu loop.
- `connecter()` *(private)* looks up an employee by matricule or CIN. This is identifier lookup, not password authentication.
- `menuConnecte()` *(private)* displays actions for the signed-in employee.
- `afficherProfil()` *(private)* displays personal, professional, and department information.
- `afficherInfosPersonnelles()` *(private)* displays name, CIN, birth date, email, and telephone.
- `afficherInfosProfessionnelles()` *(private)* displays matricule, hiring date, status, and seniority.
- `afficherDepartement()` *(private)* displays the department ID or that the employee is unassigned.
- `modifierCoordonnees()` *(private)* collects email/telephone changes and calls the manager's validated method.
- `deposerDemande()` *(private)* delegates to `MenuDemandes::deposerDemande` for the signed-in employee.
- `mesDemandes()` *(private)* delegates to `MenuDemandes::consulterMesDemandes`.
- `deconnecter()` *(private)* clears the current employee ID.

To try this space, first add an employee from the administrator menu, then sign in during the same run. Closing the program clears all employee records; administrative requests stay in `data/demandes.txt`.

### `EspaceRH`

Declared in `headers/EspaceRH.h` and implemented in `source/EspaceRH.cpp`. This is the HR console space. It currently offers request processing only; other HR features will be added by another developer.

- `EspaceRH(gestion, demandes)` connects the space to the shared employee manager and to the administrative-requests menu.
- `executer()` starts the menu loop.
- `menuConnecte()` *(private)* displays the HR menu and dispatches to `MenuDemandes`.

### `MenuDemandes`

Declared in `headers/MenuDemandes.h` and implemented in `source/MenuDemandes.cpp`. It is the console layer of the administrative-requests module and contains no business rules.

- `deposerDemande(idEmploye)` collects the request type and description, creates the request, and saves it to disk.
- `consulterMesDemandes(idEmploye)` displays the requests of one employee.
- `consulterDemandes()` displays all requests or only those awaiting a decision.
- `traiterDemande()` lists pending requests, then accepts or refuses one by ID.

### `GestionnaireDemandes`

Declared in `headers/GestionnaireDemandes.h` and implemented in `source/GestionnaireDemandes.cpp`. It holds requests in memory and applies the request rules.

- `creerDemande(donnees)` validates input and creates a request with the status `En attente`.
- `traiterDemande(id, decision)` accepts or refuses a pending request. It throws `DemandeIntrouvable` if the ID is unknown and `DemandeDejaTraitee` if a decision was already taken.
- `rechercher(id)` returns a pointer or `nullptr`.
- `lister()`, `listerParEmploye(idEmploye)`, and `listerEnAttente()` return request copies.
- `charger(donnees)` rebuilds the in-memory state from the file loaded at start-up.

### `GestionnaireFichiersDemandes`

Declared in `headers/GestionnaireFichiersDemandes.h` and implemented in `source/GestionnaireFichiersDemandes.cpp`. It is the persistence layer for requests.

- `lireDemandes()` reads `data/demandes.txt` and returns an empty vector when the file is missing. Malformed lines, non-positive IDs and trailing characters are rejected.
- `ecrireDemandes(demandes)` writes the whole file. It writes to a temporary file first and renames it, so an interrupted write does not destroy the previous data.

### `DemandeAdministrative`

Declared in `headers/DemandeAdministrative.h` and implemented in `source/DemandeAdministrative.cpp`.

- `DemandeAdministrative(id, donnees)` creates a request with the status `En attente`.
- `definirStatut(statut)` moves the request forward and throws `DemandeDejaTraitee` once a decision was taken.
- `estEnAttente()`, `afficherInfos()`, and the getters expose the state.
- `statutValide(statut)` and `typesProposes()` are static helpers.

### `Employe` and `DonneesEmploye`

Declared in `headers/Employe.h` and implemented in `source/Employe.cpp`.

`DonneesEmploye` is the value object used when creating or fully modifying a record. It contains name, first name, CIN, email, telephone, birth date, hiring date, status, and base salary.

`Employe(id, matricule, donnees)` constructs an employee. Its other methods are:

- `afficherInfos()` displays inherited personal information and employee-specific fields.
- `modifierInformations(donnees)` updates editable personal/professional fields and records the change. Normally call it through `GestionnaireEmployes::modifier` to apply validation and CIN uniqueness checks.
- `modifierStatut(statut)` changes status and records the change. Normally call it through `GestionnaireEmployes::modifierStatut` to validate the value.
- `affecterDepartement(id)` and `affecterPoste(id)` set assignment IDs and record the change. Department assignment through the manager rejects negative IDs, but does not check whether a department exists.
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

### `Departement`

Declared in `headers/Departement.h`, this class represents one department. It is not a department registry or manager.

- `Departement(id, nom, description)` stores the ID and validates the name and description.
- `setNom(nom)` and `setDescription(description)` validate replacements.
- `getId()`, `getNom()`, and `getDescription()` return the fields.
- `afficherInfos()` displays the department.
- `operator==` compares department IDs.

### `Salaire`

Declared in `headers/Salaire.h`. This is the current minimal salary model.

- `Salaire()` creates a zero-base salary; `Salaire(base)` initializes a base amount.
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
- `operator==`, `operator<`, and `operator<=` compare dates.

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

These exceptions can also be caught as `std::exception`.

