package runner

import (
	"math"
	"path/filepath"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
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

// schemaWith is a tiny test helper that builds a TableSchema from
// (name,type) pairs so the cell-comparison tests stay readable.
func schemaWith(pairs ...string) *bqtypes.TableSchema {
	if len(pairs)%2 != 0 {
		panic("schemaWith: need (name,type) pairs")
	}
	s := &bqtypes.TableSchema{}
	for i := 0; i < len(pairs); i += 2 {
		s.Fields = append(s.Fields, bqtypes.TableFieldSchema{
			Name: pairs[i],
			Type: pairs[i+1],
		})
	}
	return s
}

// rowOf builds a wire-format bqtypes.Row from raw cell values.
func rowOf(values ...any) bqtypes.Row {
	row := bqtypes.Row{F: make([]bqtypes.Cell, len(values))}
	for i, v := range values {
		row.F[i] = bqtypes.Cell{V: v}
	}
	return row
}

// TestRowDiffOrderedMatchesEqualRows pins the happy-path: when the
// gateway returns rows that match the YAML, rowDiff is empty.
func TestRowDiffOrderedMatchesEqualRows(t *testing.T) {
	exp := Expectation{
		Rows: []map[string]any{
			{"id": "1", testColumnName: testNameAda},
			{"id": "2", testColumnName: testNameLinus},
		},
	}
	schema := schemaWith("id", testTypeINT64, testColumnName, testTypeSTRING)
	actual := []bqtypes.Row{rowOf("1", testNameAda), rowOf("2", testNameLinus)}
	if diff := rowDiff(exp, schema, actual); diff != "" {
		t.Fatalf("rowDiff: want empty (match), got:\n%s", diff)
	}
}

func TestRowDiffOrderedDetectsCellValueDifference(t *testing.T) {
	exp := Expectation{
		Rows: []map[string]any{{"id": "1", testColumnName: testNameAda}},
	}
	schema := schemaWith("id", testTypeINT64, testColumnName, testTypeSTRING)
	actual := []bqtypes.Row{rowOf("1", "augusta")}
	diff := rowDiff(exp, schema, actual)
	if diff == "" {
		t.Fatal("rowDiff: want non-empty diff, got empty")
	}
	if !strings.Contains(diff, testNameAda) || !strings.Contains(diff, "augusta") {
		t.Fatalf("diff missing one of the cell values:\n%s", diff)
	}
}

func TestRowDiffOrderedDetectsRowCountMismatch(t *testing.T) {
	exp := Expectation{
		Rows: []map[string]any{{"id": "1"}, {"id": "2"}},
	}
	schema := schemaWith("id", testTypeINT64)
	actual := []bqtypes.Row{rowOf("1")}
	diff := rowDiff(exp, schema, actual)
	if diff == "" {
		t.Fatal("rowDiff: want non-empty diff for row-count mismatch")
	}
	if !strings.Contains(diff, "expected:") || !strings.Contains(diff, "actual:") {
		t.Fatalf("diff missing expected/actual sections:\n%s", diff)
	}
}

// TestRowDiffIntCoercesYAMLNumeric pins typed INT64 comparison: a
// YAML `1` (decoded as int) matches the gateway's wire string "1".
func TestRowDiffIntCoercesYAMLNumeric(t *testing.T) {
	exp := Expectation{
		Rows: []map[string]any{{"id": 1}},
	}
	schema := schemaWith("id", testTypeINT64)
	actual := []bqtypes.Row{rowOf("1")}
	if diff := rowDiff(exp, schema, actual); diff != "" {
		t.Fatalf("rowDiff: want empty for typed INT coercion, got:\n%s", diff)
	}
}

// TestRowDiffNumericCoercesDecimal pins typed NUMERIC comparison
// (the rational form covers fixed-point precision the float path
// would lose).
func TestRowDiffNumericCoercesDecimal(t *testing.T) {
	exp := Expectation{
		Rows: []map[string]any{{"amount": "1.2300"}},
	}
	schema := schemaWith("amount", "NUMERIC")
	actual := []bqtypes.Row{rowOf("1.23")}
	if diff := rowDiff(exp, schema, actual); diff != "" {
		t.Fatalf("rowDiff: want empty for NUMERIC equivalence, got:\n%s", diff)
	}
}

func TestRowDiffFloatWithinEpsilonPasses(t *testing.T) {
	// 1.0 vs 1.0 + 1e-12 is well inside the 1e-9 relative
	// tolerance; both sides parse as float64 and compare equal.
	exp := Expectation{
		Rows: []map[string]any{{"v": "1.0"}},
	}
	schema := schemaWith("v", testTypeFLOAT64)
	actual := []bqtypes.Row{rowOf("1.000000000001")}
	if diff := rowDiff(exp, schema, actual); diff != "" {
		t.Fatalf("rowDiff: want empty for float within epsilon, got:\n%s", diff)
	}
}

func TestRowDiffFloatOutsideEpsilonFails(t *testing.T) {
	// 1.0 vs 1.001 is well outside 1e-9 relative tolerance.
	exp := Expectation{
		Rows: []map[string]any{{"v": "1.0"}},
	}
	schema := schemaWith("v", testTypeFLOAT64)
	actual := []bqtypes.Row{rowOf("1.001")}
	if diff := rowDiff(exp, schema, actual); diff == "" {
		t.Fatal("rowDiff: want non-empty diff for float outside epsilon")
	}
}

func TestRowDiffBoolNormalizes(t *testing.T) {
	// YAML decodes unquoted `true` as bool; wire returns "true".
	exp := Expectation{
		Rows: []map[string]any{{"flag": true}},
	}
	schema := schemaWith("flag", "BOOL")
	actual := []bqtypes.Row{rowOf("true")}
	if diff := rowDiff(exp, schema, actual); diff != "" {
		t.Fatalf("rowDiff: want empty for bool normalize, got:\n%s", diff)
	}
}

func TestRowDiffTimestampParsesRFC3339(t *testing.T) {
	exp := Expectation{
		Rows: []map[string]any{{"ts": "2024-01-02T03:04:05Z"}},
	}
	schema := schemaWith("ts", "TIMESTAMP")
	// BigQuery's wire form is Unix-seconds-as-string with optional
	// fractional component; this is the same instant.
	actual := []bqtypes.Row{rowOf("1704164645.000000")}
	if diff := rowDiff(exp, schema, actual); diff != "" {
		t.Fatalf("rowDiff: want empty for matching timestamps, got:\n%s", diff)
	}
}

func TestRowDiffNullMatchesNull(t *testing.T) {
	exp := Expectation{
		Rows: []map[string]any{{testColumnName: nil}},
	}
	schema := schemaWith(testColumnName, testTypeSTRING)
	actual := []bqtypes.Row{rowOf(nil)}
	if diff := rowDiff(exp, schema, actual); diff != "" {
		t.Fatalf("rowDiff: want empty for NULL == NULL, got:\n%s", diff)
	}
}

func TestRowDiffNullVsValueMismatch(t *testing.T) {
	exp := Expectation{
		Rows: []map[string]any{{testColumnName: nil}},
	}
	schema := schemaWith(testColumnName, testTypeSTRING)
	actual := []bqtypes.Row{rowOf(testNameAda)}
	if diff := rowDiff(exp, schema, actual); diff == "" {
		t.Fatal("rowDiff: want non-empty diff for NULL vs value")
	}
}

func TestRowDiffNullVsLiteralStringMismatch(t *testing.T) {
	// A real NULL on the wire must not match the literal string
	// "NULL" on the expected side; the diff would otherwise mask
	// a fixture writer's mistake.
	exp := Expectation{
		Rows: []map[string]any{{testColumnName: "NULL"}},
	}
	schema := schemaWith(testColumnName, testTypeSTRING)
	actual := []bqtypes.Row{rowOf(nil)}
	if diff := rowDiff(exp, schema, actual); diff == "" {
		t.Fatal(`rowDiff: want non-empty diff for "NULL" string vs NULL cell`)
	}
}

// TestRowDiffUnorderedMatchesShuffled is the canonical multiset case:
// expected and actual contain the same rows in different orders.
func TestRowDiffUnorderedMatchesShuffled(t *testing.T) {
	exp := Expectation{
		Match: MatchUnordered,
		Rows: []map[string]any{
			{"id": "1", testColumnName: testNameAda},
			{"id": "2", testColumnName: testNameLinus},
		},
	}
	schema := schemaWith("id", testTypeINT64, testColumnName, testTypeSTRING)
	actual := []bqtypes.Row{rowOf("2", testNameLinus), rowOf("1", testNameAda)}
	if diff := rowDiff(exp, schema, actual); diff != "" {
		t.Fatalf("rowDiff: want empty for shuffled-equal multiset, got:\n%s", diff)
	}
}

func TestRowDiffUnorderedDetectsSwap(t *testing.T) {
	exp := Expectation{
		Match: MatchUnordered,
		Rows: []map[string]any{
			{"id": "1", testColumnName: testNameAda},
			{"id": "2", testColumnName: testNameLinus},
		},
	}
	schema := schemaWith("id", testTypeINT64, testColumnName, testTypeSTRING)
	// id=2 row's name swapped for "grace"; this should surface
	// as one missing (linus) and one extra (grace).
	actual := []bqtypes.Row{rowOf("1", testNameAda), rowOf("2", "grace")}
	diff := rowDiff(exp, schema, actual)
	if diff == "" {
		t.Fatal("rowDiff: want non-empty diff for swapped row")
	}
	if !strings.Contains(diff, "missing") || !strings.Contains(diff, "extra") {
		t.Fatalf("unordered diff missing missing/extra sections:\n%s", diff)
	}
	if !strings.Contains(diff, testNameLinus) {
		t.Fatalf("expected 'linus' in missing section:\n%s", diff)
	}
	if !strings.Contains(diff, "grace") {
		t.Fatalf("expected 'grace' in extra section:\n%s", diff)
	}
}

func TestRowDiffUnorderedRowCountMismatch(t *testing.T) {
	exp := Expectation{
		Match: MatchUnordered,
		Rows: []map[string]any{
			{"id": "1"},
			{"id": "2"},
		},
	}
	schema := schemaWith("id", testTypeINT64)
	actual := []bqtypes.Row{rowOf("1")}
	diff := rowDiff(exp, schema, actual)
	if diff == "" {
		t.Fatal("rowDiff: want non-empty diff for unordered row-count mismatch")
	}
	if !strings.Contains(diff, "missing") {
		t.Fatalf("unordered diff missing 'missing' section:\n%s", diff)
	}
}

// TestRowDiffSchemaOnlyMatchesDifferentRows pins the dryRun-style
// case: rows differ wildly but the column set is the same, so
// schema_only mode passes.
func TestRowDiffSchemaOnlyMatchesDifferentRows(t *testing.T) {
	exp := Expectation{
		Match: MatchSchemaOnly,
		Schema: []ExpectedColumn{
			{Name: "id", Type: testTypeINT64},
			{Name: testColumnName, Type: testTypeSTRING},
		},
	}
	schema := schemaWith("id", testTypeINT64, testColumnName, testTypeSTRING)
	actual := []bqtypes.Row{
		rowOf("7", "henrietta"),
		rowOf("8", "grace"),
	}
	if diff := rowDiff(exp, schema, actual); diff != "" {
		t.Fatalf("rowDiff schema_only: want empty, got:\n%s", diff)
	}
}

func TestRowDiffSchemaOnlyDetectsColumnNameMismatch(t *testing.T) {
	exp := Expectation{
		Match: MatchSchemaOnly,
		Schema: []ExpectedColumn{
			{Name: "id", Type: testTypeINT64},
			{Name: testColumnName, Type: testTypeSTRING},
		},
	}
	// Actual has "id" + "label" (not testColumnName).
	schema := schemaWith("id", testTypeINT64, "label", testTypeSTRING)
	actual := []bqtypes.Row{rowOf("1", testNameAda)}
	diff := rowDiff(exp, schema, actual)
	if diff == "" {
		t.Fatal("rowDiff schema_only: want non-empty diff for column-name mismatch")
	}
	if !strings.Contains(diff, testColumnName) || !strings.Contains(diff, "label") {
		t.Fatalf("schema_only diff missing column names:\n%s", diff)
	}
}

func TestRowDiffSchemaOnlyDetectsTypeMismatch(t *testing.T) {
	exp := Expectation{
		Match: MatchSchemaOnly,
		Schema: []ExpectedColumn{
			{Name: "amount", Type: testTypeFLOAT64},
		},
	}
	schema := schemaWith("amount", testTypeINT64)
	if diff := rowDiff(exp, schema, nil); diff == "" {
		t.Fatal("rowDiff schema_only: want non-empty diff for type mismatch")
	}
}

func TestRowDiffSchemaOnlyFallsBackToRowKeys(t *testing.T) {
	// No schema: declared, only rows[0] keys. The runner should
	// still pin the column-name set from those keys.
	exp := Expectation{
		Match: MatchSchemaOnly,
		Rows:  []map[string]any{{"id": 0, testColumnName: ""}},
	}
	schema := schemaWith("id", testTypeINT64, testColumnName, testTypeSTRING)
	actual := []bqtypes.Row{rowOf("42", "anything")}
	if diff := rowDiff(exp, schema, actual); diff != "" {
		t.Fatalf("rowDiff schema_only (rows fallback): want empty, got:\n%s", diff)
	}
}

// TestExpectedSchemaPreflightCatchesColumnDrift exercises the
// advisory `expected.schema:` preflight used by ordered/unordered
// modes. It does NOT replace the row diff; it only catches schema
// drift before the row diff runs.
func TestExpectedSchemaPreflightCatchesColumnDrift(t *testing.T) {
	exp := Expectation{
		Match: MatchOrdered,
		Schema: []ExpectedColumn{
			{Name: "id", Type: testTypeINT64},
			{Name: testColumnName, Type: testTypeSTRING},
		},
		Rows: []map[string]any{{"id": "1", "label": testNameAda}},
	}
	// Actual schema has "label" instead of testColumnName.
	schema := schemaWith("id", testTypeINT64, "label", testTypeSTRING)
	actual := []bqtypes.Row{rowOf("1", testNameAda)}
	diff := rowDiff(exp, schema, actual)
	if diff == "" {
		t.Fatal("preflight: want non-empty diff for schema drift")
	}
	if !strings.Contains(diff, "schema mismatch") {
		t.Fatalf("want 'schema mismatch' in diff, got:\n%s", diff)
	}
}

func TestNumericEqualRejectsTypeDrift(t *testing.T) {
	// "abc" is not a number; the equality must refuse rather
	// than silently zeroing both sides.
	if numericEqual("abc", "1") {
		t.Fatal("numericEqual: must reject non-numeric expected")
	}
	if numericEqual("1", "abc") {
		t.Fatal("numericEqual: must reject non-numeric actual")
	}
}

func TestFloatEqualHandlesNaN(t *testing.T) {
	if !floatEqual(math.NaN(), math.NaN()) {
		t.Fatal("floatEqual: NaN vs NaN should be equal (treated as match for diff purposes)")
	}
	if floatEqual(math.NaN(), 1.0) {
		t.Fatal("floatEqual: NaN vs 1.0 should NOT match")
	}
}

func TestErrorDiffMatchesCodeAndMessage(t *testing.T) {
	body := []byte(
		`{"error":{"code":400,"message":"Syntax error: missing FROM near bad","status":"invalidQuery","errors":[{"reason":"invalidQuery","message":"Syntax error: missing FROM near bad"}]}}`,
	)
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
	got, err := resolveProfiles([]string{ProfileDuckDB})
	if err != nil {
		t.Fatalf("resolveProfiles: %v", err)
	}
	if len(got) != 1 || got[0].Name != ProfileDuckDB {
		t.Fatalf("got=%v, want [duckdb]", got)
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
