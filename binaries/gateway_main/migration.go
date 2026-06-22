// Legacy --database flag handling and data-dir layout warnings for operators
// migrating from the recidiviz/goccy single-file SQLite catalog to this
// emulator's directory-based DuckDB layout.
package main

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

const legacyDatabaseMigrationDoc = "docs/REST_API.md#persistence-and-data-dir"

// applyLegacyDatabaseFlag maps the removed recidiviz/goccy --database=file.db
// flag onto --data-dir=<parent> and appends an actionable deprecation warning.
func applyLegacyDatabaseFlag(cfg *Config) error {
	if cfg.LegacyDatabase == "" {
		return nil
	}
	if cfg.DataDir != "" {
		return fmt.Errorf(
			"cannot use both --database and --data-dir; replace --database=%q with --data-dir=%q (see %s)",
			cfg.LegacyDatabase,
			filepath.Dir(cfg.LegacyDatabase),
			legacyDatabaseMigrationDoc,
		)
	}
	cfg.DataDir = filepath.Dir(cfg.LegacyDatabase)
	cfg.StartupWarnings = append(cfg.StartupWarnings,
		fmt.Sprintf(
			"DEPRECATED: --database is removed. The recidiviz/goccy fork stored catalog state "+
				"in a single SQLite file (%q); this emulator persists under a directory "+
				"(--data-dir) with catalog.duckdb and sidecar parquet/meta.json files. "+
				"Mapped --database -> --data-dir=%q. Data in the old single-file format is "+
				"not automatically loaded; mount the volume at --data-dir and migrate or "+
				"re-seed if needed. See %s.",
			cfg.LegacyDatabase,
			cfg.DataDir,
			legacyDatabaseMigrationDoc,
		),
	)
	return nil
}

// collectDataDirLayoutWarnings scans an on-disk data-dir for layouts that
// suggest an operator pointed --data-dir at a legacy single-file catalog or
// left orphaned SQLite files on a shared volume after switching flags.
func collectDataDirLayoutWarnings(dataDir string) []string {
	if dataDir == "" {
		return nil
	}
	info, err := os.Stat(dataDir)
	if err != nil {
		if os.IsNotExist(err) {
			return nil
		}
		return []string{fmt.Sprintf("WARN: cannot stat --data-dir %q: %v", dataDir, err)}
	}
	if !info.IsDir() {
		return []string{
			fmt.Sprintf(
				"ERROR: --data-dir %q is a file, not a directory. The recidiviz/goccy "+
					"--database=/path/catalog.db flag pointed at a single SQLite file; "+
					"this emulator expects --data-dir=/parent/directory. See %s.",
				dataDir,
				legacyDatabaseMigrationDoc,
			),
		}
	}
	hasCatalog, legacyDB, readErr := scanDataDirRoot(dataDir)
	if readErr != "" {
		return []string{readErr}
	}
	return legacyDataDirWarnings(dataDir, hasCatalog, legacyDB)
}

func scanDataDirRoot(dataDir string) (hasCatalog bool, legacyDB []string, readErr string) {
	entries, err := os.ReadDir(dataDir)
	if err != nil {
		return false, nil, fmt.Sprintf("WARN: cannot read --data-dir %q: %v", dataDir, err)
	}
	for _, e := range entries {
		if e.IsDir() {
			continue
		}
		name := e.Name()
		if name == "catalog.duckdb" {
			hasCatalog = true
			continue
		}
		lower := strings.ToLower(name)
		if strings.HasSuffix(lower, ".db") ||
			strings.HasSuffix(lower, ".sqlite") ||
			strings.HasSuffix(lower, ".sqlite3") {
			legacyDB = append(legacyDB, name)
		}
	}
	return hasCatalog, legacyDB, ""
}

func legacyDataDirWarnings(dataDir string, hasCatalog bool, legacyDB []string) []string {
	if len(legacyDB) == 0 {
		return nil
	}
	if hasCatalog {
		return []string{
			fmt.Sprintf(
				"WARN: --data-dir %q contains legacy single-file database(s) %v alongside "+
					"catalog.duckdb; the old SQLite files are ignored. Safe to delete after "+
					"confirming catalog.duckdb has your data.",
				dataDir,
				legacyDB,
			),
		}
	}
	return []string{
		fmt.Sprintf(
			"WARN: --data-dir %q contains file(s) %v that look like the recidiviz/goccy "+
				"single-file SQLite catalog, but no catalog.duckdb from this emulator. "+
				"State from the old format is not loaded automatically; point --data-dir "+
				"at an empty directory or re-seed. See %s.",
			dataDir,
			legacyDB,
			legacyDatabaseMigrationDoc,
		),
	}
}
