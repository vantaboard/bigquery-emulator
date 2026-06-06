package handlers

import (
	"bytes"
	"context"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

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
	if env.Error.Status != reasonInvalidQuery {
		t.Fatalf("error.status = %q, want %q", env.Error.Status, reasonInvalidQuery)
	}
	if len(env.Error.Errors) == 0 {
		t.Fatal("error.errors is empty; BigQuery clients expect a detail")
	}
	if env.Error.Errors[0].Reason != reasonInvalidQuery {
		t.Fatalf("error.errors[0].reason = %q, want %q",
			env.Error.Errors[0].Reason, reasonInvalidQuery)
	}
}

// TestQueryRunAcceptsLegacySQLFalse asserts the GoogleSQL-explicit
// case passes the dialect gate. The engine is not wired in this test
// (deps.Query == nil), so the non-dry-run handler degrades to the
// structured 501 stub instead of attempting an ExecuteQuery call;
// this pins the engine-absent behavior so a future regression does
// not silently start panicking on the nil client.
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
// (the common case for modern clients) passes the dialect gate. The
// engine is not wired here either, so the same nil-Query fallback
// applies; see TestQueryRunAcceptsLegacySQLFalse.
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

// assertDryRunForwarding pins the engine RPC arguments derived from
// the inbound dryRun=true /queries body. Hoisted out of the caller
// test to keep its statement count below the funlen budget.
func assertDryRunForwarding(t *testing.T, fake *fakeQueryClient) {
	t.Helper()
	if fake.lastDryRun == nil {
		t.Fatal("Query.DryRun was not called")
	}
	if got := fake.lastDryRun.GetProjectId(); got != testProjectID {
		t.Errorf("project_id forwarded = %q, want %q", got, testProjectID)
	}
	if got := fake.lastDryRun.GetDefaultDatasetId(); got != "ds" {
		t.Errorf("default_dataset_id forwarded = %q, want %q", got, "ds")
	}
	if got := fake.lastDryRun.GetSql(); got != "SELECT id, name FROM ds.t" {
		t.Errorf("sql forwarded = %q", got)
	}
	if fake.lastDryRun.GetUseLegacySql() {
		t.Errorf("use_legacy_sql forwarded = true; want false")
	}
}

// assertDryRunEnvelope pins the QueryResponse envelope produced from
// a successful dry-run: kind, jobComplete=true, decimal-string
// totalBytesProcessed, and an empty rows page.
func assertDryRunEnvelope(t *testing.T, resp bqtypes.QueryResponse) {
	t.Helper()
	if resp.Kind != "bigquery#queryResponse" {
		t.Errorf("kind = %q, want %q", resp.Kind, "bigquery#queryResponse")
	}
	if !resp.JobComplete {
		t.Error("jobComplete = false, want true for completed dry run")
	}
	if resp.TotalBytesProcessed != "4096" {
		t.Errorf("totalBytesProcessed = %q, want %q (decimal string)",
			resp.TotalBytesProcessed, "4096")
	}
	if len(resp.Rows) != 0 {
		t.Errorf("rows = %d, want 0 for a dry run", len(resp.Rows))
	}
}

// assertDryRunSchema pins the column names + types decoded out of the
// engine's DryRunResponse schema.
func assertDryRunSchema(t *testing.T, resp bqtypes.QueryResponse) {
	t.Helper()
	if resp.Schema == nil || len(resp.Schema.Fields) != 2 {
		t.Fatalf("schema not propagated: %+v", resp.Schema)
	}
	if resp.Schema.Fields[0].Name != "id" || resp.Schema.Fields[0].Type != "INTEGER" {
		t.Errorf("schema field[0] mismatch: %+v", resp.Schema.Fields[0])
	}
	if resp.Schema.Fields[1].Name != testColumnName || resp.Schema.Fields[1].Type != sqlTypeSTRING {
		t.Errorf("schema field[1] mismatch: %+v", resp.Schema.Fields[1])
	}
}

