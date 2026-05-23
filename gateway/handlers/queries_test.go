package handlers

import (
	"bytes"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
)

// runQuery posts the given JSON body to QueryRun and returns the
// recorder for inspection.
func runQuery(t *testing.T, body string) *httptest.ResponseRecorder {
	t.Helper()
	rec := httptest.NewRecorder()
	req := httptest.NewRequest(http.MethodPost,
		"/bigquery/v2/projects/test/queries", strings.NewReader(body))
	req.Header.Set("Content-Type", "application/json")
	QueryRun(Dependencies{})(rec, req)
	return rec
}

// TestQueryRunRejectsLegacySQL asserts that an explicit
// `useLegacySql=true` produces a BigQuery-shaped 400 with
// `reason: invalidQuery`, per docs/REST_API.md "SQL dialect".
func TestQueryRunRejectsLegacySQL(t *testing.T) {
	body, err := json.Marshal(map[string]any{
		"query":        "SELECT 1",
		"useLegacySql": true,
	})
	if err != nil {
		t.Fatalf("marshal: %v", err)
	}

	rec := runQuery(t, string(body))

	if rec.Code != http.StatusBadRequest {
		t.Fatalf("status = %d, want %d", rec.Code, http.StatusBadRequest)
	}
	var env errorEnvelope
	if err := json.NewDecoder(rec.Body).Decode(&env); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if env.Error.Code != http.StatusBadRequest {
		t.Fatalf("error.code = %d, want %d", env.Error.Code, http.StatusBadRequest)
	}
	if env.Error.Status != "invalidQuery" {
		t.Fatalf("error.status = %q, want %q", env.Error.Status, "invalidQuery")
	}
	if len(env.Error.Errors) == 0 {
		t.Fatal("error.errors is empty; BigQuery clients expect a detail")
	}
	if env.Error.Errors[0].Reason != "invalidQuery" {
		t.Fatalf("error.errors[0].reason = %q, want %q",
			env.Error.Errors[0].Reason, "invalidQuery")
	}
}

// TestQueryRunAcceptsLegacySQLFalse asserts the GoogleSQL-explicit case
// passes the dialect gate. The engine is not wired yet so the handler
// falls through to the 501 stub; this test pins the pre-engine behavior
// so a future engine wire-up does not regress the dialect check.
func TestQueryRunAcceptsLegacySQLFalse(t *testing.T) {
	body, err := json.Marshal(map[string]any{
		"query":        "SELECT 1",
		"useLegacySql": false,
	})
	if err != nil {
		t.Fatalf("marshal: %v", err)
	}

	rec := runQuery(t, string(body))

	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want %d (useLegacySql=false should reach engine stub)",
			rec.Code, http.StatusNotImplemented)
	}
}

// TestQueryRunAcceptsLegacySQLUnset asserts that omitting the field
// (the common case for modern clients) passes the dialect gate.
func TestQueryRunAcceptsLegacySQLUnset(t *testing.T) {
	rec := runQuery(t, `{"query":"SELECT 1"}`)

	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want %d (unset useLegacySql should reach engine stub)",
			rec.Code, http.StatusNotImplemented)
	}
}

// TestQueryRunMalformedBody asserts that a body that is not valid JSON
// is reported as a 400 with `reason: invalid` so client libraries see a
// structured error envelope instead of a 501 or 500.
func TestQueryRunMalformedBody(t *testing.T) {
	rec := httptest.NewRecorder()
	req := httptest.NewRequest(http.MethodPost,
		"/bigquery/v2/projects/test/queries", bytes.NewReader([]byte("{not json")))
	req.Header.Set("Content-Type", "application/json")
	QueryRun(Dependencies{})(rec, req)

	if rec.Code != http.StatusBadRequest {
		t.Fatalf("status = %d, want %d", rec.Code, http.StatusBadRequest)
	}
	var env errorEnvelope
	if err := json.NewDecoder(rec.Body).Decode(&env); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if env.Error.Status != "invalid" {
		t.Fatalf("error.status = %q, want %q", env.Error.Status, "invalid")
	}
}
