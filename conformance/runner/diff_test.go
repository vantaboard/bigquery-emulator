package runner

import (
	"math"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

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
