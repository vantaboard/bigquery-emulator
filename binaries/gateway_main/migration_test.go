package main

import (
	"bytes"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestApplyLegacyDatabaseFlag_MapsToParentDir(t *testing.T) {
	cfg := Config{LegacyDatabase: "/opt/x.db"}
	if err := applyLegacyDatabaseFlag(&cfg); err != nil {
		t.Fatalf("applyLegacyDatabaseFlag: %v", err)
	}
	if cfg.DataDir != "/opt" {
		t.Errorf("DataDir=%q, want /opt", cfg.DataDir)
	}
	if len(cfg.StartupWarnings) != 1 {
		t.Fatalf("StartupWarnings=%v, want one deprecation notice", cfg.StartupWarnings)
	}
	if !strings.Contains(cfg.StartupWarnings[0], "DEPRECATED") {
		t.Errorf("warning=%q", cfg.StartupWarnings[0])
	}
}

func TestApplyLegacyDatabaseFlag_RejectsBothFlags(t *testing.T) {
	cfg := Config{LegacyDatabase: "/opt/x.db", DataDir: "/var/lib/bq"}
	if err := applyLegacyDatabaseFlag(&cfg); err == nil {
		t.Fatal("expected error when --database and --data-dir are both set")
	}
}

func TestCollectDataDirLayoutWarnings_LegacySQLiteWithoutCatalog(t *testing.T) {
	dir := t.TempDir()
	if err := os.WriteFile(filepath.Join(dir, "x.db"), []byte("sqlite"), 0o600); err != nil {
		t.Fatal(err)
	}
	warn := collectDataDirLayoutWarnings(dir)
	if len(warn) != 1 {
		t.Fatalf("warnings=%v, want one", warn)
	}
	if !strings.Contains(warn[0], "no catalog.duckdb") {
		t.Errorf("warning=%q", warn[0])
	}
}

func TestCollectDataDirLayoutWarnings_FileDataDirFails(t *testing.T) {
	file := filepath.Join(t.TempDir(), "catalog.db")
	if err := os.WriteFile(file, []byte("sqlite"), 0o600); err != nil {
		t.Fatal(err)
	}
	warn := collectDataDirLayoutWarnings(file)
	if len(warn) != 1 || !strings.HasPrefix(warn[0], "ERROR:") {
		t.Fatalf("warnings=%v, want ERROR prefix", warn)
	}
}

func TestParseArgs_LegacyDatabaseFlag(t *testing.T) {
	var errBuf bytes.Buffer
	cfg, err := parseArgs([]string{"--database=/opt/x.db"}, &errBuf, emptyEnv)
	if err != nil {
		t.Fatalf("parseArgs: %v\nstderr=%s", err, errBuf.String())
	}
	if cfg.DataDir != "/opt" {
		t.Errorf("DataDir=%q, want /opt", cfg.DataDir)
	}
	if len(cfg.StartupWarnings) < 1 {
		t.Fatalf("StartupWarnings=%v, want deprecation notice", cfg.StartupWarnings)
	}
}

func TestParseArgs_DataDirWithLegacyLayoutWarns(t *testing.T) {
	dir := t.TempDir()
	if err := os.WriteFile(filepath.Join(dir, "legacy.db"), []byte("x"), 0o600); err != nil {
		t.Fatal(err)
	}
	cfg, err := parseArgs([]string{"--data-dir=" + dir}, &bytes.Buffer{}, emptyEnv)
	if err != nil {
		t.Fatalf("parseArgs: %v", err)
	}
	found := false
	for _, w := range cfg.StartupWarnings {
		if strings.Contains(w, "legacy.db") {
			found = true
			break
		}
	}
	if !found {
		t.Fatalf("StartupWarnings=%v, want legacy.db mention", cfg.StartupWarnings)
	}
}

func TestLogStartupWarnings_ErrorPrefixFails(t *testing.T) {
	err := logStartupWarnings([]string{"ERROR: --data-dir is a file"})
	if err == nil {
		t.Fatal("expected error")
	}
}
