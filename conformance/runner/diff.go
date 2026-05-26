package runner

import (
	"encoding/json"
	"fmt"
	"sort"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// rowDiff compares the gateway's response against the fixture's
// expected row set and returns a non-empty unified-diff string on
// mismatch. An empty return means the rows match.
//
// We normalize both sides to `[{column: value, ...}, ...]` keyed by
// the schema's column names, then JSON-marshal each row as a stable
// pretty string. The textual diff is rendered against those lines so
// the diff output is human-readable.
func rowDiff(expected []map[string]any, schema *bqtypes.TableSchema, actualRows []bqtypes.Row) string {
	cols := schemaColumns(schema)
	expectedNormalized := normalizeExpectedRows(expected, cols)
	actualNormalized := normalizeActualRows(actualRows, cols)
	if len(expectedNormalized) == len(actualNormalized) {
		match := true
		for i := range expectedNormalized {
			if !rowsEqual(expectedNormalized[i], actualNormalized[i]) {
				match = false
				break
			}
		}
		if match {
			return ""
		}
	}
	return unifiedDiff(renderRows(expectedNormalized), renderRows(actualNormalized))
}

// schemaColumns returns the schema's column names in declared order.
// When the gateway omits the schema (e.g. some legacy
// `getQueryResults` paths), we fall back to positional column names
// (`col0`, `col1`, ...) so the diff still tells the user which cell
// was off.
func schemaColumns(schema *bqtypes.TableSchema) []string {
	if schema == nil {
		return nil
	}
	out := make([]string, len(schema.Fields))
	for i, f := range schema.Fields {
		out[i] = f.Name
	}
	return out
}

// normalizeExpectedRows turns the YAML-decoded expected rows into a
// canonical `[]map[string]string` where values are the
// `fmt.Sprint(...)` form (matching the wire's string-cell encoding).
// Missing keys in the YAML map are reported as `<missing>` so the
// diff is informative.
func normalizeExpectedRows(rows []map[string]any, cols []string) []map[string]string {
	out := make([]map[string]string, len(rows))
	for i, r := range rows {
		out[i] = canonicalizeRow(r, cols)
	}
	return out
}

// normalizeActualRows extracts each cell value from the BigQuery
// wire shape (nested `f`/`v` arrays) and keys it by the schema's
// column name. Nested STRUCT / REPEATED cells are JSON-encoded so
// the diff stays one line per row.
func normalizeActualRows(rows []bqtypes.Row, cols []string) []map[string]string {
	out := make([]map[string]string, len(rows))
	for i, r := range rows {
		m := make(map[string]string, len(r.F))
		for j, cell := range r.F {
			name := positionalName(cols, j)
			m[name] = cellString(cell.V)
		}
		out[i] = m
	}
	return out
}

// canonicalizeRow strings every value in the YAML expected map and,
// if columns are known, fills missing columns with `<missing>` so the
// diff shows the column-name divergence rather than silently passing.
// Extra YAML keys not in cols are kept under their literal name so
// fixtures cannot pin columns that the engine never returned.
func canonicalizeRow(r map[string]any, cols []string) map[string]string {
	out := make(map[string]string, len(r))
	for k, v := range r {
		out[k] = valueString(v)
	}
	for _, c := range cols {
		if _, ok := out[c]; !ok {
			out[c] = "<missing>"
		}
	}
	return out
}

// positionalName returns the column name at position i, falling back
// to `col<i>` when the schema is absent or too short. The fallback
// keeps the diff readable even when the gateway omits the schema
// envelope (some `getQueryResults` paths do this; see
// gateway/handlers/queries.go).
func positionalName(cols []string, i int) string {
	if i < len(cols) {
		return cols[i]
	}
	return fmt.Sprintf("col%d", i)
}

// cellString renders one cell value as a comparable string. Scalars
// already come back as strings on the BigQuery wire; nested objects
// (STRUCT cells and REPEATED arrays) are JSON-encoded so the diff
// keeps one row per line.
func cellString(v any) string {
	if v == nil {
		return "NULL"
	}
	if s, ok := v.(string); ok {
		return s
	}
	// Anything else (a map for STRUCT, a slice for REPEATED) gets
	// the JSON representation; deterministic enough for diffs and
	// roundtrips cleanly through `json.Marshal`.
	b, err := json.Marshal(v)
	if err != nil {
		return fmt.Sprintf("%v", v)
	}
	return string(b)
}

// valueString renders a YAML-decoded expected value as the same
// stringy form `cellString` emits, so the two sides can compare for
// equality without type drift.
func valueString(v any) string {
	if v == nil {
		return "NULL"
	}
	switch x := v.(type) {
	case string:
		return x
	case bool:
		if x {
			return "true"
		}
		return "false"
	case int, int32, int64, uint, uint32, uint64:
		return fmt.Sprintf("%d", x)
	case float32, float64:
		return fmt.Sprintf("%v", x)
	default:
		b, err := json.Marshal(v)
		if err != nil {
			return fmt.Sprintf("%v", v)
		}
		return string(b)
	}
}

// rowsEqual is a strict map equality check on the canonical row form.
func rowsEqual(a, b map[string]string) bool {
	if len(a) != len(b) {
		return false
	}
	for k, v := range a {
		if bv, ok := b[k]; !ok || bv != v {
			return false
		}
	}
	return true
}

// renderRows turns a slice of canonical rows into a deterministic
// multi-line string for the unified diff: one row per line, keys in
// alphabetical order so the output is byte-stable.
func renderRows(rows []map[string]string) []string {
	out := make([]string, 0, len(rows))
	for i, r := range rows {
		keys := make([]string, 0, len(r))
		for k := range r {
			keys = append(keys, k)
		}
		sort.Strings(keys)
		var b strings.Builder
		fmt.Fprintf(&b, "row %d: {", i)
		for j, k := range keys {
			if j > 0 {
				b.WriteString(", ")
			}
			fmt.Fprintf(&b, "%s=%q", k, r[k])
		}
		b.WriteString("}")
		out = append(out, b.String())
	}
	return out
}

// unifiedDiff is a minimal expected-vs-actual diff renderer. Not a
// full Myers diff (we do not need O(n+m) here -- conformance row
// counts are small and a side-by-side listing is more legible than
// a hunk-grouped diff). Format:
//
//	expected:
//	  row 0: {...}
//	  row 1: {...}
//	actual:
//	  row 0: {...}
func unifiedDiff(expected, actual []string) string {
	var b strings.Builder
	b.WriteString("expected:\n")
	if len(expected) == 0 {
		b.WriteString("  (no rows)\n")
	}
	for _, line := range expected {
		b.WriteString("  ")
		b.WriteString(line)
		b.WriteString("\n")
	}
	b.WriteString("actual:\n")
	if len(actual) == 0 {
		b.WriteString("  (no rows)\n")
	}
	for _, line := range actual {
		b.WriteString("  ")
		b.WriteString(line)
		b.WriteString("\n")
	}
	return b.String()
}

// errorDiff compares the gateway's error envelope against an
// `expected.error` block and returns an empty string on match or a
// human-readable message on mismatch.
func errorDiff(expected ExpectedError, status int, body []byte) string {
	var env struct {
		Error struct {
			Code    int    `json:"code"`
			Message string `json:"message"`
			Status  string `json:"status"`
			Errors  []struct {
				Reason  string `json:"reason"`
				Message string `json:"message"`
			} `json:"errors"`
		} `json:"error"`
	}
	_ = json.Unmarshal(body, &env)

	if expected.Code != 0 && expected.Code != status {
		return fmt.Sprintf("error code: expected %d, got %d (body: %s)",
			expected.Code, status, snippet(body))
	}
	if expected.MessageContains != "" {
		hay := env.Error.Message
		if hay == "" && len(env.Error.Errors) > 0 {
			hay = env.Error.Errors[0].Message
		}
		if !strings.Contains(hay, expected.MessageContains) {
			return fmt.Sprintf(
				"error message: expected to contain %q, got %q (body: %s)",
				expected.MessageContains, hay, snippet(body))
		}
	}
	return ""
}

// snippet truncates a body for inclusion in a diff message; the body
// can be large (the engine emits ZetaSQL parse-error pointers) and
// we want the diff to stay scannable.
func snippet(b []byte) string {
	const limit = 240
	s := strings.TrimSpace(string(b))
	if len(s) > limit {
		s = s[:limit] + "..."
	}
	return s
}
