package runner

import (
	"path/filepath"
	"strings"
	"testing"
)

// Per-package test consts. Several conformance-runner tests assert on
// the same BigQuery TypeKind spellings or the same synthetic-name
// fixtures; promoting them to consts stops goconst from flagging the
// repeats and pins the spelling to a single source of truth.
const (
	testTypeINT64   = bqTypeINT64
	testTypeFLOAT64 = bqTypeFLOAT64
	testTypeSTRING  = bqTypeSTRING
	testNameAda     = "ada"
	testNameLinus   = "linus"
	testColumnName  = "name"
	// testBogus is the shared placeholder for "definitely not a
	// known {profile, route, ...} entry" across the runner test
	// suite. Promoted to a const so goconst's
	// repeated-string-literal gate stays clean as new validation
	// tests land.
	testBogus = "bogus"
)

// TestLoadValidFixture pins the happy-path load for every seed
// fixture: every file under `conformance/fixtures/` must parse, have
// at least one declared profile, and pass the schema validation
// helpers without needing a running engine.
func TestLoadValidFixture(t *testing.T) {
	matches, err := filepath.Glob("../fixtures/*.yaml")
	if err != nil {
		t.Fatalf("glob fixtures: %v", err)
	}
	if len(matches) == 0 {
		t.Fatal("no fixtures under ../fixtures")
	}
	for _, p := range matches {
		t.Run(filepath.Base(p), func(t *testing.T) {
			f, err := Load(p)
			if err != nil {
				t.Fatalf("Load(%s): %v", p, err)
			}
			if f.Name == "" {
				t.Error("fixture has empty name")
			}
			if len(f.Profiles) == 0 {
				t.Error("fixture has no resolved profiles")
			}
			if f.Query == "" {
				t.Error("fixture has empty query")
			}
		})
	}
}

func TestLoadDefaultsToActiveProfileSet(t *testing.T) {
	body := []byte(`name: test
query: SELECT 1
expected:
  rows:
    - {n: "1"}
`)
	f, err := loadBytes(body, "test.yaml")
	if err != nil {
		t.Fatalf("loadBytes: %v", err)
	}
	if len(f.Profiles) != len(defaultProfiles) {
		t.Fatalf("profiles=%v, want %d default entries", f.Profiles, len(defaultProfiles))
	}
}

func TestLoadDefaultsMatchToOrdered(t *testing.T) {
	body := []byte(`name: test
query: SELECT 1
expected:
  rows:
    - {n: "1"}
`)
	f, err := loadBytes(body, "test.yaml")
	if err != nil {
		t.Fatalf("loadBytes: %v", err)
	}
	if f.Expected.Match != MatchOrdered {
		t.Fatalf("Match=%q, want %q", f.Expected.Match, MatchOrdered)
	}
}

func TestLoadAcceptsUnorderedMatch(t *testing.T) {
	body := []byte(`name: test
query: SELECT 1
expected:
  match: unordered
  rows:
    - {n: "1"}
`)
	f, err := loadBytes(body, "test.yaml")
	if err != nil {
		t.Fatalf("loadBytes: %v", err)
	}
	if f.Expected.Match != MatchUnordered {
		t.Fatalf("Match=%q, want unordered", f.Expected.Match)
	}
}

func TestLoadAcceptsSchemaOnlyMatch(t *testing.T) {
	body := []byte(`name: test
query: SELECT 1
expected:
  match: schema_only
  schema:
    - {name: n, type: INT64}
`)
	f, err := loadBytes(body, "test.yaml")
	if err != nil {
		t.Fatalf("loadBytes: %v", err)
	}
	if f.Expected.Match != MatchSchemaOnly {
		t.Fatalf("Match=%q, want schema_only", f.Expected.Match)
	}
	if len(f.Expected.Schema) != 1 || f.Expected.Schema[0].Name != "n" {
		t.Fatalf("Schema=%v, want [{n, INT64}]", f.Expected.Schema)
	}
}

