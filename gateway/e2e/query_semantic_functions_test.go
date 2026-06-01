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
// See `.cursor/plans/semantic-functions-compliance.plan.md`.
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