// TestQueryRunDryRunForwardsToEngine asserts the dryRun=true branch
// forwards (project, default dataset, sql) to enginepb.Query.DryRun
// and converts the resulting schema + estimated bytes into a
// QueryResponse with `jobComplete=true` and an empty rows page. The
// per-axis assertions live in their own helpers above.
func TestQueryRunDryRunForwardsToEngine(t *testing.T) {
	fake := &fakeQueryClient{
		dryRunFn: func(_ context.Context, _ *enginepb.QueryRequest) (*enginepb.DryRunResponse, error) {
			return &enginepb.DryRunResponse{
				Schema: &enginepb.TableSchema{
					Fields: []*enginepb.FieldSchema{
						{Name: "id", Type: sqlTypeINT64, Mode: sqlModeRequired},
						{Name: testColumnName, Type: sqlTypeSTRING, Mode: sqlModeNullable},
					},
				},
				EstimatedBytesProcessed: 4096,
			}, nil
		},
	}
	deps := Dependencies{Query: fake}
	body := `{"query":"SELECT id, name FROM ds.t","dryRun":true,"useLegacySql":false,"defaultDataset":{"datasetId":"ds"}}`
	rec := runQueryWithDeps(t, testProjectID, deps, body)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}

	var resp bqtypes.QueryResponse
	if err := json.NewDecoder(rec.Body).Decode(&resp); err != nil {
		t.Fatalf("decode body: %v", err)
	}

	assertDryRunForwarding(t, fake)
	assertDryRunEnvelope(t, resp)
	assertDryRunSchema(t, resp)
}

// TestQueryRunDryRunInvalidArgumentMapsToInvalidQuery asserts a parse
// or analysis error from the engine surfaces as HTTP 400 with
// `reason: invalidQuery` (not the generic `invalid`), per
// docs/REST_API.md and BigQuery's documented reason codes.
func TestQueryRunDryRunInvalidArgumentMapsToInvalidQuery(t *testing.T) {
	fake := &fakeQueryClient{
		dryRunFn: func(_ context.Context, _ *enginepb.QueryRequest) (*enginepb.DryRunResponse, error) {
			return nil, status.Error(codes.InvalidArgument,
				"1:8: Syntax error: Unexpected keyword FROM")
		},
	}
	deps := Dependencies{Query: fake}
	body := `{"query":"SELECT FROM","dryRun":true,"useLegacySql":false}`
	rec := runQueryWithDeps(t, testProjectID, deps, body)

	if rec.Code != http.StatusBadRequest {
		t.Fatalf("status = %d, want 400; body=%s", rec.Code, rec.Body.String())
	}
	var env errorEnvelope
	if err := json.NewDecoder(rec.Body).Decode(&env); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if env.Error.Status != reasonInvalidQuery {
		t.Errorf("error.status = %q, want %q", env.Error.Status, reasonInvalidQuery)
	}
	if len(env.Error.Errors) == 0 || env.Error.Errors[0].Reason != reasonInvalidQuery {
		t.Errorf("error.errors = %+v, want [{reason:invalidQuery}]", env.Error.Errors)
	}
	if !strings.Contains(env.Error.Message, "Syntax error") {
		t.Errorf("error.message = %q, want to contain analyzer message",
			env.Error.Message)
	}
}

// TestQueryRunDryRunWithoutEngineFallsBackTo501 documents the
// nil-Query behavior so unit-mode runs (--engine_binary="") keep the
// structured 501 envelope.
func TestQueryRunDryRunWithoutEngineFallsBackTo501(t *testing.T) {
	body := `{"query":"SELECT 1","dryRun":true,"useLegacySql":false}`
	rec := runQueryWithDeps(t, testProjectID, Dependencies{}, body)

	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want %d (deps.Query==nil must degrade gracefully)",
			rec.Code, http.StatusNotImplemented)
	}
}

// TestQueryRunDryRunUnimplementedFromEngine asserts a legacy engine
// build (one without GoogleSQL linked in) surfaces UNIMPLEMENTED as
// HTTP 501 notImplemented rather than the generic invalidQuery
// mapping.
func TestQueryRunDryRunUnimplementedFromEngine(t *testing.T) {
	fake := &fakeQueryClient{
		dryRunFn: func(_ context.Context, _ *enginepb.QueryRequest) (*enginepb.DryRunResponse, error) {
			return nil, status.Error(codes.Unimplemented,
				"Query.DryRun is not implemented yet")
		},
	}
	deps := Dependencies{Query: fake}
	body := `{"query":"SELECT 1","dryRun":true,"useLegacySql":false}`
	rec := runQueryWithDeps(t, testProjectID, deps, body)

	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want 501", rec.Code)
	}
	var env errorEnvelope
	if err := json.NewDecoder(rec.Body).Decode(&env); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if env.Error.Status != reasonNotImplemented {
		t.Errorf("error.status = %q, want %q", env.Error.Status, reasonNotImplemented)
	}
}