func TestLoadRejectsUnknownMatch(t *testing.T) {
	body := []byte(`name: test
query: SELECT 1
expected:
  match: cosmic
  rows: []
`)
	_, err := loadBytes(body, "test.yaml")
	if err == nil || !strings.Contains(err.Error(), "match=") {
		t.Fatalf("want unknown-match error, got %v", err)
	}
}

func TestLoadRejectsSchemaOnlyWithoutSchemaOrRows(t *testing.T) {
	body := []byte(`name: test
query: SELECT 1
expected:
  match: schema_only
`)
	_, err := loadBytes(body, "test.yaml")
	if err == nil || !strings.Contains(err.Error(), "schema_only") {
		t.Fatalf("want schema_only requires schema/rows error, got %v", err)
	}
}

func TestLoadRejectsUnknownProfile(t *testing.T) {
	body := []byte(`name: test
profiles: [bogus]
query: SELECT 1
expected:
  rows: []
`)
	_, err := loadBytes(body, "test.yaml")
	if err == nil || !strings.Contains(err.Error(), "unknown profile") {
		t.Fatalf("want unknown-profile error, got %v", err)
	}
}

func TestLoadRejectsMissingExpectation(t *testing.T) {
	body := []byte(`name: test
query: SELECT 1
`)
	_, err := loadBytes(body, "test.yaml")
	if err == nil || !strings.Contains(err.Error(), "expected:") {
		t.Fatalf("want missing-expectation error, got %v", err)
	}
}

func TestLoadRejectsBothExpectations(t *testing.T) {
	body := []byte(`name: test
query: SELECT 1
expected:
  rows: []
  error:
    code: 400
`)
	_, err := loadBytes(body, "test.yaml")
	if err == nil || !strings.Contains(err.Error(), "cannot be combined") {
		t.Fatalf("want exclusive-expectation error, got %v", err)
	}
}

func TestLoadRejectsEmptyError(t *testing.T) {
	body := []byte(`name: test
query: SELECT 1
expected:
  error: {}
`)
	_, err := loadBytes(body, "test.yaml")
	if err == nil || !strings.Contains(err.Error(), "expected.error") {
		t.Fatalf("want empty-error error, got %v", err)
	}
}

func TestLoadRejectsSetupStepWithMultipleKinds(t *testing.T) {
	body := []byte(`name: test
query: SELECT 1
setup:
  - dataset: ds_foo
    sql: SELECT 1
expected:
  rows: []
`)
	_, err := loadBytes(body, "test.yaml")
	if err == nil || !strings.Contains(err.Error(), "exactly one") {
		t.Fatalf("want one-kind-per-step error, got %v", err)
	}
}

// TestSetupStepValidate covers the validate() branch matrix for
// SetupStep so the loader's error messages stay readable.
func TestSetupStepValidate(t *testing.T) {
	cases := []struct {
		name    string
		step    SetupStep
		wantErr string
	}{
		{"dataset", SetupStep{Dataset: "ds"}, ""},
		{"sql", SetupStep{SQL: "SELECT 1"}, ""},
		{"table-missing-id", SetupStep{Table: &TableSetup{Dataset: "ds"}}, "table.id"},
		{"table-missing-dataset", SetupStep{Table: &TableSetup{ID: "t"}}, "table.dataset"},
		{"table-no-schema", SetupStep{Table: &TableSetup{Dataset: "ds", ID: "t"}}, "table.schema"},
		{"empty", SetupStep{}, "exactly one"},
		{"two-kinds", SetupStep{Dataset: "ds", SQL: "SELECT 1"}, "exactly one"},
	}
	for _, c := range cases {
		t.Run(c.name, func(t *testing.T) {
			err := c.step.validate()
			if c.wantErr == "" {
				if err != nil {
					t.Fatalf("want nil err, got %v", err)
				}
				return
			}
			if err == nil || !strings.Contains(err.Error(), c.wantErr) {
				t.Fatalf("want err containing %q, got %v", c.wantErr, err)
			}
		})
	}
}
