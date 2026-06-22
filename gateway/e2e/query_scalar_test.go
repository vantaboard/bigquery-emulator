//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestQueryScalarSelectArithmetic exercises the
// `docs/ENGINE_POLICY.md` scalar-only SELECT path
// end-to-end: `SELECT 1 + 2` lowers through the
// semantic executor (no FROM clause -> classifier promotes to the
// `kSemanticExecutor` route) and returns a single-row Arrow batch
// matching the same wire envelope the DuckDB fast path would
// produce for the same query.
func TestQueryScalarSelectArithmetic(t *testing.T) {
	env := startEmulator(t)

	const projectID = "proj-scalar-add"
	base := env.URL() + "/bigquery/v2/projects/" + projectID
	status, body := doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":"SELECT 1 + 2","useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("jobs.query -> %d: %s", status, string(body))
	}
	var resp bqtypes.QueryResponse
	if err := json.Unmarshal(body, &resp); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if !resp.JobComplete {
		t.Error("jobComplete = false, want true")
	}
	if resp.TotalRows != "1" {
		t.Errorf("totalRows = %q, want %q", resp.TotalRows, "1")
	}
	if len(resp.Rows) != 1 || len(resp.Rows[0].F) != 1 {
		t.Fatalf("rows shape = %+v, want one row with one cell", resp.Rows)
	}
	if v := resp.Rows[0].F[0].V; v != "3" {
		t.Errorf("rows[0].f[0].v = %v, want %q", v, "3")
	}
}

// TestQueryScalarSelectNullPropagation pins BigQuery's NULL
// semantics: `NULL + 1` evaluates to NULL even though the
// non-null operand is well-formed. The cell surfaces on the wire
// as a JSON `null`.
func TestQueryScalarSelectNullPropagation(t *testing.T) {
	env := startEmulator(t)

	const projectID = "proj-scalar-null"
	base := env.URL() + "/bigquery/v2/projects/" + projectID
	status, body := doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":"SELECT CAST(NULL AS INT64) + 1","useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("jobs.query -> %d: %s", status, string(body))
	}
	var resp bqtypes.QueryResponse
	if err := json.Unmarshal(body, &resp); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if len(resp.Rows) != 1 {
		t.Fatalf("rows shape = %+v, want one row", resp.Rows)
	}
	cell := resp.Rows[0].F[0].V
	if cell != nil {
		t.Errorf("rows[0].f[0].v = %v, want JSON null", cell)
	}
}

// TestQueryScalarSelectDivisionByZeroSurfacesError pins the
// BigQuery error surface for `SELECT 1.0 / 0` -- the semantic
// executor surfaces an INVALID_ARGUMENT status with a structured
// `kDivisionByZero` reason payload that the gateway maps onto an
// HTTP 400 `invalidQuery` envelope.
func TestQueryScalarSelectDivisionByZeroSurfacesError(t *testing.T) {
	env := startEmulator(t)

	const projectID = "proj-scalar-divzero"
	base := env.URL() + "/bigquery/v2/projects/" + projectID
	status, body := doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":"SELECT 1.0 / 0","useLegacySql":false}`))
	if status != http.StatusBadRequest {
		t.Fatalf("jobs.query divisionByZero -> %d, want 400; body=%s",
			status, string(body))
	}
	if !strings.Contains(string(body), "division") &&
		!strings.Contains(string(body), "zero") {
		t.Errorf("response body should mention division by zero; got %s",
			string(body))
	}
}

