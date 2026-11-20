# Gestion des Congés (C++17, console)

Application console : un **Employé** dépose des demandes de congé, le **Responsable RH** les accepte ou les refuse. Les données sont sauvegardées dans `data/conges.txt`.

## Lancer

Prérequis : `g++` (C++17) et `make`. À exécuter **depuis ce dossier** (le chemin `data/` est relatif).

```bash
make          # compile l'application et les tests
make run      # lance l'application
make test     # lance les 44 tests automatiques (attendu : "Echoues : 0")
make clean
```

Sans `make` : `g++ -std=c++17 -Iinclude main.cpp src/*.cpp -o gestion_conges`

## Utilisation

Menu principal → **1. Espace Employé** (saisir un ID > 0) ou **2. Espace RH**. `0` = retour / annuler à chaque étape.

- **Employé** : déposer une demande (dates `AAAA-MM-JJ`, type choisi par numéro) · consulter ses congés · voir son bilan et son solde.
- **RH** : traiter les demandes en attente (accepter / refuser) · consulter tous les congés · bilan d'un employé · statistiques globales.

Données de départ : employés `101`, `102`, `104` et `1` ; demandes en attente : n° 2, 3 et 4.

## Règles à vérifier

| Règle | Résultat attendu |
|---|---|
| Date inexistante ou mal formée (`2026-02-30`, `2026-1-5`, `abc`) | Refusée, redemandée |
| Début > fin | « Operation refusee » |
| Type | Annuel, Maladie, Sans solde, Exceptionnel |
| Statut initial d'une demande | `[EN ATTENTE]` |
| Chevauchement avec une demande *en attente* ou *acceptée* du même employé | Dépôt refusé |
| Accepter une demande qui chevauche un congé déjà accepté | Refusé, statut inchangé |
| Traiter une demande déjà traitée | Refusé |
| Quota annuel | 22 jours ; seuls les congés **Annuel** acceptés le consomment ; alerte RH si dépassé |
| Redémarrage de l'application | Toutes les données sont conservées |
| Saisie invalide (lettres, vide, hors plage, négatif) | Message d'erreur, pas de plantage |

## Scénario de test conseillé

1. Employé `104` : déposer `2027-02-01` → `2027-02-05`, type 1 (Annuel). Une confirmation avec l'ID s'affiche.
2. Quitter, puis relancer l'application.
3. RH → *Traiter les demandes* → choisir l'ID de la demande → vérifier la durée (5 jours) et le solde prévisionnel (17 / 22) → **1. Accepter**.
4. Employé `104` → *Consulter mes congés* : la demande est `[ACCEPTEE]`.
5. Employé `104` : redéposer `2027-02-03` → `2027-02-04` : refusé (chevauchement).
6. Vérifier que `data/conges.txt` contient la ligne `…;2027-02-01;2027-02-05;Annuel;Acceptée`.

## Format de `data/conges.txt`

`id;idEmploye;dateDebut;dateFin;type;statut`, une ligne par congé, par exemple :
`1;101;2026-12-01;2026-12-05;Annuel;Acceptée`

Le fichier est créé automatiquement s'il n'existe pas. Pour revenir aux données de départ, restaurez la copie d'origine du fichier. Si une ligne est invalide, l'application s'arrête avec un message clair (code de sortie 1) et ne modifie pas le fichier.
