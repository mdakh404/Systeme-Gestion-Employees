# Couche MySQL / MariaDB

L'application garde son fonctionnement par fichiers (`data/*.txt`) **et** sait travailler avec MySQL.
Au demarrage, `main.cpp` tente de se connecter : si la base repond, TOUT (employes, conges, demandes,
absences, formations) est lu et ecrit en base ; sinon l'application continue en mode fichiers.

## 1. Preparer la base (une seule fois)

```bash
mysql -u root -p < docs/gestion_employes.sql      # schema de l'equipe BD
mysql -u root -p < docs/migration_mysql.sql       # adaptations necessaires au code C++
```
(`migration_mysql.sql` : departement/poste optionnels, statuts, colonne `description` des demandes,
colonne `annulee` des absences, tables `formation` / `inscription_formation`.
Elle ne doit etre executee qu'une fois. MySQL >= 8.0.19 ou MariaDB >= 10.4.)

## 2. Parametres de connexion

Copier `docs/db.cfg.example` en `data/db.cfg` et adapter (le dossier `data/` n'est pas versionne) :

```
host=127.0.0.1
port=3306
user=root
password=VOTRE_MOT_DE_PASSE
database=gestion_employes
```
Alternative : variables d'environnement `EMS_DB_HOST`, `EMS_DB_PORT`, `EMS_DB_USER`,
`EMS_DB_PASSWORD`, `EMS_DB_NAME` (elles priment sur le fichier).

## 3. Bibliotheque cliente

* **Windows / MinGW (MSYS2, terminal "MSYS2 MinGW x64")** :
  `pacman -S mingw-w64-x86_64-libmariadbclient` (si le nom n'existe pas : `pacman -Ss mariadb`).
  Les en-tetes sont dans `C:\msys64\mingw64\include\mariadb`.
* **Ubuntu/Debian** : `sudo apt install libmariadb-dev` (en-tetes : `/usr/include/mariadb`).
* MySQL Connector/C++ n'est PAS utilise (incompatible avec MinGW) : c'est l'API C `mysql.h`.

## 4. Compilation

Sans `-DUSE_MYSQL` : comportement d'origine (fichiers uniquement).

```bash
# Avec MySQL (MSYS2 MinGW64)
g++ -std=c++17 -Wall -Wextra -DUSE_MYSQL -I/mingw64/include/mariadb \
    -Iheaders -Iheaders/admin -Iheaders/espaceEmploye \
    source/*.cpp source/admin/*.cpp source/espaceEmploye/*.cpp \
    -L/mingw64/lib -lmariadb -o GestionEmployes.exe
```
Linux : meme commande avec `-I/usr/include/mariadb` et `-lmariadb`.
Au lancement, la premiere ligne affichee indique le mode : `[BD] Connecte a MySQL : ...` ou
`[BD] Base de donnees indisponible (...) -> mode fichiers`.

Sous Windows, `libmariadb.dll` doit etre a cote de l'exe (ou dans le PATH) : copier
`C:\msys64\mingw64\bin\libmariadb.dll`.

## 5. Test automatique (smoke test)

```bash
g++ -std=c++17 -DUSE_MYSQL -I/mingw64/include/mariadb -Iheaders \
    $(ls source/*.cpp source/admin/*.cpp source/espaceEmploye/*.cpp | grep -v main.cpp) \
    tests/test_bd.cpp -L/mingw64/lib -lmariadb -o test_bd.exe
./test_bd.exe
```
Il ajoute un employe, un conge, une demande, une absence, recharge depuis la base et verifie
que tout est revenu. Il laisse quelques lignes de test en base (employe marque `supprime`).

## 6. Correspondance C++ <-> SQL

| C++ | SQL |
|---|---|
| `Actif / Inactif / En conge / Suspendu` | `actif / inactif / en_conge / suspendu` |
| employe supprime dans l'application | `statut = 'supprime'` (pas de DELETE : trigger `trg_employe_bd`) |
| `idDepartement / idPoste = 0` | `NULL` |
| conge `En attente / Acceptee / Refusee` | `en_attente / approuve / refuse` |
| conge `Annuel / Maladie / Sans solde / Exceptionnel` | `annuel / maladie / sans_solde / exceptionnel` |
| demande `En attente / Acceptee / Refusee` | `en_attente / traitee / rejetee` |
| dates `JJ/MM/AAAA` (employes, absences, demandes) | `DATE` (conversion dans la couche BD) |

## 7. Fichiers ajoutes / modifies

Ajoutes : `headers/Database.h`, `source/Database.cpp`, `headers/DbPersistence.h`,
`source/DbPersistence.cpp`, `source/GestionnaireEmployesBD.cpp`, `docs/migration_mysql.sql`,
`docs/db.cfg.example`, `docs/README_BD.md`, `tests/test_bd.cpp`.
Modifies (un `if (Database::instance().estConnectee())` en tete des fonctions de persistance) :
`GestionnaireEmployes.h/.cpp`, `GestionnaireFichiersConge.cpp`, `GestionnaireFichiersDemandes.cpp`,
`GestionnaireFichiersAbsences.cpp`, `TrainingService.cpp`, `main.cpp`, `admin/EspaceAdmin.cpp`
(annule l'ajout en memoire si la base le refuse).
