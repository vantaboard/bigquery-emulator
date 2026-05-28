package runner

import (
	"encoding/json"
	"fmt"
	"math"
	"math/big"
	"slices"
	"sort"
	"strconv"
	"strings"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// floatRelEpsilon is the relative tolerance used when comparing
// FLOAT64 cells. 1e-9 is loose enough to absorb the round-trip
// IEEE-754 noise that BigQuery's wire encoding introduces (the
// gateway formats float64s with `strconv.FormatFloat(v, 'g', -1, 64)`
// which is bit-exact, but the YAML decoder + JSON unmarshal pair on
// the expected side does pass values through `strconv.ParseFloat`).
const floatRelEpsilon = 1e-9

// rowDiff dispatches the actual row vs expected comparison based on
// the fixture's declared match mode. An empty string means PASS.
//
// The diff engine is mode-aware:
//
//   - MatchOrdered: pairwise compare row i ↔ actualRows[i] with typed
//     cell comparison driven by the gateway-supplied schema (INT64
//     compares as int, FLOAT64 with a relative epsilon, etc.). This
//     is the default and matches the plan-40 behavior.
//   - MatchUnordered: treats both sides as a multiset and compares
//     after type-aware canonicalization. Use when the storage engine
//     does not guarantee row order and the query lacks ORDER BY.
//   - MatchSchemaOnly: ignores `expected.rows` entirely and checks
//     that the response schema matches `expected.schema` (or, if no
//     explicit schema is declared, the column names taken from
//     `expected.rows[0]`).
func rowDiff(exp Expectation, schema *bqtypes.TableSchema, actualRows []bqtypes.Row) string {
	mode := exp.Match
	if mode == "" {
		mode = MatchOrdered
	}
	if mode == MatchSchemaOnly {
		return schemaDiff(exp, schema)
	}
	if diff := schemaPreflight(exp, schema); diff != "" {
		return diff
	}
	switch mode {
	case MatchOrdered:
		return orderedRowDiff(exp.Rows, schema, actualRows)
	case MatchUnordered:
		return unorderedRowDiff(exp.Rows, schema, actualRows)
	default:
		// Loader validates mode, so this is unreachable at run time.
		return fmt.Sprintf("internal: unknown match mode %q", mode)
	}
}

// schemaPreflight enforces an opt-in column-set assertion before the
// row diff runs. If the fixture declared `expected.schema:` it must
// match the gateway's response; otherwise we are silent.
func schemaPreflight(exp Expectation, actual *bqtypes.TableSchema) string {
	if len(exp.Schema) == 0 {
		return ""
	}
	return diffSchemaList(exp.Schema, actual, true)
}

// schemaDiff is the schema_only-mode entry point. Tries the explicit
// `expected.schema:` declaration first; falls back to the column-name
// set derived from `expected.rows[0]` if the fixture writer leaned on
// the rows-as-column-template shorthand.
func schemaDiff(exp Expectation, actual *bqtypes.TableSchema) string {
	if len(exp.Schema) > 0 {
		return diffSchemaList(exp.Schema, actual, true)
	}
	// Names-only fallback. Pull the expected column set from
	// rows[0] so a writer can pin "make sure these columns come
	// back" without having to spell out the type for each one.
	if len(exp.Rows) == 0 {
		// Loader rejects this combo, so this is only a safety
		// net.
		return "schema_only: nothing to compare against (no schema:, no rows:)"
	}
	expected := make([]ExpectedColumn, 0, len(exp.Rows[0]))
	for name := range exp.Rows[0] {
		expected = append(expected, ExpectedColumn{Name: name})
	}
	sort.Slice(expected, func(i, j int) bool { return expected[i].Name < expected[j].Name })
	return diffSchemaList(expected, actual, false)
}

// diffSchemaList compares a list of expected columns against the
// gateway's schema. checkTypes=true enforces the Type field on each
// column (case-insensitive); checkTypes=false is the names-only path
// used by the rows-shorthand for schema_only.
func diffSchemaList(expected []ExpectedColumn, actual *bqtypes.TableSchema, checkTypes bool) string {
	if actual == nil || len(actual.Fields) == 0 {
		return fmt.Sprintf(
			"schema mismatch: expected %d columns, gateway returned no schema",
			len(expected))
	}
	if len(expected) != len(actual.Fields) {
		return renderSchemaDiff(expected, actual)
	}

	// When the fallback path supplies expected as a sorted
	// column-name set, allow the actual schema's order to differ;
	// otherwise the comparison is positional (matches BigQuery's
	// `schema.fields[]` ordering semantics).
	if !checkTypes {
		actualNames := make([]string, 0, len(actual.Fields))
		for _, f := range actual.Fields {
			actualNames = append(actualNames, f.Name)
		}
		sort.Strings(actualNames)
		for i, n := range actualNames {
			if !strings.EqualFold(expected[i].Name, n) {
				return renderSchemaDiff(expected, actual)
			}
		}
		return ""
	}

	for i, e := range expected {
		a := actual.Fields[i]
		if !strings.EqualFold(e.Name, a.Name) {
			return renderSchemaDiff(expected, actual)
		}
		if e.Type != "" && !strings.EqualFold(e.Type, a.Type) {
			return renderSchemaDiff(expected, actual)
		}
	}
	return ""
}

// renderSchemaDiff prints both schemas side by side so the failing
// column or type is visible at a glance.
func renderSchemaDiff(expected []ExpectedColumn, actual *bqtypes.TableSchema) string {
	var b strings.Builder
	b.WriteString("schema mismatch\nexpected:\n")
	for _, c := range expected {
		if c.Type == "" {
			fmt.Fprintf(&b, "  %s\n", c.Name)
			continue
		}
		fmt.Fprintf(&b, "  %s:%s\n", c.Name, strings.ToUpper(c.Type))
	}
	b.WriteString("actual:\n")
	if actual == nil || len(actual.Fields) == 0 {
		b.WriteString("  (no schema)\n")
	} else {
		for _, f := range actual.Fields {
			fmt.Fprintf(&b, "  %s:%s\n", f.Name, strings.ToUpper(f.Type))
		}
	}
	return b.String()
}

// orderedRowDiff is the plan-40 default: row i is compared against
// actualRows[i] cell-by-cell. Typed comparison kicks in based on the
// column's SQL type from the gateway-supplied schema.
func orderedRowDiff(expected []map[string]any, schema *bqtypes.TableSchema, actualRows []bqtypes.Row) string {
	cols := schemaColumns(schema)
	types := schemaTypes(schema)
	if len(expected) == len(actualRows) {
		match := true
		for i := range expected {
			if !rowMatchesTyped(expected[i], actualRows[i], cols, types) {
				match = false
				break
			}
		}
		if match {
			return ""
		}
	}
	return unifiedDiff(
		renderExpectedRows(expected, cols, types),
		renderActualRows(actualRows, cols, types),
	)
}

// unorderedRowDiff compares the two sides as a multiset. Both sides
// are canonicalized to type-normalized strings and bucketed; any row
// with mismatched counts surfaces in the unified diff as
// "missing" (present only on the expected side) or
// "extra" (present only on the actual side).
//
// Float epsilon is best-effort under this mode: the canonicalizer
// rounds float64 values to 12 significant digits so values within
// ~1e-12 relative tolerance still bucket together. Ordered mode
// remains the right tool for fixtures whose tolerance budget is
// tighter than that.
func unorderedRowDiff(expected []map[string]any, schema *bqtypes.TableSchema, actualRows []bqtypes.Row) string {
	cols := schemaColumns(schema)
	types := schemaTypes(schema)

	expCanon, expLines := groupExpected(expected, cols, types)
	actCanon, actLines := groupActual(actualRows, cols, types)

	if multisetsEqual(expCanon, actCanon) {
		return ""
	}

	missing, extra := diffMultiset(expCanon, actCanon)
	sort.Strings(missing)
	sort.Strings(extra)
	sort.Strings(expLines)
	sort.Strings(actLines)
	return renderUnorderedDiff(expLines, actLines, missing, extra)
}

// groupExpected canonicalizes the expected rows and returns both the
// per-line multiset and the original (canonical) line ordering. The
// caller relies on the latter for the "expected (multiset)" stanza.
func groupExpected(expected []map[string]any, cols, types []string) (map[string]int, []string) {
	canon := make(map[string]int, len(expected))
	lines := make([]string, 0, len(expected))
	for _, r := range expected {
		line := canonicalExpectedRow(r, cols, types)
		canon[line]++
		lines = append(lines, line)
	}
	return canon, lines
}

// groupActual mirrors groupExpected for the engine-emitted rows.
func groupActual(actual []bqtypes.Row, cols, types []string) (map[string]int, []string) {
	canon := make(map[string]int, len(actual))
	lines := make([]string, 0, len(actual))
	for _, r := range actual {
		line := canonicalActualRow(r, cols, types)
		canon[line]++
		lines = append(lines, line)
	}
	return canon, lines
}

// multisetsEqual returns true when both line→count maps describe the
// same multiset (both sizes and per-key counts agree).
func multisetsEqual(a, b map[string]int) bool {
	if len(a) != len(b) {
		return false
	}
	for k, v := range a {
		if b[k] != v {
			return false
		}
	}
	return true
}

// diffMultiset returns the lines that appear too few times on the
// actual side ("missing") and too many times on the actual side
// ("extra"). Both slices are unsorted; the caller sorts for stable
// diff output.
func diffMultiset(exp, act map[string]int) (missing, extra []string) {
	for k, v := range exp {
		for range v - act[k] {
			missing = append(missing, k)
		}
	}
	for k, v := range act {
		for range v - exp[k] {
			extra = append(extra, k)
		}
	}
	return missing, extra
}

// renderUnorderedDiff materializes the user-facing multiset-diff
// string. Each stanza is emitted whether or not the corresponding
// slice is empty, except `missing`/`extra` which are skipped when
// the row count is zero (so a swap-only mismatch only prints the
// two multisets without phantom "missing:" / "extra:" headers).
func renderUnorderedDiff(expLines, actLines, missing, extra []string) string {
	var b strings.Builder
	b.WriteString("unordered row mismatch\nexpected (multiset):\n")
	writeRowStanza(&b, expLines)
	b.WriteString("actual (multiset):\n")
	writeRowStanza(&b, actLines)
	if len(missing) > 0 {
		b.WriteString("missing (expected but not in actual):\n")
		for _, line := range missing {
			b.WriteString("  ")
			b.WriteString(line)
			b.WriteString("\n")
		}
	}
	if len(extra) > 0 {
		b.WriteString("extra (actual but not in expected):\n")
		for _, line := range extra {
			b.WriteString("  ")
			b.WriteString(line)
			b.WriteString("\n")
		}
	}
	return b.String()
}

// writeRowStanza emits the indented row block for one side of the
// unordered diff, substituting the explicit "(no rows)" sentinel
// when the slice is empty so the renderer never collapses an empty
// section silently.
func writeRowStanza(b *strings.Builder, lines []string) {
	if len(lines) == 0 {
		b.WriteString("  (no rows)\n")
		return
	}
	for _, line := range lines {
		b.WriteString("  ")
		b.WriteString(line)
		b.WriteString("\n")
	}
}

// rowMatchesTyped is the per-row typed comparator used by ordered
// mode. Returns true when every cell in `expected` matches the
// corresponding cell in `actual` under the column's SQL type
// (INT64/NUMERIC compare as numbers, FLOAT64 with epsilon, etc.).
// Missing keys on either side are surfaced as mismatches so the
// diff exposes column-name drift.
func rowMatchesTyped(expected map[string]any, actual bqtypes.Row, cols []string, types []string) bool {
	for i, col := range cols {
		var actVal any
		if i < len(actual.F) {
			actVal = actual.F[i].V
		}
		expVal, hasExp := expected[col]
		if !hasExp {
			// Expected row lacks this column. If both sides are
			// "missing" we treat it as NULL; otherwise it is a
			// real divergence.
			if actVal == nil {
				continue
			}
			return false
		}
		fieldType := ""
		if i < len(types) {
			fieldType = types[i]
		}
		if !cellsEqual(expVal, actVal, fieldType) {
			return false
		}
	}
	// Reject extra keys on the expected side that the schema does
	// not include; otherwise the fixture writer could pin a column
	// the engine never returned and the diff would silently pass.
	for k := range expected {
		if !containsString(cols, k) {
			return false
		}
	}
	// Reject extra cells on the actual side that the schema does
	// not enumerate (the gateway should never do this, but the
	// belt-and-braces check keeps the diff honest if it does).
	if len(actual.F) > len(cols) {
		return false
	}
	return true
}

// cellsEqual is the type-aware cell equality predicate. It returns
// false for NULL-vs-non-NULL pairs and otherwise delegates to a
// per-type comparator. Fall-through for unknown types is the
// string-form compare used by plan-40.
func cellsEqual(expected, actual any, fieldType string) bool {
	expIsNull := isNullExpected(expected)
	actIsNull := actual == nil
	if expIsNull && actIsNull {
		return true
	}
	if expIsNull != actIsNull {
		return false
	}
	switch strings.ToUpper(strings.TrimSpace(fieldType)) {
	case bqTypeINT64, "INTEGER", "NUMERIC", "BIGNUMERIC":
		return numericEqual(expected, actual)
	case bqTypeFLOAT64, "FLOAT":
		return floatEqual(expected, actual)
	case "BOOL", "BOOLEAN":
		return boolEqual(expected, actual)
	case "TIMESTAMP", "DATE", "DATETIME", "TIME":
		return timeEqual(expected, actual)
	case bqTypeSTRING, "BYTES":
		return stringForm(expected) == stringForm(actual)
	default:
		// Unknown / empty type (e.g. STRUCT/REPEATED at the top
		// level, or the schema is absent): fall back to the
		// plan-40 stringy compare so nothing regresses for the
		// existing fixtures.
		return stringForm(expected) == stringForm(actual)
	}
}

// isNullExpected returns true when a YAML-decoded value is the
// canonical NULL marker. Distinguishes between `nil` (YAML `null`)
// and the literal string "NULL" (which a fixture would have to
// quote explicitly).
func isNullExpected(v any) bool {
	return v == nil
}

// numericEqual compares two values as exact rationals. INT64,
// NUMERIC, and BIGNUMERIC all use this path so a YAML `1` matches
// the wire `"1"` regardless of how either side wrote it. Returns
// false when either side cannot be parsed as a rational.
func numericEqual(expected, actual any) bool {
	e := toRat(expected)
	a := toRat(actual)
	if e == nil || a == nil {
		return false
	}
	return e.Cmp(a) == 0
}

// toRat best-effort parses a value into math/big.Rat. Integers,
// floats, and strings of either are all accepted; everything else
// returns nil so cellsEqual can flag a type drift instead of a
// silent zero-vs-zero pass.
func toRat(v any) *big.Rat {
	switch x := v.(type) {
	case nil:
		return nil
	case *big.Rat:
		return x
	case int:
		return new(big.Rat).SetInt64(int64(x))
	case int32:
		return new(big.Rat).SetInt64(int64(x))
	case int64:
		return new(big.Rat).SetInt64(x)
	case uint:
		r := new(big.Rat)
		r.SetUint64(uint64(x))
		return r
	case uint32:
		r := new(big.Rat)
		r.SetUint64(uint64(x))
		return r
	case uint64:
		r := new(big.Rat)
		r.SetUint64(x)
		return r
	case float32:
		r := new(big.Rat)
		r.SetFloat64(float64(x))
		return r
	case float64:
		r := new(big.Rat)
		r.SetFloat64(x)
		return r
	case string:
		s := strings.TrimSpace(x)
		if s == "" {
			return nil
		}
		if r, ok := new(big.Rat).SetString(s); ok {
			return r
		}
		return nil
	}
	return nil
}

// floatEqual compares two values as float64 with a relative
// epsilon (floatRelEpsilon). Special-cases exact zero so an
// expected-zero / actual-zero pair does not divide by zero.
func floatEqual(expected, actual any) bool {
	e, ok1 := toFloat(expected)
	a, ok2 := toFloat(actual)
	if !ok1 || !ok2 {
		return false
	}
	if math.IsNaN(e) || math.IsNaN(a) {
		return math.IsNaN(e) && math.IsNaN(a)
	}
	if e == a {
		return true
	}
	diff := math.Abs(e - a)
	norm := math.Max(math.Abs(e), math.Abs(a))
	if norm == 0 {
		return diff <= floatRelEpsilon
	}
	return diff/norm <= floatRelEpsilon
}

// toFloat parses a value into float64 best-effort. Strings of digit
// literals are accepted (BigQuery's wire format encodes everything
// as a string).
func toFloat(v any) (float64, bool) {
	switch x := v.(type) {
	case nil:
		return 0, false
	case float64:
		return x, true
	case float32:
		return float64(x), true
	case int:
		return float64(x), true
	case int32:
		return float64(x), true
	case int64:
		return float64(x), true
	case uint:
		return float64(x), true
	case uint64:
		return float64(x), true
	case string:
		s := strings.TrimSpace(x)
		f, err := strconv.ParseFloat(s, 64)
		if err != nil {
			return 0, false
		}
		return f, true
	}
	return 0, false
}

// boolEqual normalizes "true"/"false"/"1"/"0" forms (case
// insensitive) before comparison. The YAML decoder gives us a real
// bool; the wire gives us a string; the normalizer reconciles them.
func boolEqual(expected, actual any) bool {
	e, ok1 := toBool(expected)
	a, ok2 := toBool(actual)
	if !ok1 || !ok2 {
		return false
	}
	return e == a
}

// toBool returns the canonical bool form for a value. Strings are
// recognized as "true"/"false"/"t"/"f"/"1"/"0" (case insensitive);
// integers as 0/non-zero; anything else returns ok=false.
func toBool(v any) (bool, bool) {
	switch x := v.(type) {
	case bool:
		return x, true
	case string:
		switch strings.ToLower(strings.TrimSpace(x)) {
		case "true", "t", "1":
			return true, true
		case "false", "f", "0":
			return false, true
		}
	case int:
		return x != 0, true
	case int32:
		return x != 0, true
	case int64:
		return x != 0, true
	}
	return false, false
}

// timeEqual parses both sides as time.Time and compares for
// instant-equality. Accepts RFC3339 with optional nanoseconds, the
// SQL `YYYY-MM-DD HH:MM:SS[.fffffffff]` shape, plain dates, and the
// Unix-seconds-as-string form BigQuery uses for TIMESTAMP on the
// wire.
func timeEqual(expected, actual any) bool {
	e, ok1 := toTime(expected)
	a, ok2 := toTime(actual)
	if !ok1 || !ok2 {
		return false
	}
	return e.Equal(a)
}

var timeFormats = []string{
	time.RFC3339Nano,
	time.RFC3339,
	"2006-01-02T15:04:05.999999999",
	"2006-01-02T15:04:05",
	"2006-01-02 15:04:05.999999999 MST",
	"2006-01-02 15:04:05.999999999",
	"2006-01-02 15:04:05",
	"2006-01-02",
	"15:04:05.999999999",
	"15:04:05",
}

// toTime parses a value into time.Time. Returns ok=false when no
// recognized format matches.
func toTime(v any) (time.Time, bool) {
	switch x := v.(type) {
	case nil:
		return time.Time{}, false
	case time.Time:
		return x, true
	case string:
		if t, ok := parseTimestampString(x); ok {
			return t, true
		}
	case int:
		return time.Unix(int64(x), 0).UTC(), true
	case int64:
		return time.Unix(x, 0).UTC(), true
	case float64:
		sec := int64(x)
		nsec := int64((x - float64(sec)) * 1e9)
		return time.Unix(sec, nsec).UTC(), true
	}
	return time.Time{}, false
}

// parseTimestampString tries the registered RFC formats first, then
// falls back to BigQuery's TIMESTAMP wire form (Unix seconds with an
// optional fractional component). Pulled out of toTime so the
// fallback's natural conditional nesting stops tripping nestif.
func parseTimestampString(raw string) (time.Time, bool) {
	s := strings.TrimSpace(raw)
	if s == "" {
		return time.Time{}, false
	}
	for _, f := range timeFormats {
		if t, err := time.Parse(f, s); err == nil {
			return t.UTC(), true
		}
	}
	if t, ok := parseUnixSecondsString(s); ok {
		return t, true
	}
	if sec, err := strconv.ParseInt(s, 10, 64); err == nil {
		return time.Unix(sec, 0).UTC(), true
	}
	return time.Time{}, false
}

// parseUnixSecondsString parses BigQuery's `<sec>.<frac>` TIMESTAMP
// wire encoding without going through float64 (which would drop
// precision past microseconds). Returns ok=false when the input is
// not in the dotted form.
func parseUnixSecondsString(s string) (time.Time, bool) {
	before, after, ok := strings.Cut(s, ".")
	if !ok {
		return time.Time{}, false
	}
	sec, err := strconv.ParseInt(before, 10, 64)
	if err != nil {
		return time.Time{}, false
	}
	frac := after
	if len(frac) > 9 {
		frac = frac[:9]
	}
	for len(frac) < 9 {
		frac += "0"
	}
	nsec, err := strconv.ParseInt(frac, 10, 64)
	if err != nil {
		return time.Time{}, false
	}
	return time.Unix(sec, nsec).UTC(), true
}

// stringForm returns the canonical scalar string for the diff
// renderer. Mirrors plan-40 behaviour for STRING/BYTES, with the
// NULL sentinel kept distinct from the literal string "NULL".
func stringForm(v any) string {
	if v == nil {
		return "<NULL>"
	}
	switch x := v.(type) {
	case string:
		return x
	case bool:
		if x {
			return boolLiteralTrue
		}
		return boolLiteralFalse
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

// schemaColumns returns the schema's column names in declared order
// (mirrors the plan-40 helper).
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

// schemaTypes returns the schema's column types in declared order.
// Empty when the schema is nil so callers can rely on positional
// lookup without bounds-checking.
func schemaTypes(schema *bqtypes.TableSchema) []string {
	if schema == nil {
		return nil
	}
	out := make([]string, len(schema.Fields))
	for i, f := range schema.Fields {
		out[i] = f.Type
	}
	return out
}

// positionalName returns the column name at position i, falling
// back to `col<i>` when the schema is absent or too short.
func positionalName(cols []string, i int) string {
	if i < len(cols) {
		return cols[i]
	}
	return fmt.Sprintf("col%d", i)
}

// canonicalExpectedRow renders an expected row into its
// type-normalized one-line form (sorted by column name) so the
// unordered bucketing can compare it byte-for-byte.
func canonicalExpectedRow(r map[string]any, cols []string, types []string) string {
	pairs := make([]string, 0, len(cols)+len(r))
	seen := make(map[string]bool, len(cols))
	for i, c := range cols {
		ft := ""
		if i < len(types) {
			ft = types[i]
		}
		v, ok := r[c]
		if !ok {
			pairs = append(pairs, c+"=<missing>")
		} else {
			pairs = append(pairs, fmt.Sprintf("%s=%s", c, canonicalCell(v, ft)))
		}
		seen[c] = true
	}
	// Surface stray expected columns that the schema does not
	// know about so the diff exposes the divergence.
	extras := make([]string, 0)
	for k := range r {
		if !seen[k] {
			extras = append(extras, k)
		}
	}
	sort.Strings(extras)
	for _, k := range extras {
		pairs = append(pairs, fmt.Sprintf("%s=%s", k, canonicalCell(r[k], "")))
	}
	return "{" + strings.Join(pairs, ", ") + "}"
}

// canonicalActualRow renders one wire-format row into the same
// canonical form `canonicalExpectedRow` emits.
func canonicalActualRow(r bqtypes.Row, cols []string, types []string) string {
	pairs := make([]string, 0, len(r.F))
	for i, cell := range r.F {
		name := positionalName(cols, i)
		ft := ""
		if i < len(types) {
			ft = types[i]
		}
		pairs = append(pairs, fmt.Sprintf("%s=%s", name, canonicalCell(cell.V, ft)))
	}
	return "{" + strings.Join(pairs, ", ") + "}"
}

// canonicalCell renders one value into its type-normalized text
// form. The result is what both sides of the unordered multiset
// bucket on, so the implementation must be deterministic across
// "1" vs 1, "true" vs true, 1.0 vs "1.0", etc.
func canonicalCell(v any, fieldType string) string {
	if v == nil {
		return "<NULL>"
	}
	switch strings.ToUpper(strings.TrimSpace(fieldType)) {
	case bqTypeINT64, "INTEGER", "NUMERIC", "BIGNUMERIC":
		if r := toRat(v); r != nil {
			return r.RatString()
		}
	case bqTypeFLOAT64, "FLOAT":
		if f, ok := toFloat(v); ok {
			// 12 significant digits absorbs ~1e-12 relative
			// drift; ordered-mode epsilon still applies for
			// tighter tolerances.
			return strconv.FormatFloat(f, 'g', 12, 64)
		}
	case "BOOL", "BOOLEAN":
		if b, ok := toBool(v); ok {
			if b {
				return boolLiteralTrue
			}
			return boolLiteralFalse
		}
	case "TIMESTAMP", "DATE", "DATETIME", "TIME":
		if t, ok := toTime(v); ok {
			return t.UTC().Format(time.RFC3339Nano)
		}
	}
	return stringForm(v)
}

// renderExpectedRows is the diff-rendering helper for the ordered
// path. Mirrors the plan-40 layout (one row per line, sorted keys)
// so the new typed diff stays scannable.
func renderExpectedRows(rows []map[string]any, cols []string, types []string) []string {
	out := make([]string, 0, len(rows))
	for i, r := range rows {
		out = append(out, fmt.Sprintf("row %d: %s", i, canonicalExpectedRow(r, cols, types)))
	}
	return out
}

// renderActualRows is the diff-rendering helper for the actual side.
func renderActualRows(rows []bqtypes.Row, cols []string, types []string) []string {
	out := make([]string, 0, len(rows))
	for i, r := range rows {
		out = append(out, fmt.Sprintf("row %d: %s", i, canonicalActualRow(r, cols, types)))
	}
	return out
}

// unifiedDiff is the side-by-side expected-vs-actual renderer used
// for the ordered-mode mismatch path. See plan-40 for the rationale
// of not running a full Myers diff: fixture row counts are small and
// a side-by-side listing is more legible than a hunk-grouped diff.
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

// snippet truncates a body for inclusion in a diff message; the
// body can be large (the engine emits ZetaSQL parse-error pointers)
// and we want the diff to stay scannable.
func snippet(b []byte) string {
	const limit = 240
	s := strings.TrimSpace(string(b))
	if len(s) > limit {
		s = s[:limit] + "..."
	}
	return s
}

func containsString(haystack []string, needle string) bool {
	return slices.Contains(haystack, needle)
}
