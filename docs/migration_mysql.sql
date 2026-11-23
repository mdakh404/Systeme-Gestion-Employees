-- =====================================================================
-- MIGRATION : adapte gestion_employes.sql au code C++ (a executer UNE SEULE FOIS,
-- apres gestion_employes.sql).   mysql -u root -p < docs/migration_mysql.sql
-- Compatible MySQL >= 8.0.19 et MariaDB >= 10.4.
-- =====================================================================
USE gestion_employes;

-- 1. Employe : departement / poste optionnels (le C++ utilise 0 = non affecte -> NULL)
--    + statuts utilises par l'application ('inactif' = Inactif, 'supprime' = employe supprime
--      dans l'application : on ne supprime jamais physiquement, cf. trigger trg_employe_bd).
ALTER TABLE employe
  MODIFY departement_id INT UNSIGNED NULL,
  MODIFY poste_id INT UNSIGNED NULL;

ALTER TABLE employe DROP CONSTRAINT ck_employe_statut;
ALTER TABLE employe ADD CONSTRAINT ck_employe_statut CHECK (statut IN
  ('actif','inactif','en_conge','suspendu','demissionnaire','licencie','retraite','supprime'));

-- 2. Demandes administratives : type libre (Attestation de travail, Formation...) + description
ALTER TABLE demande_administrative DROP CONSTRAINT ck_demande_type;
ALTER TABLE demande_administrative MODIFY type_demande VARCHAR(50) NOT NULL;
ALTER TABLE demande_administrative ADD COLUMN description VARCHAR(255) NOT NULL DEFAULT '';

-- 3. Absences : colonne "annulee" + une absence annulee peut coexister avec une nouvelle absence
--    du meme jour (l'unicite est controlee par l'application pour les absences actives).
ALTER TABLE absence ADD COLUMN annulee BOOLEAN NOT NULL DEFAULT FALSE;
ALTER TABLE absence ADD INDEX idx_absence_employe (employe_id);
ALTER TABLE absence DROP INDEX uq_absence_employe_jour;

-- 4. Formations (module TrainingService)
CREATE TABLE IF NOT EXISTS formation (
  id              INT UNSIGNED  NOT NULL,
  titre           VARCHAR(150)  NOT NULL,
  type_formation  VARCHAR(50)   NOT NULL,
  prix            DECIMAL(10,2) NOT NULL DEFAULT 0.00,
  duree_heures    INT           NOT NULL DEFAULT 0,
  date_debut      VARCHAR(30)   NOT NULL DEFAULT '',
  CONSTRAINT pk_formation PRIMARY KEY (id)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS inscription_formation (
  employe_id    INT UNSIGNED NOT NULL,
  formation_id  INT UNSIGNED NOT NULL,
  CONSTRAINT pk_inscription_formation PRIMARY KEY (employe_id, formation_id),
  CONSTRAINT fk_inscription_employe   FOREIGN KEY (employe_id)   REFERENCES employe (id)   ON DELETE CASCADE,
  CONSTRAINT fk_inscription_formation FOREIGN KEY (formation_id) REFERENCES formation (id) ON DELETE CASCADE
) ENGINE=InnoDB;
