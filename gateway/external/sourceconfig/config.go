// Package sourceconfig resolves external data sources to fixture, local, or
// live modes. Defaults favor deterministic offline behavior; live upstream
// fetches are strictly opt-in per source kind or identity.
package sourceconfig

import (
	"errors"
	"os"
	"path/filepath"
	"strings"

	"gopkg.in/yaml.v3"
)

// Mode is how an external source identity is resolved at fetch time.
type Mode string

const (
	// ModeFixture reads committed snapshots under DataDir/external/fixtures/.
	ModeFixture Mode = "fixture"
	// ModeLocal uses fake-gcs (STORAGE_EMULATOR_HOST) or local file paths.
	ModeLocal Mode = "local"
	// ModeLive reaches a real upstream (credentials / network gated).
	ModeLive Mode = "live"
)

// Kind classifies an external source for default-mode lookup.
type Kind string

const (
	KindGCS          Kind = "gcs"
	KindGoogleSheets Kind = "google_sheets"
	KindConnection   Kind = "connection"
)

// Config holds per-source resolution rules loaded from DataDir and env vars.
type Config struct {
	DataDir string
	// defaults by kind; overridden per source id in Sources.
	defaults map[Kind]Mode
	sources  map[string]Mode
}

type fileShape struct {
	Defaults map[string]string      `yaml:"defaults"`
	Sources  map[string]sourceEntry `yaml:"sources"`
}

type sourceEntry struct {
	Kind string `yaml:"kind"`
	Mode string `yaml:"mode"`
}

// Load builds a Config from dataDir and optional external_sources.yaml.
// When dataDir is empty, only env-based overrides apply.
func Load(dataDir string) (*Config, error) {
	c := &Config{
		DataDir: dataDir,
		defaults: map[Kind]Mode{
			KindGCS:          ModeLocal,
			KindGoogleSheets: ModeFixture,
			KindConnection:   ModeFixture,
		},
		sources: map[string]Mode{},
	}
	if dataDir != "" {
		if err := c.loadYAMLFile(dataDir); err != nil {
			return nil, err
		}
	}
	c.applyEnvOverrides()
	return c, nil
}

func (c *Config) loadYAMLFile(dataDir string) error {
	path := filepath.Join(dataDir, "external_sources.yaml")
	raw, err := os.ReadFile(path) //nolint:gosec // operator-controlled data dir
	if err != nil {
		if errors.Is(err, os.ErrNotExist) {
			return nil
		}
		return err
	}
	var f fileShape
	if err := yaml.Unmarshal(raw, &f); err != nil {
		return err
	}
	for k, v := range f.Defaults {
		if m := parseMode(v); m != "" {
			c.defaults[parseKind(k)] = m
		}
	}
	for id, ent := range f.Sources {
		if m := parseMode(ent.Mode); m != "" {
			c.sources[normalizeID(id)] = m
		}
	}
	return nil
}

func (c *Config) applyEnvOverrides() {
	if truthy(os.Getenv("BIGQUERY_EMULATOR_LIVE_SHEETS")) {
		c.defaults[KindGoogleSheets] = ModeLive
	}
	if v := strings.TrimSpace(os.Getenv("BIGQUERY_EMULATOR_EXTERNAL_GCS_MODE")); v != "" {
		if m := parseMode(v); m != "" {
			c.defaults[KindGCS] = m
		}
	}
	if v := strings.TrimSpace(os.Getenv("BIGQUERY_EMULATOR_EXTERNAL_CONNECTIONS_MODE")); v != "" {
		if m := parseMode(v); m != "" {
			c.defaults[KindConnection] = m
		}
	}
}

func truthy(s string) bool {
	switch strings.ToLower(strings.TrimSpace(s)) {
	case "1", "true", "yes", "on":
		return true
	default:
		return false
	}
}

