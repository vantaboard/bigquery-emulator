//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestQuerySemanticFunctionBitCountNegative pins the BigQuery
// two's-complement contract for `BIT_COUNT(-1)` end-to-end. The
// row in `functions.yaml` was previously `unsupported` (the
// BigQuery flavor counts the sign-bit pattern while DuckDB's
// `bit_count` accepts only unsigned arguments). After the row
// flipped to `semantic_executor`, the route classifier promotes
// the scalar-only SELECT to the local executor and
// `numeric_edges::BitCount` returns 64.
//
// See `.cursor/plans/local-exec-09-date-time.plan.md`.
func TestQuerySemanticFunctionBitCountNegative(t *testing.T) {
	env := startEmulator(t)

	const projectID = "proj-semfn-bit-count-neg"
	base := env.URL() + "/bigquery/v2/projects/" + projectID
	status, body := doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":"SELECT BIT_COUNT(-1)","useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("jobs.query BIT_COUNT(-1) -> %d: %s", status, string(body))
	}
	var resp bqtypes.QueryResponse
	if err := json.Unmarshal(body, &resp); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if len(resp.Rows) != 1 || len(resp.Rows[0].F) != 1 {
		t.Fatalf("rows shape = %+v, want one row with one cell", resp.Rows)
	}
	if v := resp.Rows[0].F[0].V; v != "64" {
		t.Errorf("rows[0].f[0].v = %v, want %q", v, "64")
	}
}

// TestQuerySemanticFunctionBitCountSmall checks the small-int
// popcount path so the impl does not regress for the common case.
func TestQuerySemanticFunctionBitCountSmall(t *testing.T) {
	env := startEmulator(t)

	const projectID = "proj-semfn-bit-count-small"
	base := env.URL() + "/bigquery/v2/projects/" + projectID
	status, body := doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":"SELECT BIT_COUNT(7)","useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("jobs.query BIT_COUNT(7) -> %d: %s", status, string(body))
	}
	var resp bqtypes.QueryResponse
	if err := json.Unmarshal(body, &resp); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if len(resp.Rows) != 1 || len(resp.Rows[0].F) != 1 {
		t.Fatalf("rows shape = %+v, want one row with one cell", resp.Rows)
	}
	if v := resp.Rows[0].F[0].V; v != "3" {
		t.Errorf("rows[0].f[0].v = %v, want %q", v, "3")
	}
}

// TestQuerySemanticFunctionIeeeDivideByZeroProducesInf pins the
// BigQuery IEEE_DIVIDE contract: division by zero does NOT
// error; it returns the IEEE 754 sentinel (+Inf here). The row
// exists because DuckDB's `/` raises on FLOAT64 / 0 and SAFE_DIVIDE
// returns NULL -- neither matches BigQuery's sentinel contract.
func TestQuerySemanticFunctionIeeeDivideByZeroProducesInf(t *testing.T) {
	env := startEmulator(t)

	const projectID = "proj-semfn-ieee-divide-inf"
	base := env.URL() + "/bigquery/v2/projects/" + projectID
	status, body := doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":"SELECT IEEE_DIVIDE(1.0, 0.0)","useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("jobs.query IEEE_DIVIDE(1,0) -> %d: %s", status, string(body))
	}
	var resp bqtypes.QueryResponse
	if err := json.Unmarshal(body, &resp); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if len(resp.Rows) != 1 || len(resp.Rows[0].F) != 1 {
		t.Fatalf("rows shape = %+v, want one row with one cell", resp.Rows)
	}
	v, _ := resp.Rows[0].F[0].V.(string)
	if !strings.EqualFold(v, "+Inf") && !strings.EqualFold(v, "Infinity") {
		t.Errorf("rows[0].f[0].v = %v, want +Inf-like sentinel", resp.Rows[0].F[0].V)
	}
}

// TestQuerySemanticFunctionIeeeDivideZeroOverZeroProducesNaN pins
// the 0/0 sentinel path: BigQuery returns NaN, never errors.
func TestQuerySemanticFunctionIeeeDivideZeroOverZeroProducesNaN(t *testing.T) {
	env := startEmulator(t)

	const projectID = "proj-semfn-ieee-divide-nan"
	base := env.URL() + "/bigquery/v2/projects/" + projectID
	status, body := doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":"SELECT IEEE_DIVIDE(0.0, 0.0)","useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("jobs.query IEEE_DIVIDE(0,0) -> %d: %s", status, string(body))
	}
	var resp bqtypes.QueryResponse
	if err := json.Unmarshal(body, &resp); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if len(resp.Rows) != 1 || len(resp.Rows[0].F) != 1 {
		t.Fatalf("rows shape = %+v, want one row with one cell", resp.Rows)
	}
	v, _ := resp.Rows[0].F[0].V.(string)
	if !strings.EqualFold(v, "NaN") {
		t.Errorf("rows[0].f[0].v = %v, want NaN", resp.Rows[0].F[0].V)
	}
}

