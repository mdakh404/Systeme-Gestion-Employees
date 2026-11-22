#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <stdexcept>

#include "Conge.h"
#include "GestionnaireConges.h"
#include "GestionnaireFichiersConge.h"
#include "MenuConge.h"

using namespace std;
namespace fs = std::filesystem;

int reussis = 0;
int echoues = 0;

void verifier(bool condition, const string& nom)
{
    if (condition) {
        reussis++;
        cout << "  [OK]    " << nom << endl;
    }
    else {
        echoues++;
        cout << "  [ECHEC] " << nom << endl;
    }
}

bool contient(const string& texte, const string& morceau)
{
    return texte.find(morceau) != string::npos;
}

// Compte les occurrences d'un morceau de texte
int compter(const string& texte, const string& morceau)
{
    int n = 0;
    size_t pos = texte.find(morceau);
    while (pos != string::npos) {
        n++;
        pos = texte.find(morceau, pos + 1);
    }
    return n;
}

// Ecrit un jeu de donnees connu dans data/conges.txt
void ecrireFichier(const string& contenu)
{
    fs::create_directories("data");
    ofstream f("data/conges.txt");
    f << contenu;
}

string lireFichier()
{
    ifstream f("data/conges.txt");
    stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Jeu de donnees de reference
const string DONNEES =
    "1;101;2026-12-01;2026-12-05;Annuel;Acceptee\n"
    "2;102;2026-12-10;2026-12-12;Maladie;En attente\n"
    "3;101;2027-01-15;2027-01-20;Sans solde;En attente\n"
    "4;103;2026-03-01;2026-03-30;Annuel;En attente\n";


string sessionEmploye(int idEmploye, const string& saisie)
{
    GestionnaireConges gc;
    istringstream entree(saisie);
    MenuConge menu(gc, entree);

    ostringstream sortie;
    streambuf* ancien = cout.rdbuf(sortie.rdbuf());
    try {
        menu.lancerEmploye(idEmploye);
    }
    catch (...) {
        cout.rdbuf(ancien);
        throw;
    }
    cout.rdbuf(ancien);
    return sortie.str();
}

string sessionRH(const string& saisie)
{
    GestionnaireConges gc;
    istringstream entree(saisie);
    MenuConge menu(gc, entree);

    ostringstream sortie;
    streambuf* ancien = cout.rdbuf(sortie.rdbuf());
    try {
        menu.lancerRH();
    }
    catch (...) {
        cout.rdbuf(ancien);
        throw;
    }
    cout.rdbuf(ancien);
    return sortie.str();
}

// ------------------------------------------------------------
void testsIndicateurs()
{
    cout << "\n=== Indicateurs de statut ===" << endl;
    verifier(MenuConge::indicateurStatut("En attente") == "[EN ATTENTE]", "I1 En attente");
    verifier(MenuConge::indicateurStatut("Acceptee") == "[ACCEPTEE]", "I2 Acceptee (sans accent)");
    verifier(MenuConge::indicateurStatut("Accept\xC3\xA9" "e") == "[ACCEPTEE]", "I3 Acceptee (avec accent)");
    verifier(MenuConge::indicateurStatut("Refusee") == "[REFUSEE]", "I4 Refusee (sans accent)");
    verifier(MenuConge::indicateurStatut("Refus\xC3\xA9" "e") == "[REFUSEE]", "I5 Refusee (avec accent)");
}

void testsEmploye()
{
    cout << "\n=== Menu Employe ===" << endl;

    // M1 : depot normal
    ecrireFichier(DONNEES);
    string s = sessionEmploye(104, "1\n2026-12-10\n2026-12-15\n1\n0\n");
    verifier(contient(s, "Demande deposee avec succes"), "M1a depot via le menu : confirmation");
    verifier(contient(lireFichier(), "5;104;2026-12-10;2026-12-15;Annuel;En attente"),
             "M1b la demande est dans conges.txt (ID 5, type choisi par numero)");

    // M2 : saisies invalides dans le menu
    ecrireFichier(DONNEES);
    s = sessionEmploye(104, "abc\n\n9\n-1\n 2 \n0\n");
    verifier(compter(s, "Saisie invalide") == 4, "M2a lettres, vide, hors plage, negatif : 4 refus");
    verifier(contient(s, "Aucune demande enregistree"), "M2b ' 2 ' (espaces) accepte : historique vide pour 104");

    // M3 : dates et type invalides => re-demande
    ecrireFichier(DONNEES);
    s = sessionEmploye(104, "1\n2026/12/10\n2026-2-5\nabc\n2026-12-10\n2026-12-15\n7\n3\n0\n");
    verifier(compter(s, "Format invalide") == 3, "M3a trois formats de date faux : re-demande");
    verifier(contient(s, "Saisie invalide"), "M3b type hors menu (7) : re-demande");
    verifier(contient(lireFichier(), "5;104;2026-12-10;2026-12-15;Sans solde;En attente"),
             "M3c depot correct ensuite (type 3 = Sans solde)");

    // M4 : refus metier de T3 : exceptions attrapees, pas de crash
    ecrireFichier(DONNEES);
    s = sessionEmploye(104, "1\n2026-02-30\n2026-03-05\n1\n"      // date inexistante
                            "1\n2026-12-20\n2026-12-10\n1\n"      // dates inversees
                            "1\n2026-12-10\n2026-12-15\n1\n"      // depot valide
                            "1\n2026-12-12\n2026-12-20\n1\n"      // chevauche la demande precedente
                            "0\n");
    verifier(compter(s, "Operation refusee") == 3, "M4a 3 refus de T3 attrapes (30 fev, inversees, chevauchement)");
    verifier(compter(lireFichier(), "104;") == 1, "M4b une seule demande enregistree pour 104");

    // M5 : annulation a chaque etape
    ecrireFichier(DONNEES);
    s = sessionEmploye(104, "1\n0\n"
                            "1\n2026-12-10\n0\n"
                            "1\n2026-12-10\n2026-12-15\n0\n"
                            "0\n");
    verifier(compter(s, "Depot annule") == 3, "M5a annulation avec 0 a chaque etape");
    verifier(lireFichier() == DONNEES, "M5b fichier inchange");

    // M6 : consultation : seulement MES demandes
    ecrireFichier(DONNEES);
    s = sessionEmploye(101, "2\n0\n");
    verifier(contient(s, "[ACCEPTEE]") && contient(s, "[EN ATTENTE]"), "M6a indicateurs de statut affiches");
    verifier(contient(s, "Sans solde") && !contient(s, "102"), "M6b seulement les demandes de 101");

    // M7 : bilan et solde (T4 : afficherStatistiques)
    ecrireFichier(DONNEES);
    s = sessionEmploye(101, "3\n0\n");
    verifier(contient(s, "17 / 22"), "M7 bilan : solde 22 - 5 jours = 17 / 22");

    // M8 : fin brutale de la saisie
    ecrireFichier(DONNEES);
    s = sessionEmploye(104, "1\n2026-12-10\n");
    verifier(lireFichier() == DONNEES, "M8a fin brutale en plein depot : pas de boucle infinie, rien enregistre");
    s = sessionEmploye(104, "");
    verifier(true, "M8b saisie vide des le debut : le menu se termine");

    // M9 : ID employe invalide
    bool exceptionLevee = false;
    try {
        sessionEmploye(0, "0\n");
    }
    catch (const invalid_argument&) {
        exceptionLevee = true;
    }
    verifier(exceptionLevee, "M9 ID employe 0 => invalid_argument");
}

void testsRH()
{
    cout << "\n=== Menu RH ===" << endl;

    // R1 : aucune demande
    ecrireFichier("1;101;2026-12-01;2026-12-05;Annuel;Acceptee\n");
    string s = sessionRH("1\n0\n");
    verifier(contient(s, "Aucune demande en attente"), "R1 aucune demande en attente");

    // R2 : accepter (type Annuel, solde previsionnel)
    ecrireFichier(DONNEES);
    s = sessionRH("1\n4\n1\n0\n");
    verifier(contient(s, "Duree demandee : 30 jour(s)"), "R2a duree affichee avant la decision");
    verifier(contient(s, "Solde previsionnel : -8 / 22") && contient(s, "ATTENTION"),
             "R2b solde previsionnel negatif => alerte quota");
    verifier(contient(lireFichier(), "4;103;2026-03-01;2026-03-30;Annuel;Accept"), "R2c statut sauvegarde dans le fichier");

    // R3 : refuser
    ecrireFichier(DONNEES);
    s = sessionRH("1\n2\n2\n0\n");
    verifier(contient(s, "Ce type de conge ne consomme pas") && contient(s, "refusee"), "R3a refus d'un conge Maladie");
    verifier(contient(lireFichier(), "2;102;2026-12-10;2026-12-12;Maladie;Refus"), "R3b statut Refuse sauvegarde");

    // R4 : ID introuvable / deja traite / annulation
    ecrireFichier(DONNEES);
    s = sessionRH("1\n99\n"            // inconnu
                  "1\n1\n"             // ID 1 deja Acceptee : pas dans la liste
                  "1\n2\n0\n"          // decision 0 = annuler
                  "0\n");
    verifier(compter(s, "introuvable ou deja traitee") == 2, "R4a ID inconnu ou deja traite : message");
    verifier(contient(s, "Traitement annule"), "R4b annulation de la decision");
    verifier(lireFichier() == DONNEES, "R4c aucune modification");

    // R5 : choix invalides dans le menu RH
    ecrireFichier(DONNEES);
    s = sessionRH("x\n9\n1\nz\n0\n0\n");
    verifier(compter(s, "Saisie invalide") >= 3, "R5 lettres / hors plage / ID non numerique => re-demande");

    // R6 : tous les conges
    ecrireFichier(DONNEES);
    s = sessionRH("2\n0\n");
    verifier(contient(s, "103") && contient(s, "[ACCEPTEE]") && contient(s, "[EN ATTENTE]"),
             "R6 consultation de tous les conges (toutes les lignes)");

    // R7 : bilan d'un employe et statistiques globales
    ecrireFichier(DONNEES);
    s = sessionRH("3\n101\n4\n0\n");
    verifier(contient(s, "EMPLOYE 101") && contient(s, "17 / 22"), "R7a bilan de l'employe 101");
    verifier(contient(s, "STATISTIQUES GLOBALES") && contient(s, "Taux d'acceptation"), "R7b statistiques globales");

    // R8 : acceptation bloquee par T4 (chevauchement avec un conge accepte)
    ecrireFichier("1;101;2026-12-01;2026-12-10;Annuel;Acceptee\n"
                  "2;101;2026-12-05;2026-12-08;Maladie;En attente\n");
    s = sessionRH("1\n2\n1\n0\n");
    verifier(contient(s, "Operation refusee") && contient(s, "chevauchement"), "R8 exception de T4 attrapee, pas de crash");
    verifier(contient(lireFichier(), "2;101;2026-12-05;2026-12-08;Maladie;En attente"), "R8b statut inchange");
}

void testsDemo()
{
    cout << "\n=== Scenario de demonstration (Etape 4 du cahier) ===" << endl;
    ecrireFichier(DONNEES);

    // 1. L'employe depose ; 2. fermeture (fin de session)
    sessionEmploye(104, "1\n2027-02-01\n2027-02-05\n1\n0\n");
    // 3. reouverture : le RH retrouve la demande et l'accepte
    string s = sessionRH("1\n5\n1\n0\n");
    verifier(contient(s, "2027-02-01") && contient(s, "acceptee"), "D1 apres redemarrage, le RH retrouve et accepte la demande");
    // 4. l'employe verifie son historique
    s = sessionEmploye(104, "2\n0\n");
    verifier(contient(s, "[ACCEPTEE]"), "D2 l'employe voit [ACCEPTEE] dans son historique");

    // 5. regle metier T3 : un nouveau depot qui chevauche un conge ACCEPTE est bloque
    s = sessionEmploye(104, "1\n2027-02-03\n2027-02-04\n1\n0\n");
    verifier(contient(s, "Operation refusee"), "D3 chevauchement avec un conge accepte : bloque");
}

void testsRobustesse()
{
    cout << "\n=== Robustesse (dates mal formees, fichier corrompu) ===" << endl;

    ecrireFichier(DONNEES);
    GestionnaireConges gc;
    const char* mauvaisesDates[] = { "2026-1-5", "2026/12/10", "2026-12-1", "abcd-ef-gh", "" };
    int refus = 0;
    for (const char* d : mauvaisesDates) {
        try { gc.deposerDemande(104, d, "2026-12-20", "Annuel"); }
        catch (const invalid_argument&) { refus++; }
    }
    verifier(refus == 5, "B1 dates mal formees refusees par le metier (sans zeros, mauvais separateur, texte, vide)");
    verifier(lireFichier() == DONNEES, "B2 rien n'est enregistre apres ces refus");

    ecrireFichier("ligne_pourrie\n");
    bool leve = false;
    try { GestionnaireConges g2; }
    catch (const runtime_error&) { leve = true; }
    verifier(leve, "B3 ligne corrompue => runtime_error (pas de crash silencieux)");

    ecrireFichier("abc;1;2026-01-01;2026-01-02;Annuel;En attente\n");
    leve = false;
    try { GestionnaireConges g3; }
    catch (const runtime_error&) { leve = true; }
    verifier(leve, "B4 ID non numerique dans le fichier => runtime_error");
}

void testsRapportQA()
{
    cout << "\n=== Corrections du rapport QA ===" << endl;

    // Q1 : le solde est calcule par annee civile
    ecrireFichier("1;50;2026-03-01;2026-03-22;Annuel;Acceptée\n");   // 22 jours en 2026
    {
        GestionnaireConges gc;
        verifier(gc.soldeRestant(50, 2026) == 0,  "Q1a 22 jours pris en 2026 => solde 2026 = 0");
        verifier(gc.soldeRestant(50, 2027) == 22, "Q1b solde 2027 remis a 22");
    }

    // Q2 : un conge a cheval sur deux annees est ventile
    ecrireFichier("1;51;2026-12-30;2027-01-02;Annuel;Acceptée\n");
    {
        GestionnaireConges gc;
        verifier(gc.soldeRestant(51, 2026) == 20 && gc.soldeRestant(51, 2027) == 20,
                 "Q2 conge a cheval : 2 jours en 2026, 2 jours en 2027");
    }

    // Q3 : fichier syntaxiquement correct mais semantiquement invalide => refus au demarrage
    const char* invalides[] = {
        "1;1;2026-12-05;2026-12-01;Annuel;Acceptée\n",   // dates inversees
        "1;1;2026-02-31;2026-03-02;Annuel;Acceptée\n",   // date inexistante
        "1;1;2026-12-01;2026-12-05;Foo;Acceptée\n",      // type inconnu
        "1;1;2026-12-01;2026-12-05;Annuel;Bar\n",        // statut inconnu
        "1;0;2026-12-01;2026-12-05;Annuel;Acceptée\n"    // employe invalide
    };
    int rejets = 0;
    for (const char* ligne : invalides) {
        ecrireFichier(ligne);
        try { GestionnaireConges g; }
        catch (const runtime_error&) { rejets++; }
    }
    verifier(rejets == 5, "Q3 5 types de lignes invalides rejetees au chargement");

    // Q4 : echec d'ecriture disque => pas de demande fantome en memoire
    ecrireFichier(DONNEES);
    {
        GestionnaireConges gc;
        size_t avant = gc.tousLesConges().size();
        fs::remove("data/conges.txt");
        fs::create_directory("data/conges.txt");      // l'ecriture va echouer
        bool leve = false;
        try { gc.deposerDemande(104, "2030-01-01", "2030-01-03", "Annuel"); }
        catch (const runtime_error&) { leve = true; }
        verifier(leve && gc.tousLesConges().size() == avant, "Q4a depot : erreur levee + rollback memoire");

        leve = false;
        try { gc.traiterDemande(2, true); }
        catch (const runtime_error&) { leve = true; }
        bool toujoursEnAttente = false;
        for (const Conge& c : gc.tousLesConges())
            if (c.Getid() == 2) toujoursEnAttente = (c.Getstatut() == "En attente");
        verifier(leve && toujoursEnAttente, "Q4b traitement : erreur levee + statut restaure");

        fs::remove("data/conges.txt");
    }
    ecrireFichier(DONNEES);
}

int main()
{
    fs::path ancienDossier = fs::current_path();
    fs::path dossierTest = fs::temp_directory_path() / "test_conges_T5";
    fs::remove_all(dossierTest);
    fs::create_directories(dossierTest);
    fs::current_path(dossierTest);

    testsIndicateurs();
    testsEmploye();
    testsRH();
    testsDemo();
    testsRobustesse();
    testsRapportQA();

    fs::current_path(ancienDossier);
    fs::remove_all(dossierTest);

    cout << "\n==============================" << endl;
    cout << "Reussis : " << reussis << "   Echoues : " << echoues << endl;
    return echoues == 0 ? 0 : 1;
}
