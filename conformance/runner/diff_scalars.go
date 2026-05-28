package runner

import (
	"encoding/json"
	"fmt"
	"math"
	"math/big"
	"strconv"
	"strings"
	"time"
)

// floatRelEpsilon is the relative tolerance used when comparing
// FLOAT64 cells. 1e-9 is loose enough to absorb the round-trip
// IEEE-754 noise that BigQuery's wire encoding introduces (the
// gateway formats float64s with `strconv.FormatFloat(v, 'g', -1, 64)`
// which is bit-exact, but the YAML decoder + JSON unmarshal pair on
// the expected side does pass values through `strconv.ParseFloat`).
const floatRelEpsilon = 1e-9

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