// TestQuerySemanticFunctionSafeDivideByZeroProducesNull pins the
// SAFE_DIVIDE NULL-swap contract end-to-end through the gateway.
// The bare `/` operator on the same operands surfaces a
// `divisionByZero` HTTP 400 envelope; SAFE_DIVIDE swaps that to a
// JSON `null` cell via the semantic executor's `WrapSafe`
// trampoline.
func TestQuerySemanticFunctionSafeDivideByZeroProducesNull(t *testing.T) {
	env := startEmulator(t)

	const projectID = "proj-semfn-safe-divide-null"
	base := env.URL() + "/bigquery/v2/projects/" + projectID
	status, body := doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":"SELECT SAFE_DIVIDE(1.0, 0.0)","useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("jobs.query SAFE_DIVIDE(1,0) -> %d: %s", status, string(body))
	}
	var resp bqtypes.QueryResponse
	if err := json.Unmarshal(body, &resp); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if len(resp.Rows) != 1 || len(resp.Rows[0].F) != 1 {
		t.Fatalf("rows shape = %+v, want one row with one cell", resp.Rows)
	}
	if resp.Rows[0].F[0].V != nil {
		t.Errorf("SAFE_DIVIDE(1,0) should produce NULL; got %v",
			resp.Rows[0].F[0].V)
	}
}

// TestQuerySemanticFunctionSoundexReferenceValue exercises the
// classic SOUNDEX algorithm end-to-end through the gateway.
// `SOUNDEX('Ashcraft')` returns "A261" -- BigQuery's documented
// example output -- through the semantic executor (DuckDB v1.5.3
// does not ship a `soundex` scalar at all, so this surface is
// only reachable via the semantic-functions route).
func TestQuerySemanticFunctionSoundexReferenceValue(t *testing.T) {
	env := startEmulator(t)

	const projectID = "proj-semfn-soundex"
	base := env.URL() + "/bigquery/v2/projects/" + projectID
	status, body := doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":"SELECT SOUNDEX('Ashcraft')",`+
			`"useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("jobs.query SOUNDEX('Ashcraft') -> %d: %s",
			status, string(body))
	}
	var resp bqtypes.QueryResponse
	if err := json.Unmarshal(body, &resp); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if len(resp.Rows) != 1 || len(resp.Rows[0].F) != 1 {
		t.Fatalf("rows shape = %+v, want one row with one cell", resp.Rows)
	}
	if v := resp.Rows[0].F[0].V; v != "A261" {
		t.Errorf("rows[0].f[0].v = %v, want %q", v, "A261")
	}
}

// TestQuerySemanticFunctionInstrReversePosition pins the
// negative-position INSTR surface DuckDB's 2-arg `instr` cannot
// model. `INSTR('ababab', 'ab', -1)` returns 5 (the rightmost
// match's 1-based byte index).
func TestQuerySemanticFunctionInstrReversePosition(t *testing.T) {
	env := startEmulator(t)

	const projectID = "proj-semfn-instr-reverse"
	base := env.URL() + "/bigquery/v2/projects/" + projectID
	status, body := doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":"SELECT INSTR('ababab', 'ab', -1)",`+
			`"useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("jobs.query INSTR reverse -> %d: %s",
			status, string(body))
	}
	var resp bqtypes.QueryResponse
	if err := json.Unmarshal(body, &resp); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if len(resp.Rows) != 1 || len(resp.Rows[0].F) != 1 {
		t.Fatalf("rows shape = %+v, want one row with one cell", resp.Rows)
	}
	if v := resp.Rows[0].F[0].V; v != "5" {
		t.Errorf("rows[0].f[0].v = %v, want %q", v, "5")
	}
}

// TestQuerySemanticFunctionInstrNthOccurrence covers the 4-arg
// form -- the surface DuckDB's `instr` cannot model at all. The
// dispatch picks the Nth match starting from `position`.
func TestQuerySemanticFunctionInstrNthOccurrence(t *testing.T) {
	env := startEmulator(t)

	const projectID = "proj-semfn-instr-nth"
	base := env.URL() + "/bigquery/v2/projects/" + projectID
	status, body := doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":"SELECT INSTR('ababab', 'ab', 1, 3)",`+
			`"useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("jobs.query INSTR Nth -> %d: %s",
			status, string(body))
	}
	var resp bqtypes.QueryResponse
	if err := json.Unmarshal(body, &resp); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if len(resp.Rows) != 1 || len(resp.Rows[0].F) != 1 {
		t.Fatalf("rows shape = %+v, want one row with one cell", resp.Rows)
	}
	if v := resp.Rows[0].F[0].V; v != "5" {
		t.Errorf("rows[0].f[0].v = %v, want %q", v, "5")
	}
}

// TestQuerySemanticFunctionSafeNegateOverflowProducesNull pins the
// SAFE_NEGATE NULL-swap contract on the `-INT64_MIN` overflow
// corner. Bare `-` on the same input surfaces a structured
// `overflow` envelope; SAFE_NEGATE swaps it to NULL.
func TestQuerySemanticFunctionSafeNegateOverflowProducesNull(t *testing.T) {
	env := startEmulator(t)

	const projectID = "proj-semfn-safe-negate-null"
	base := env.URL() + "/bigquery/v2/projects/" + projectID
	status, body := doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":"SELECT SAFE_NEGATE(-9223372036854775808)",`+
			`"useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("jobs.query SAFE_NEGATE overflow -> %d: %s",
			status, string(body))
	}
	var resp bqtypes.QueryResponse
	if err := json.Unmarshal(body, &resp); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if len(resp.Rows) != 1 || len(resp.Rows[0].F) != 1 {
		t.Fatalf("rows shape = %+v, want one row with one cell", resp.Rows)
	}
	if resp.Rows[0].F[0].V != nil {
		t.Errorf("SAFE_NEGATE overflow should produce NULL; got %v",
			resp.Rows[0].F[0].V)
	}
}