func parseMode(s string) Mode {
	switch strings.ToLower(strings.TrimSpace(s)) {
	case string(ModeFixture):
		return ModeFixture
	case string(ModeLocal):
		return ModeLocal
	case string(ModeLive):
		return ModeLive
	default:
		return ""
	}
}

func parseKind(s string) Kind {
	switch strings.ToLower(strings.TrimSpace(s)) {
	case "gcs":
		return KindGCS
	case "google_sheets", "googlesheets", "sheets":
		return KindGoogleSheets
	case "connection", "connections":
		return KindConnection
	default:
		return Kind(s)
	}
}

func normalizeID(id string) string {
	return strings.ToLower(strings.TrimSpace(id))
}

// ResolveGCS returns the mode for a gs:// URI.
func (c *Config) ResolveGCS(uri string) Mode {
	if c == nil {
		return ModeLocal
	}
	if m, ok := c.sources[normalizeID(uri)]; ok {
		return m
	}
	bucket := gcsBucket(uri)
	if bucket != "" {
		if m, ok := c.sources[normalizeID(bucket)]; ok {
			return m
		}
	}
	return c.defaults[KindGCS]
}

// ResolveGoogleSheets returns the mode for a Sheets doc id or URL.
func (c *Config) ResolveGoogleSheets(docOrURL string) Mode {
	if c == nil {
		return ModeFixture
	}
	id := ExtractSheetDocID(docOrURL)
	if id == "" {
		id = docOrURL
	}
	if m, ok := c.sources[normalizeID(id)]; ok {
		return m
	}
	return c.defaults[KindGoogleSheets]
}

// ResolveConnection returns the mode for a connection resource name or id.
func (c *Config) ResolveConnection(name string) Mode {
	if c == nil {
		return ModeFixture
	}
	id := connectionID(name)
	if m, ok := c.sources[normalizeID(id)]; ok {
		return m
	}
	if m, ok := c.sources[normalizeID(name)]; ok {
		return m
	}
	return c.defaults[KindConnection]
}

// FixtureRoot returns the directory for committed external snapshots.
func (c *Config) FixtureRoot() string {
	if c == nil || c.DataDir == "" {
		return ""
	}
	return filepath.Join(c.DataDir, "external", "fixtures")
}

// GCSCacheRoot returns the directory where gs:// objects are materialized
// for engine LOAD/EXPORT and offline snapshots.
func (c *Config) GCSCacheRoot() string {
	if c == nil || c.DataDir == "" {
		return ""
	}
	return filepath.Join(c.DataDir, "external", "gcs-cache")
}

// ConnectionFixtureRoot returns fixture SQL result files for EXTERNAL_QUERY.
func (c *Config) ConnectionFixtureRoot() string {
	if c == nil || c.DataDir == "" {
		return ""
	}
	return filepath.Join(c.DataDir, "external", "connections")
}

func gcsBucket(uri string) string {
	rest := strings.TrimPrefix(uri, "gs://")
	if i := strings.Index(rest, "/"); i > 0 {
		return rest[:i]
	}
	return ""
}

func connectionID(name string) string {
	name = strings.TrimSpace(name)
	if i := strings.LastIndex(name, "/"); i >= 0 {
		return name[i+1:]
	}
	// EXTERNAL_QUERY connection arg may be region.id
	if i := strings.LastIndex(name, "."); i >= 0 {
		return name[i+1:]
	}
	return name
}

// ExtractSheetDocID parses a Google Sheets URL or bare doc id.
func ExtractSheetDocID(uri string) string {
	uri = strings.TrimSpace(uri)
	if uri == "" {
		return ""
	}
	const marker = "/d/"
	if _, after, ok := strings.Cut(uri, marker); ok {
		rest := after
		if j := strings.IndexAny(rest, "/#?"); j >= 0 {
			return rest[:j]
		}
		return rest
	}
	if !strings.Contains(uri, "/") && !strings.Contains(uri, "://") {
		return uri
	}
	return ""
}
