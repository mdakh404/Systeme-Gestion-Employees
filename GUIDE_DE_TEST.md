# Guide de Test et de Validation — Système de Gestion des Employés (EMS C++)

Ce document explique comment lancer et tester l'application unifiée avec tous les modules intégrés.

---

## 1. Lancement rapide

L'exécutable `GestionEmployes.exe` est déjà compilé.
Double-cliquez dessus ou exécutez dans PowerShell :

```powershell
.\GestionEmployes.exe
```

Si vous souhaitez recompiler l'application vous-même avec MinGW/GCC :
```powershell
g++ -std=c++17 -Wall -Wextra -Iheaders -Iheaders/admin -Iheaders/espaceEmploye source/*.cpp source/admin/*.cpp source/espaceEmploye/*.cpp -o GestionEmployes.exe
```

---

## 2. Scénario de test complet étape par étape

### Étape 1 : Créer un employé dans l'Espace Administrateur
1. Dans le menu principal, tapez **`1`** (Espace Administrateur).
2. Choisissez **`1. Ajouter un employe`**.
3. Renseignez les champs :
   - Nom : `Alami`
   - Prénom : `Yassine`
   - CIN : `AB123456`
   - Email : `yassine.alami@company.ma`
   - Téléphone : `0612345678`
   - Date de naissance (JJ/MM/AAAA) : `15/05/1990`
   - Date d'embauche (JJ/MM/AAAA) : `01/09/2018`
   - Salaire de base : `8500`
   *(Note : Le statut est automatiquement initialisé à "Actif" à la création).*
4. Notez bien le matricule attribué (par ex. `EMP0001`) et l'ID (par ex. `1`).
5. Tapez **`0`** pour revenir au menu principal.

---

### Étape 2 : Tester l'Espace Employé
1. Dans le menu principal, tapez **`3`** (Espace Employé).
2. Tapez **`1`** pour vous connecter, puis saisissez `EMP0001` (ou votre CIN `AB123456`).
3. Vous accédez au menu employé :
   - **Choix 1 :** Consulter votre profil complet.
   - **Choix 6 :** Déposer une demande administrative (choisissez par ex. *Attestation de travail*).
   - **Choix 7 :** Vérifier que votre demande apparaît avec le statut `[En attente]`.
   - **Choix 8 :** Déposer une demande de congé :
     - Date début : `2027-04-01`
     - Date fin : `2027-04-05`
     - Type : `1` (Annuel)
     - Consultez vos congés : la demande est créée `[EN ATTENTE]`.
4. Tapez **`0`** pour vous déconnecter et revenir au menu principal.

---

### Étape 3 : Tester l'Espace Responsable RH
1. Dans le menu principal, tapez **`2`** (Espace Responsable RH).
2. Vous accédez aux 5 volets de gestion :
   - **Choix 1 :** Consulter les demandes administratives en attente.
   - **Choix 2 :** Traiter la demande de Yassine Alami (tapez son ID et validez pour accepter).
   - **Choix 3 :** Traiter les congés :
     - Consultez les demandes en attente.
     - Validez la demande de congé déposée à l'étape 2.
     - Affichez le bilan et le solde restant.
   - **Choix 4 :** Enregistrer une absence :
     - Enregistrez une absence justifiée pour l'employé `1`.
     - Consultez l'historique des absences.
   - **Choix 5 :** Services RH :
     - **Bonification des primes :** calcule automatiquement la prime d'ancienneté selon les années de service.
     - **Estimation retraite :** calcule l'âge et la pension estimée.
     - **Formations :** ajoutez une formation et inscrivez l'employé.
3. Tapez **`0`** pour quitter l'Espace RH.

---

## 3. Persistance des données
Toutes les données validées (**employés** dans `data/employes.txt`, **demandes** dans `data/demandes.txt`, **congés** dans `data/conges.txt`, **absences** dans `data/absences.txt` et **formations** dans `data/formations.txt`) sont automatiquement sauvegardées de façon atomique et rechargées au démarrage suivant.
