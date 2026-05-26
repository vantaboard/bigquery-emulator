package runner

import (
	"path/filepath"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
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
		p := p
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

func TestLoadDefaultsToBothProfiles(t *testing.T) {
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
	if len(f.Profiles) != 2 {
		t.Fatalf("profiles=%v, want 2 default entries", f.Profiles)
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
	if err == nil || !strings.Contains(err.Error(), "only one") {
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

// TestRowDiffMatchesEqualRows pins the happy-path: when the gateway
// returns rows that match the YAML, rowDiff is an empty string.
func TestRowDiffMatchesEqualRows(t *testing.T) {
	expected := []map[string]any{
		{"id": "1", "name": "ada"},
		{"id": "2", "name": "linus"},
	}
	schema := &bqtypes.TableSchema{
		Fields: []bqtypes.TableFieldSchema{
			{Name: "id", Type: "INT64"},
			{Name: "name", Type: "STRING"},
		},
	}
	actual := []bqtypes.Row{
		{F: []bqtypes.Cell{{V: "1"}, {V: "ada"}}},
		{F: []bqtypes.Cell{{V: "2"}, {V: "linus"}}},
	}
	if diff := rowDiff(expected, schema, actual); diff != "" {
		t.Fatalf("rowDiff: want empty (match), got:\n%s", diff)
	}
}

// TestRowDiffDetectsCellValueDifference pins the negative path: when
// one cell value differs, rowDiff returns a unified diff that
// mentions both the expected and actual values so a human can pin
// which column drifted.
func TestRowDiffDetectsCellValueDifference(t *testing.T) {
	expected := []map[string]any{
		{"id": "1", "name": "ada"},
	}
	schema := &bqtypes.TableSchema{
		Fields: []bqtypes.TableFieldSchema{
			{Name: "id", Type: "INT64"},
			{Name: "name", Type: "STRING"},
		},
	}
	actual := []bqtypes.Row{
		{F: []bqtypes.Cell{{V: "1"}, {V: "augusta"}}},
	}
	diff := rowDiff(expected, schema, actual)
	if diff == "" {
		t.Fatal("rowDiff: want non-empty diff, got empty")
	}
	if !strings.Contains(diff, "ada") || !strings.Contains(diff, "augusta") {
		t.Fatalf("diff missing one of the cell values:\n%s", diff)
	}
}

func TestRowDiffDetectsRowCountMismatch(t *testing.T) {
	expected := []map[string]any{
		{"id": "1"},
		{"id": "2"},
	}
	schema := &bqtypes.TableSchema{
		Fields: []bqtypes.TableFieldSchema{{Name: "id", Type: "INT64"}},
	}
	actual := []bqtypes.Row{
		{F: []bqtypes.Cell{{V: "1"}}},
	}
	diff := rowDiff(expected, schema, actual)
	if diff == "" {
		t.Fatal("rowDiff: want non-empty diff for row-count mismatch")
	}
	if !strings.Contains(diff, "expected:") || !strings.Contains(diff, "actual:") {
		t.Fatalf("diff missing expected/actual sections:\n%s", diff)
	}
}

func TestRowDiffCoercesIntegerYAMLValues(t *testing.T) {
	// YAML decoders give us `int` for unquoted numerics; cellString
	// returns the string form. The diff should still pass because
	// both sides canonicalize to "1".
	expected := []map[string]any{
		{"id": 1},
	}
	schema := &bqtypes.TableSchema{
		Fields: []bqtypes.TableFieldSchema{{Name: "id", Type: "INT64"}},
	}
	actual := []bqtypes.Row{
		{F: []bqtypes.Cell{{V: "1"}}},
	}
	if diff := rowDiff(expected, schema, actual); diff != "" {
		t.Fatalf("rowDiff: want empty for coerced int, got:\n%s", diff)
	}
}

func TestErrorDiffMatchesCodeAndMessage(t *testing.T) {
	body := []byte(`{"error":{"code":400,"message":"Syntax error: missing FROM near bad","status":"invalidQuery","errors":[{"reason":"invalidQuery","message":"Syntax error: missing FROM near bad"}]}}`)
	expected := ExpectedError{Code: 400, MessageContains: "Syntax error"}
	if msg := errorDiff(expected, 400, body); msg != "" {
		t.Fatalf("errorDiff: want empty, got %q", msg)
	}
}

func TestErrorDiffDetectsCodeMismatch(t *testing.T) {
	body := []byte(`{"error":{"code":500,"message":"Internal error"}}`)
	expected := ExpectedError{Code: 400, MessageContains: "Syntax"}
	msg := errorDiff(expected, 500, body)
	if !strings.Contains(msg, "expected 400") {
		t.Fatalf("errorDiff: want 'expected 400' in message, got %q", msg)
	}
}

func TestErrorDiffDetectsMessageMismatch(t *testing.T) {
	body := []byte(`{"error":{"code":400,"message":"Some other thing happened"}}`)
	expected := ExpectedError{Code: 400, MessageContains: "Unrecognized"}
	msg := errorDiff(expected, 400, body)
	if !strings.Contains(msg, "Unrecognized") {
		t.Fatalf("errorDiff: want Unrecognized in mismatch message, got %q", msg)
	}
}

func TestErrorDiffFallsBackToInnerErrorMessage(t *testing.T) {
	// Some gateway paths leave the top-level message empty but
	// populate errors[0].message; the diff should treat that as
	// the source of truth.
	body := []byte(`{"error":{"code":400,"errors":[{"reason":"invalidQuery","message":"Unrecognized name: bad"}]}}`)
	expected := ExpectedError{Code: 400, MessageContains: "Unrecognized"}
	if msg := errorDiff(expected, 400, body); msg != "" {
		t.Fatalf("errorDiff: want empty via fallback, got %q", msg)
	}
}

func TestResolveProfilesDefaultsToAll(t *testing.T) {
	got, err := resolveProfiles(nil)
	if err != nil {
		t.Fatalf("resolveProfiles(nil): %v", err)
	}
	if len(got) != len(KnownProfiles()) {
		t.Errorf("len=%d, want %d", len(got), len(KnownProfiles()))
	}
}

func TestResolveProfilesFiltersToSubset(t *testing.T) {
	got, err := resolveProfiles([]string{"memory"})
	if err != nil {
		t.Fatalf("resolveProfiles: %v", err)
	}
	if len(got) != 1 || got[0].Name != "memory" {
		t.Fatalf("got=%v, want [memory]", got)
	}
}

func TestResolveProfilesRejectsUnknown(t *testing.T) {
	_, err := resolveProfiles([]string{"bogus"})
	if err == nil || !strings.Contains(err.Error(), "unknown") {
		t.Fatalf("want unknown-profile error, got %v", err)
	}
}

// TestRunReportExitCode exercises the exit-code derivation so the
// CLI's exit-mapping is unit-testable without a real subprocess.
func TestRunReportExitCode(t *testing.T) {
	cases := []struct {
		name string
		r    *Report
		want int
	}{
		{"nil", nil, 2},
		{"all pass", &Report{Summary: Summary{Total: 3, Passed: 3}}, 0},
		{"one fail", &Report{Summary: Summary{Total: 3, Passed: 2, Failed: 1}}, 1},
	}
	for _, c := range cases {
		t.Run(c.name, func(t *testing.T) {
			if got := c.r.ExitCode(); got != c.want {
				t.Errorf("ExitCode=%d, want %d", got, c.want)
			}
		})
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