// TestQueryScalarSelectSafeAddOverflow pins the SAFE_<fn> family:
// an arithmetic overflow inside a SAFE-prefixed call returns NULL
// instead of erroring out. This is BigQuery's documented contract
// for SAFE_ADD / SAFE_DIVIDE / ... and is owned by the semantic
// executor (the DuckDB fast path uses different overflow rules).
func TestQueryScalarSelectSafeAddOverflow(t *testing.T) {
	env := startEmulator(t)

	const projectID = "proj-scalar-safeadd"
	base := env.URL() + "/bigquery/v2/projects/" + projectID
	status, body := doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":"SELECT SAFE_ADD(9223372036854775807, 1)",`+
			`"useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("jobs.query SAFE_ADD overflow -> %d; body=%s",
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
		t.Errorf("SAFE_ADD overflow should produce NULL; got %v",
			resp.Rows[0].F[0].V)
	}
}

// TestQueryScalarSelectCaseExpression covers the CASE branch the
// semantic executor handles: the analyzer lowers
// `CASE WHEN ... END` onto a `$case_no_value` function call and
// the executor walks it picking the first TRUE branch (or the
// ELSE result when none matches).
func TestQueryScalarSelectCaseExpression(t *testing.T) {
	env := startEmulator(t)

	const projectID = "proj-scalar-case"
	base := env.URL() + "/bigquery/v2/projects/" + projectID
	status, body := doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":"SELECT CASE WHEN 1 = 2 THEN 'a' WHEN 1 = 1 `+
			`THEN 'b' ELSE 'c' END","useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("jobs.query CASE -> %d; body=%s", status, string(body))
	}
	var resp bqtypes.QueryResponse
	if err := json.Unmarshal(body, &resp); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if len(resp.Rows) != 1 || len(resp.Rows[0].F) != 1 {
		t.Fatalf("rows shape = %+v, want one row with one cell", resp.Rows)
	}
	if v := resp.Rows[0].F[0].V; v != "b" {
		t.Errorf("rows[0].f[0].v = %v, want %q", v, "b")
	}
}

// TestQueryScalarSelectNamedParameter exercises the gateway's
// REST `queryParameters` plumbing end-to-end: the request carries
// `@p = 41`, the gateway converts it to the engine's name-keyed
// proto map, the coordinator declares the parameter to the
// analyzer (so the resolved AST contains a typed
// `ResolvedParameter`), and the semantic executor binds the value
// off `request.parameters`. The output cell is the analyzer's
// resolved sum.
func TestQueryScalarSelectNamedParameter(t *testing.T) {
	env := startEmulator(t)

	const projectID = "proj-scalar-param"
	base := env.URL() + "/bigquery/v2/projects/" + projectID
	const body = `{
	  "query": "SELECT @p + 1",
	  "useLegacySql": false,
	  "queryParameters": [{
	    "name": "p",
	    "parameterType": {"type": "INT64"},
	    "parameterValue": {"value": "41"}
	  }]
	}`
	status, respBody := doJSON(t, http.MethodPost, base+"/queries", []byte(body))
	if status != http.StatusOK {
		t.Fatalf("jobs.query @p -> %d; body=%s", status, string(respBody))
	}
	var resp bqtypes.QueryResponse
	if err := json.Unmarshal(respBody, &resp); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(respBody))
	}
	if len(resp.Rows) != 1 || len(resp.Rows[0].F) != 1 {
		t.Fatalf("rows shape = %+v, want one row with one cell", resp.Rows)
	}
	if v := resp.Rows[0].F[0].V; v != "42" {
		t.Errorf("rows[0].f[0].v = %v, want %q", v, "42")
	}
}

// TestQueryScalarSelectTimestampParameterIsoWithoutTimezone exercises
// TIMESTAMP query parameters that omit an explicit timezone suffix.
func TestQueryScalarSelectTimestampParameterIsoWithoutTimezone(t *testing.T) {
	env := startEmulator(t)

	const projectID = "proj-scalar-timestamp-param"
	base := env.URL() + "/bigquery/v2/projects/" + projectID
	const body = `{
	  "query": "SELECT @reference_dt",
	  "useLegacySql": false,
	  "queryParameters": [{
	    "name": "reference_dt",
	    "parameterType": {"type": "TIMESTAMP"},
	    "parameterValue": {"value": "2026-06-22T10:00:00"}
	  }]
	}`
	status, respBody := doJSON(t, http.MethodPost, base+"/queries", []byte(body))
	if status != http.StatusOK {
		t.Fatalf("jobs.query @reference_dt -> %d; body=%s", status, string(respBody))
	}
	var resp bqtypes.QueryResponse
	if err := json.Unmarshal(respBody, &resp); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(respBody))
	}
	if len(resp.Rows) != 1 || len(resp.Rows[0].F) != 1 {
		t.Fatalf("rows shape = %+v, want one row with one cell", resp.Rows)
	}
	if v := resp.Rows[0].F[0].V; v != "2026-06-22 10:00:00 UTC" && v != "2026-06-22 10:00:00+00" {
		t.Errorf("rows[0].f[0].v = %v, want UTC timestamp for 2026-06-22T10:00:00", v)
	}
}
