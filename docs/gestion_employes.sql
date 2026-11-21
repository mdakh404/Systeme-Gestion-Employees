CREATE DATABASE IF NOT EXISTS gestion_employes
  CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE gestion_employes;
SET NAMES utf8mb4;

-- ---------------------------------------------------------------------
-- Classe Personne (classe mère)
-- ---------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS personne (
  id              INT UNSIGNED NOT NULL AUTO_INCREMENT,
  nom             VARCHAR(50)  NOT NULL,
  prenom          VARCHAR(50)  NOT NULL,
  cin             VARCHAR(20)  NOT NULL,
  date_naissance  DATE         NOT NULL,
  email           VARCHAR(100) NOT NULL,
  telephone       VARCHAR(20)  NULL,
  CONSTRAINT pk_personne           PRIMARY KEY (id),
  CONSTRAINT uq_personne_cin       UNIQUE (cin),
  CONSTRAINT uq_personne_email     UNIQUE (email),
  CONSTRAINT ck_personne_naissance CHECK (date_naissance >= '1900-01-01'),
  CONSTRAINT ck_personne_email     CHECK (email LIKE '%_@_%._%'),
  CONSTRAINT ck_personne_telephone CHECK (telephone IS NULL OR telephone REGEXP '^[0-9+ ().-]{6,20}$'),
  INDEX idx_personne_nom_prenom (nom, prenom),
  INDEX idx_personne_date_naissance (date_naissance)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Classe Departement
-- ---------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS departement (
  id           INT UNSIGNED NOT NULL AUTO_INCREMENT,
  nom          VARCHAR(100) NOT NULL,
  description  VARCHAR(255) NULL,
  CONSTRAINT pk_departement     PRIMARY KEY (id),
  CONSTRAINT uq_departement_nom UNIQUE (nom)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Classe Poste
-- ---------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS poste (
  id               INT UNSIGNED NOT NULL AUTO_INCREMENT,
  intitule         VARCHAR(100) NOT NULL,
  responsabilites  TEXT         NULL,
  CONSTRAINT pk_poste          PRIMARY KEY (id),
  CONSTRAINT uq_poste_intitule UNIQUE (intitule)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Classe Employe (hérite de Personne)
-- ---------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS employe (
  id              INT UNSIGNED NOT NULL,
  matricule       VARCHAR(20)  NOT NULL,
  date_embauche   DATE         NOT NULL,
  statut          VARCHAR(20)  NOT NULL DEFAULT 'actif',
  departement_id  INT UNSIGNED NOT NULL,
  poste_id        INT UNSIGNED NOT NULL,
  CONSTRAINT pk_employe             PRIMARY KEY (id),
  CONSTRAINT uq_employe_matricule   UNIQUE (matricule),
  CONSTRAINT ck_employe_statut      CHECK (statut IN ('actif','en_conge','suspendu','demissionnaire','licencie','retraite')),
  CONSTRAINT ck_employe_embauche    CHECK (date_embauche >= '1950-01-01'),
  CONSTRAINT fk_employe_personne    FOREIGN KEY (id)             REFERENCES personne (id)    ON DELETE CASCADE,
  CONSTRAINT fk_employe_departement FOREIGN KEY (departement_id) REFERENCES departement (id) ON DELETE RESTRICT,
  CONSTRAINT fk_employe_poste       FOREIGN KEY (poste_id)       REFERENCES poste (id)       ON DELETE RESTRICT,
  INDEX idx_employe_date_embauche (date_embauche),
  INDEX idx_employe_statut (statut)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Classe Administrateur (hérite de Employe)
-- ---------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS administrateur (
  id                 INT UNSIGNED NOT NULL,
  login              VARCHAR(50)  NOT NULL,
  -- SÉCURITÉ : stocker UNIQUEMENT un hash (bcrypt/argon2), jamais le mot de passe en clair.
  mot_de_passe_hash  VARCHAR(255) NOT NULL,
  CONSTRAINT pk_administrateur       PRIMARY KEY (id),
  CONSTRAINT uq_administrateur_login UNIQUE (login),
  CONSTRAINT ck_administrateur_login CHECK (CHAR_LENGTH(login) >= 3),
  CONSTRAINT fk_administrateur_employe FOREIGN KEY (id) REFERENCES employe (id) ON DELETE CASCADE
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Classe ResponsableRH (hérite de Employe)
-- ---------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS responsable_rh (
  id                 INT UNSIGNED NOT NULL,
  login              VARCHAR(50)  NOT NULL,
  mot_de_passe_hash  VARCHAR(255) NOT NULL,
  CONSTRAINT pk_responsable_rh       PRIMARY KEY (id),
  CONSTRAINT uq_responsable_rh_login UNIQUE (login),
  CONSTRAINT ck_responsable_rh_login CHECK (CHAR_LENGTH(login) >= 3),
  CONSTRAINT fk_responsable_rh_employe FOREIGN KEY (id) REFERENCES employe (id) ON DELETE CASCADE
) ENGINE=InnoDB;
-- ---------------------------------------------------------------------
-- Classe Salaire
-- ---------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS salaire (
  employe_id        INT UNSIGNED  NOT NULL,
  salaire_base      DECIMAL(10,2) NOT NULL,
  prime_anciennete  DECIMAL(10,2) NOT NULL DEFAULT 0.00,
  CONSTRAINT pk_salaire          PRIMARY KEY (employe_id),
  CONSTRAINT ck_salaire_base     CHECK (salaire_base >= 0),
  CONSTRAINT ck_salaire_prime    CHECK (prime_anciennete >= 0),
  CONSTRAINT fk_salaire_employe  FOREIGN KEY (employe_id) REFERENCES employe (id) ON DELETE CASCADE
) ENGINE=InnoDB;


CREATE TABLE IF NOT EXISTS historique_salaire (
  id                INT UNSIGNED  NOT NULL AUTO_INCREMENT,
  employe_id        INT UNSIGNED  NOT NULL,
  operation         VARCHAR(15)   NOT NULL,
  salaire_base      DECIMAL(10,2) NOT NULL,
  prime_anciennete  DECIMAL(10,2) NOT NULL,
  date_effet        DATETIME      NOT NULL DEFAULT CURRENT_TIMESTAMP,
  CONSTRAINT pk_historique_salaire   PRIMARY KEY (id),
  CONSTRAINT ck_historique_operation CHECK (operation IN ('creation','modification','suppression')),
  CONSTRAINT ck_historique_base      CHECK (salaire_base >= 0),
  CONSTRAINT ck_historique_prime     CHECK (prime_anciennete >= 0),
  CONSTRAINT fk_historique_employe   FOREIGN KEY (employe_id) REFERENCES employe (id) ON DELETE RESTRICT,
  INDEX idx_historique_employe_date (employe_id, date_effet),
  INDEX idx_historique_date (date_effet)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Classe Conge
-- ---------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS conge (
  id          INT UNSIGNED NOT NULL AUTO_INCREMENT,
  employe_id  INT UNSIGNED NOT NULL,
  date_debut  DATE         NOT NULL,
  date_fin    DATE         NOT NULL,
  type_conge  VARCHAR(20)  NOT NULL,
  statut      VARCHAR(20)  NOT NULL DEFAULT 'en_attente',
  CONSTRAINT pk_conge         PRIMARY KEY (id),
  CONSTRAINT ck_conge_dates   CHECK (date_fin >= date_debut),
  CONSTRAINT ck_conge_type    CHECK (type_conge IN ('annuel','maladie','maternite','paternite','sans_solde','exceptionnel')),
  CONSTRAINT ck_conge_statut  CHECK (statut IN ('en_attente','approuve','refuse','annule')),
  CONSTRAINT fk_conge_employe FOREIGN KEY (employe_id) REFERENCES employe (id) ON DELETE RESTRICT,
  INDEX idx_conge_dates (date_debut, date_fin),
  INDEX idx_conge_statut (statut)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Classe Absence 
-- ---------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS absence (
  id            INT UNSIGNED NOT NULL AUTO_INCREMENT,
  employe_id    INT UNSIGNED NOT NULL,
  date_absence  DATE         NOT NULL,
  motif         VARCHAR(255) NULL,
  justifiee     BOOLEAN      NOT NULL DEFAULT FALSE,
  CONSTRAINT pk_absence              PRIMARY KEY (id),
  CONSTRAINT uq_absence_employe_jour UNIQUE (employe_id, date_absence),
  CONSTRAINT ck_absence_justifiee    CHECK (justifiee IN (0,1)),
  CONSTRAINT fk_absence_employe      FOREIGN KEY (employe_id) REFERENCES employe (id) ON DELETE RESTRICT,
  INDEX idx_absence_date (date_absence)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Classe DemandeAdministrative 
-- ---------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS demande_administrative (
  id            INT UNSIGNED NOT NULL AUTO_INCREMENT,
  employe_id    INT UNSIGNED NOT NULL,
  type_demande  VARCHAR(30)  NOT NULL,
  date_depot    DATE         NOT NULL DEFAULT (CURRENT_DATE),
  statut        VARCHAR(20)  NOT NULL DEFAULT 'en_attente',
  CONSTRAINT pk_demande_administrative PRIMARY KEY (id),
  CONSTRAINT ck_demande_type   CHECK (type_demande IN ('attestation_travail','attestation_salaire','releve_conges','changement_infos','autre')),
  CONSTRAINT ck_demande_statut CHECK (statut IN ('en_attente','en_cours','traitee','rejetee')),
  CONSTRAINT fk_demande_employe FOREIGN KEY (employe_id) REFERENCES employe (id) ON DELETE RESTRICT,
  INDEX idx_demande_date_depot (date_depot),
  INDEX idx_demande_statut (statut)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Vue : comptes de connexion autorisés.
--   L'authentification doit passer par cette vue : un employé désactivé
--   (demissionnaire, licencie, retraite) ou suspendu n'a plus d'accès.
--   Seuls les statuts 'actif' et 'en_conge' peuvent se connecter.
-- ---------------------------------------------------------------------
CREATE OR REPLACE VIEW v_comptes_actifs AS
  SELECT a.id AS employe_id, a.login, a.mot_de_passe_hash, 'administrateur' AS role_compte
  FROM administrateur a
  JOIN employe e ON e.id = a.id
  WHERE e.statut IN ('actif', 'en_conge')
  UNION ALL
  SELECT r.id, r.login, r.mot_de_passe_hash, 'responsable_rh'
  FROM responsable_rh r
  JOIN employe e ON e.id = r.id
  WHERE e.statut IN ('actif', 'en_conge');

-- =====================================================================
-- 2. TRIGGERS
-- =====================================================================
DELIMITER $$

-- ---- Historique des salaires : création, modification ET suppression ----
DROP TRIGGER IF EXISTS trg_salaire_ai $$
CREATE TRIGGER trg_salaire_ai AFTER INSERT ON salaire
FOR EACH ROW
BEGIN
  INSERT INTO historique_salaire (employe_id, operation, salaire_base, prime_anciennete)
  VALUES (NEW.employe_id, 'creation', NEW.salaire_base, NEW.prime_anciennete);
END$$

DROP TRIGGER IF EXISTS trg_salaire_au $$
CREATE TRIGGER trg_salaire_au AFTER UPDATE ON salaire
FOR EACH ROW
BEGIN
  IF NOT (OLD.salaire_base <=> NEW.salaire_base)
     OR NOT (OLD.prime_anciennete <=> NEW.prime_anciennete) THEN
    INSERT INTO historique_salaire (employe_id, operation, salaire_base, prime_anciennete)
    VALUES (NEW.employe_id, 'modification', NEW.salaire_base, NEW.prime_anciennete);
  END IF;
END$$

-- on garde une trace de la dernière valeur avant suppression.
DROP TRIGGER IF EXISTS trg_salaire_ad $$
CREATE TRIGGER trg_salaire_ad AFTER DELETE ON salaire
FOR EACH ROW
BEGIN
  INSERT INTO historique_salaire (employe_id, operation, salaire_base, prime_anciennete)
  VALUES (OLD.employe_id, 'suppression', OLD.salaire_base, OLD.prime_anciennete);
END$$

-- ---- Rôles EXCLUSIFS + unicité du login entre administrateur et responsable_rh ----

DROP TRIGGER IF EXISTS trg_admin_login_bi $$
CREATE TRIGGER trg_admin_login_bi BEFORE INSERT ON administrateur
FOR EACH ROW
BEGIN
  -- [ROLES EXCLUSIFS]
  IF EXISTS (SELECT 1 FROM responsable_rh WHERE id = NEW.id) THEN
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Roles exclusifs : cet employe est deja responsable RH';
  END IF;
  -- [LOGIN UNIQUE]
  IF EXISTS (SELECT 1 FROM responsable_rh WHERE login = NEW.login) THEN
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Login deja utilise par un responsable RH';
  END IF;
END$$

DROP TRIGGER IF EXISTS trg_admin_login_bu $$
CREATE TRIGGER trg_admin_login_bu BEFORE UPDATE ON administrateur
FOR EACH ROW
BEGIN
  -- [ROLES EXCLUSIFS]
  IF NEW.id <> OLD.id AND EXISTS (SELECT 1 FROM responsable_rh WHERE id = NEW.id) THEN
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Roles exclusifs : cet employe est deja responsable RH';
  END IF;
  -- [LOGIN UNIQUE]
  IF NEW.login <> OLD.login
     AND EXISTS (SELECT 1 FROM responsable_rh WHERE login = NEW.login) THEN
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Login deja utilise par un responsable RH';
  END IF;
END$$

DROP TRIGGER IF EXISTS trg_rh_login_bi $$
CREATE TRIGGER trg_rh_login_bi BEFORE INSERT ON responsable_rh
FOR EACH ROW
BEGIN
  -- [ROLES EXCLUSIFS]
  IF EXISTS (SELECT 1 FROM administrateur WHERE id = NEW.id) THEN
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Roles exclusifs : cet employe est deja administrateur';
  END IF;
  -- [LOGIN UNIQUE]
  IF EXISTS (SELECT 1 FROM administrateur WHERE login = NEW.login) THEN
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Login deja utilise par un administrateur';
  END IF;
END$$

DROP TRIGGER IF EXISTS trg_rh_login_bu $$
CREATE TRIGGER trg_rh_login_bu BEFORE UPDATE ON responsable_rh
FOR EACH ROW
BEGIN
  -- [ROLES EXCLUSIFS]
  IF NEW.id <> OLD.id AND EXISTS (SELECT 1 FROM administrateur WHERE id = NEW.id) THEN
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Roles exclusifs : cet employe est deja administrateur';
  END IF;
  -- [LOGIN UNIQUE]
  IF NEW.login <> OLD.login
     AND EXISTS (SELECT 1 FROM administrateur WHERE login = NEW.login) THEN
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Login deja utilise par un administrateur';
  END IF;
END$$

-- ---- Pas de suppression physique d'un employé : on le désactive ----
DROP TRIGGER IF EXISTS trg_employe_bd $$
CREATE TRIGGER trg_employe_bd BEFORE DELETE ON employe
FOR EACH ROW
BEGIN
  IF COALESCE(@autoriser_purge_employes, 0) <> 1 THEN
    SIGNAL SQLSTATE '45000'
      SET MESSAGE_TEXT = 'Suppression physique interdite : utiliser sp_desactiver_employe()';
  END IF;
END$$

-- Ferme la porte dérobée : DELETE sur personne cascaderait vers employe SANS
-- déclencher trg_employe_bd (les actions de FK ne déclenchent pas de triggers).
DROP TRIGGER IF EXISTS trg_personne_bd $$
CREATE TRIGGER trg_personne_bd BEFORE DELETE ON personne
FOR EACH ROW
BEGIN
  IF EXISTS (SELECT 1 FROM employe WHERE id = OLD.id) THEN
    SIGNAL SQLSTATE '45000'
      SET MESSAGE_TEXT = 'Cette personne est un employe : utiliser sp_desactiver_employe()';
  END IF;
END$$

-- =====================================================================
-- 3. PROCÉDURES DE RÉINITIALISATION
-- =====================================================================

DROP PROCEDURE IF EXISTS sp_vider_donnees $$
CREATE PROCEDURE sp_vider_donnees()
BEGIN
  DECLARE EXIT HANDLER FOR SQLEXCEPTION
  BEGIN
    ROLLBACK;
    SET @autoriser_purge_employes = 0;
    RESIGNAL;
  END;

  SET @autoriser_purge_employes = 1;   -- lève le garde-fou de trg_employe_bd

  START TRANSACTION;
    DELETE FROM demande_administrative WHERE id >= 0;
    DELETE FROM absence                WHERE id >= 0;
    DELETE FROM conge                  WHERE id >= 0;
    DELETE FROM responsable_rh         WHERE id >= 0;
    DELETE FROM administrateur         WHERE id >= 0;
    DELETE FROM salaire                WHERE employe_id >= 0;  -- le trigger alimente l'historique...
    DELETE FROM historique_salaire     WHERE id >= 0;          -- ...qu'on vide donc juste après
    DELETE FROM employe                WHERE id >= 0;
    DELETE FROM poste                  WHERE id >= 0;
    DELETE FROM departement            WHERE id >= 0;
    DELETE FROM personne               WHERE id >= 0;
  COMMIT;

  SET @autoriser_purge_employes = 0;

  ALTER TABLE demande_administrative AUTO_INCREMENT = 1;
  ALTER TABLE absence                AUTO_INCREMENT = 1;
  ALTER TABLE conge                  AUTO_INCREMENT = 1;
  ALTER TABLE historique_salaire     AUTO_INCREMENT = 1;
  ALTER TABLE poste                  AUTO_INCREMENT = 1;
  ALTER TABLE departement            AUTO_INCREMENT = 1;
  ALTER TABLE personne               AUTO_INCREMENT = 1;
END$$

-- ---- 3.2 Charge le jeu de données de test (refuse de s'exécuter sur une base non vide) ----
DROP PROCEDURE IF EXISTS sp_charger_jeu_test $$
CREATE PROCEDURE sp_charger_jeu_test()
BEGIN
  IF EXISTS (SELECT 1 FROM personne) THEN
    SIGNAL SQLSTATE '45000'
      SET MESSAGE_TEXT = 'Base non vide : appelez sp_reinitialiser() pour repartir de zero';
  END IF;

  INSERT INTO personne (id, nom, prenom, cin, date_naissance, email, telephone) VALUES
    (1, 'Alaoui',     'Karim',  'AB123456', '1985-04-12', 'karim.alaoui@exemple.ma',      '0661000001'),
    (2, 'Benani',     'Salma',  'CD234567', '1988-09-30', 'salma.benani@exemple.ma',      '0661000002'),
    (3, 'El Idrissi', 'Youssef','EF345678', '1992-01-20', 'youssef.elidrissi@exemple.ma', '0661000003'),
    (4, 'Tazi',       'Lina',   'GH456789', '1995-06-08', 'lina.tazi@exemple.ma',         '0661000004'),
    (5, 'Mansouri',   'Omar',   'IJ567890', '1998-11-15', 'omar.mansouri@exemple.ma',     NULL);

  INSERT INTO departement (id, nom, description) VALUES
    (1, 'Informatique',        'Développement et infrastructure'),
    (2, 'Ressources humaines', 'Gestion du personnel'),
    (3, 'Finance',             'Comptabilité et paie');

  INSERT INTO poste (id, intitule, responsabilites) VALUES
    (1, 'Développeur',            'Conception et développement logiciel'),
    (2, 'Responsable RH',         'Gestion des congés, absences et dossiers du personnel'),
    (3, 'Comptable',              'Tenue des comptes et paie'),
    (4, 'Administrateur système', 'Administration des serveurs et des accès');

  INSERT INTO employe (id, matricule, date_embauche, statut, departement_id, poste_id) VALUES
    (1, 'EMP001', '2015-03-01', 'actif', 1, 4),
    (2, 'EMP002', '2016-09-15', 'actif', 2, 2),
    (3, 'EMP003', '2019-02-01', 'actif', 1, 1),
    (4, 'EMP004', '2021-06-01', 'actif', 3, 3),
    (5, 'EMP005', '2024-01-08', 'actif', 1, 1);

  -- Les hash ci-dessous sont factices (à remplacer par de vrais hash bcrypt/argon2).
  INSERT INTO administrateur (id, login, mot_de_passe_hash) VALUES (1, 'admin.karim', '$2b$12$hash_factice_administrateur');
  INSERT INTO responsable_rh (id, login, mot_de_passe_hash) VALUES (2, 'rh.salma',    '$2b$12$hash_factice_responsable_rh');

  -- Les INSERT dans salaire alimentent historique_salaire via le trigger ('creation').
  INSERT INTO salaire (employe_id, salaire_base, prime_anciennete) VALUES
    (1, 15000.00, 1500.00),
    (2, 14000.00, 1200.00),
    (3,  9000.00,  300.00),
    (4,  8500.00,  500.00),
    (5,  7500.00,    0.00);

  -- Deux augmentations : une ligne 'modification' d'historique pour chacune.
  UPDATE salaire SET salaire_base = 9800.00, prime_anciennete = 450.00 WHERE employe_id = 3;
  UPDATE salaire SET salaire_base = 9200.00 WHERE employe_id = 4;

  INSERT INTO conge (employe_id, date_debut, date_fin, type_conge, statut) VALUES
    (3, '2026-07-01', '2026-07-15', 'annuel',     'approuve'),
    (4, '2026-08-10', '2026-08-14', 'maladie',    'approuve'),
    (3, '2026-12-22', '2026-12-31', 'annuel',     'en_attente'),
    (5, '2026-09-01', '2026-09-05', 'sans_solde', 'refuse'),
    (2, '2026-11-02', '2026-11-06', 'annuel',     'approuve'),
    -- Congés à cheval sur deux années (pour tester le calcul au prorata) :
    (1, '2026-12-28', '2027-01-04', 'annuel',     'approuve'),   -- 4 j en 2026 + 4 j en 2027
    (4, '2025-12-29', '2026-01-03', 'annuel',     'approuve');   -- 3 j en 2025 + 3 j en 2026

  INSERT INTO absence (employe_id, date_absence, motif, justifiee) VALUES
    (3, '2026-03-10', 'Rendez-vous médical', TRUE),
    (3, '2026-05-04', NULL,                  FALSE),
    (4, '2026-05-04', 'Panne de transport',  FALSE),
    (5, '2026-06-15', NULL,                  FALSE),
    (5, '2026-09-21', NULL,                  FALSE),
    (5, '2026-10-12', 'Raisons familiales',  TRUE);

  INSERT INTO demande_administrative (employe_id, type_demande, date_depot, statut) VALUES
    (3, 'attestation_travail',  '2026-10-05', 'traitee'),
    (4, 'attestation_salaire',  '2026-10-20', 'en_cours'),
    (5, 'attestation_travail',  '2026-11-10', 'en_attente'),
    (3, 'releve_conges',        '2026-11-12', 'en_attente');
END$$

-- ---- 3.3 Remise à zéro complète : vide puis recharge le jeu de test ----
DROP PROCEDURE IF EXISTS sp_reinitialiser $$
CREATE PROCEDURE sp_reinitialiser()
BEGIN
  CALL sp_vider_donnees();
  CALL sp_charger_jeu_test();
END$$

-- ---- 3.4 Désactivation d'un employé (remplace la suppression physique) ----
-- Conserve l'employé et toutes ses traces (salaire, historique, congés, absences,
-- demandes). Il disparaît de v_comptes_actifs : plus d'accès à l'application.
-- Réactivation : UPDATE employe SET statut = 'actif' WHERE id = ...;
DROP PROCEDURE IF EXISTS sp_desactiver_employe $$
CREATE PROCEDURE sp_desactiver_employe(IN p_employe_id INT UNSIGNED, IN p_statut VARCHAR(20))
BEGIN
  IF p_statut IS NULL OR p_statut NOT IN ('demissionnaire', 'licencie', 'retraite') THEN
    SIGNAL SQLSTATE '45000'
      SET MESSAGE_TEXT = 'Statut de sortie invalide : demissionnaire, licencie ou retraite';
  END IF;
  IF NOT EXISTS (SELECT 1 FROM employe WHERE id = p_employe_id) THEN
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Employe introuvable';
  END IF;
  UPDATE employe SET statut = p_statut WHERE id = p_employe_id;
END$$

DELIMITER ;

-- Fin de script : base VIDE (aucune donnée de test). Le script reste ré-exécutable.
-- Pour recharger le jeu de test plus tard : CALL sp_reinitialiser();
CALL sp_vider_donnees();
