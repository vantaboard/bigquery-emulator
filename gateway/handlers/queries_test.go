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
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

// runQuery posts the given JSON body to QueryRun and returns the
// recorder for inspection.
func runQuery(t *testing.T, body string) *httptest.ResponseRecorder {
	t.Helper()
	return runQueryWithDeps(t, "test", Dependencies{}, body)
}

// runQueryWithDeps posts to QueryRun with explicit Dependencies and
// PathValue("projectId") populated the way the mux does at runtime.
func runQueryWithDeps(t *testing.T, projectID string, deps Dependencies, body string) *httptest.ResponseRecorder {
	t.Helper()
	rec := httptest.NewRecorder()
	req := httptest.NewRequest(http.MethodPost,
		"/bigquery/v2/projects/"+projectID+"/queries", strings.NewReader(body))
	req.Header.Set("Content-Type", "application/json")
	req.SetPathValue("projectId", projectID)
	QueryRun(deps)(rec, req)
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

// TestQueryRunDryRunForwardsToEngine asserts the dryRun=true branch
// forwards (project, default dataset, sql) to enginepb.Query.DryRun
// and converts the resulting schema + estimated bytes into a
// QueryResponse with `jobComplete=true` and an empty rows page.
func TestQueryRunDryRunForwardsToEngine(t *testing.T) {
	fake := &fakeQueryClient{
		dryRunFn: func(_ context.Context, _ *enginepb.QueryRequest) (*enginepb.DryRunResponse, error) {
			return &enginepb.DryRunResponse{
				Schema: &enginepb.TableSchema{
					Fields: []*enginepb.FieldSchema{
						{Name: "id", Type: "INT64", Mode: "REQUIRED"},
						{Name: "name", Type: "STRING", Mode: "NULLABLE"},
					},
				},
				EstimatedBytesProcessed: 4096,
			}, nil
		},
	}
	deps := Dependencies{Query: fake}
	body := `{"query":"SELECT id, name FROM ds.t","dryRun":true,"useLegacySql":false,"defaultDataset":{"datasetId":"ds"}}`
	rec := runQueryWithDeps(t, "proj", deps, body)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	if fake.lastDryRun == nil {
		t.Fatal("Query.DryRun was not called")
	}
	if got := fake.lastDryRun.GetProjectId(); got != "proj" {
		t.Errorf("project_id forwarded = %q, want %q", got, "proj")
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

	var resp bqtypes.QueryResponse
	if err := json.NewDecoder(rec.Body).Decode(&resp); err != nil {
		t.Fatalf("decode body: %v", err)
	}
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
	if resp.Schema == nil || len(resp.Schema.Fields) != 2 {
		t.Fatalf("schema not propagated: %+v", resp.Schema)
	}
	if resp.Schema.Fields[0].Name != "id" || resp.Schema.Fields[0].Type != "INT64" {
		t.Errorf("schema field[0] mismatch: %+v", resp.Schema.Fields[0])
	}
	if resp.Schema.Fields[1].Name != "name" || resp.Schema.Fields[1].Type != "STRING" {
		t.Errorf("schema field[1] mismatch: %+v", resp.Schema.Fields[1])
	}
	if len(resp.Rows) != 0 {
		t.Errorf("rows = %d, want 0 for a dry run", len(resp.Rows))
	}
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
	rec := runQueryWithDeps(t, "proj", deps, body)

	if rec.Code != http.StatusBadRequest {
		t.Fatalf("status = %d, want 400; body=%s", rec.Code, rec.Body.String())
	}
	var env errorEnvelope
	if err := json.NewDecoder(rec.Body).Decode(&env); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if env.Error.Status != "invalidQuery" {
		t.Errorf("error.status = %q, want %q", env.Error.Status, "invalidQuery")
	}
	if len(env.Error.Errors) == 0 || env.Error.Errors[0].Reason != "invalidQuery" {
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
	rec := runQueryWithDeps(t, "proj", Dependencies{}, body)

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
				"Query.DryRun is not implemented yet (Phase 4 of ROADMAP.md)")
		},
	}
	deps := Dependencies{Query: fake}
	body := `{"query":"SELECT 1","dryRun":true,"useLegacySql":false}`
	rec := runQueryWithDeps(t, "proj", deps, body)

	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want 501", rec.Code)
	}
	var env errorEnvelope
	if err := json.NewDecoder(rec.Body).Decode(&env); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if env.Error.Status != "notImplemented" {
		t.Errorf("error.status = %q, want %q", env.Error.Status, "notImplemented")
	}
}

// TestQueryRunExecuteAssemblesResponse drives the non-dry-run path
// end-to-end against a fake stream that returns a schema message
// followed by two row messages, mirroring the proto contract on
// QueryResultRow. The assembled QueryResponse must carry the
// BigQuery-shaped kind, jobReference (minted by deps.Jobs), the
// schema, two `f`/`v` rows (with NULLs as JSON null), totalRows
// matching the row count, and jobComplete=true.
func TestQueryRunExecuteAssemblesResponse(t *testing.T) {
	stream := &fakeQueryResultStream{
		msgs: []*enginepb.QueryResultRow{
			{Schema: &enginepb.TableSchema{
				Fields: []*enginepb.FieldSchema{
					{Name: "id", Type: "INT64", Mode: "REQUIRED"},
					{Name: "name", Type: "STRING", Mode: "NULLABLE"},
				},
			}},
			{Cells: []*enginepb.Cell{
				{Value: &enginepb.Cell_StringValue{StringValue: "1"}},
				{Value: &enginepb.Cell_StringValue{StringValue: "alice"}},
			}},
			{Cells: []*enginepb.Cell{
				{Value: &enginepb.Cell_StringValue{StringValue: "2"}},
				{Value: &enginepb.Cell_NullValue{NullValue: true}},
			}},
		},
	}
	fake := &fakeQueryClient{
		executeQueryFn: func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return stream, nil
		},
	}
	deps := Dependencies{Query: fake}
	body := `{"query":"SELECT id, name FROM ds.t","useLegacySql":false,"defaultDataset":{"datasetId":"ds"},"location":"US"}`
	rec := runQueryWithDeps(t, "proj", deps, body)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	if fake.lastExecuteQuery == nil {
		t.Fatal("Query.ExecuteQuery was not called")
	}
	if got := fake.lastExecuteQuery.GetProjectId(); got != "proj" {
		t.Errorf("project_id forwarded = %q, want %q", got, "proj")
	}
	if got := fake.lastExecuteQuery.GetDefaultDatasetId(); got != "ds" {
		t.Errorf("default_dataset_id forwarded = %q, want %q", got, "ds")
	}
	if got := fake.lastExecuteQuery.GetSql(); got != "SELECT id, name FROM ds.t" {
		t.Errorf("sql forwarded = %q", got)
	}
	if fake.lastExecuteQuery.GetUseLegacySql() {
		t.Errorf("use_legacy_sql forwarded = true; want false")
	}

	var resp bqtypes.QueryResponse
	if err := json.NewDecoder(rec.Body).Decode(&resp); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if resp.Kind != queryResponseKind {
		t.Errorf("kind = %q, want %q", resp.Kind, queryResponseKind)
	}
	if !resp.JobComplete {
		t.Error("jobComplete = false, want true")
	}
	if resp.TotalRows != "2" {
		t.Errorf("totalRows = %q, want %q", resp.TotalRows, "2")
	}
	if resp.JobReference == nil {
		t.Fatal("jobReference is nil; jobs.query must always emit one")
	}
	if resp.JobReference.ProjectID != "proj" {
		t.Errorf("jobReference.projectId = %q, want %q",
			resp.JobReference.ProjectID, "proj")
	}
	if !strings.HasPrefix(resp.JobReference.JobID, "job_") {
		t.Errorf("jobReference.jobId = %q, want a job_-prefixed id",
			resp.JobReference.JobID)
	}
	if resp.JobReference.Location != "US" {
		t.Errorf("jobReference.location = %q, want %q",
			resp.JobReference.Location, "US")
	}
	if resp.Schema == nil || len(resp.Schema.Fields) != 2 {
		t.Fatalf("schema not propagated: %+v", resp.Schema)
	}
	if resp.Schema.Fields[0].Name != "id" || resp.Schema.Fields[0].Type != "INT64" {
		t.Errorf("schema field[0] mismatch: %+v", resp.Schema.Fields[0])
	}
	if resp.Schema.Fields[1].Name != "name" || resp.Schema.Fields[1].Type != "STRING" {
		t.Errorf("schema field[1] mismatch: %+v", resp.Schema.Fields[1])
	}
	if len(resp.Rows) != 2 {
		t.Fatalf("rows = %d, want 2", len(resp.Rows))
	}
	if got := resp.Rows[0].F[0].V; got != "1" {
		t.Errorf("rows[0].f[0].v = %v, want %q", got, "1")
	}
	if got := resp.Rows[0].F[1].V; got != "alice" {
		t.Errorf("rows[0].f[1].v = %v, want %q", got, "alice")
	}
	if got := resp.Rows[1].F[0].V; got != "2" {
		t.Errorf("rows[1].f[0].v = %v, want %q", got, "2")
	}
	if resp.Rows[1].F[1].V != nil {
		t.Errorf("rows[1].f[1].v = %v, want nil (NULL)", resp.Rows[1].F[1].V)
	}
	if resp.TotalBytesProcessed != "0" {
		t.Errorf("totalBytesProcessed = %q, want %q (engine bytes not wired yet)",
			resp.TotalBytesProcessed, "0")
	}
}

// TestQueryRunExecuteJSONShape pins the on-the-wire JSON the BigQuery
// client libraries decode: a single `rows[*].f[*].v` array per row,
// `jobReference` with projectId + jobId, and a kind matching the
// QueryResponse resource. Catching shape regressions here is cheaper
// than re-running the e2e Go client test.
func TestQueryRunExecuteJSONShape(t *testing.T) {
	stream := &fakeQueryResultStream{
		msgs: []*enginepb.QueryResultRow{
			{Schema: &enginepb.TableSchema{
				Fields: []*enginepb.FieldSchema{
					{Name: "n", Type: "INT64", Mode: "REQUIRED"},
				},
			}},
			{Cells: []*enginepb.Cell{
				{Value: &enginepb.Cell_StringValue{StringValue: "1"}},
			}},
			{Cells: []*enginepb.Cell{
				{Value: &enginepb.Cell_StringValue{StringValue: "2"}},
			}},
		},
	}
	fake := &fakeQueryClient{
		executeQueryFn: func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return stream, nil
		},
	}
	deps := Dependencies{Query: fake}
	rec := runQueryWithDeps(t, "proj", deps, `{"query":"SELECT n FROM t","useLegacySql":false}`)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}

	var out map[string]any
	if err := json.Unmarshal(rec.Body.Bytes(), &out); err != nil {
		t.Fatalf("unmarshal: %v", err)
	}
	if got := out["kind"]; got != queryResponseKind {
		t.Errorf("kind = %v, want %q", got, queryResponseKind)
	}
	if got := out["jobComplete"]; got != true {
		t.Errorf("jobComplete = %v, want true", got)
	}
	ref, _ := out["jobReference"].(map[string]any)
	if ref == nil {
		t.Fatalf("jobReference missing or wrong type: %#v", out["jobReference"])
	}
	if ref["projectId"] != "proj" {
		t.Errorf("jobReference.projectId = %v, want %q", ref["projectId"], "proj")
	}
	if _, ok := ref["jobId"].(string); !ok {
		t.Errorf("jobReference.jobId must be a string, got %T", ref["jobId"])
	}
	rows, _ := out["rows"].([]any)
	if len(rows) != 2 {
		t.Fatalf("rows = %d, want 2", len(rows))
	}
	first, _ := rows[0].(map[string]any)
	cells, _ := first["f"].([]any)
	if len(cells) != 1 {
		t.Fatalf("rows[0].f = %d cells, want 1", len(cells))
	}
	cell, _ := cells[0].(map[string]any)
	if cell["v"] != "1" {
		t.Errorf("rows[0].f[0].v = %v, want %q", cell["v"], "1")
	}
}

// TestQueryRunExecuteRegistersJob asserts that the jobReference in
// the response is the same Job stored in deps.Jobs -- a regression
// here would silently break the upcoming jobs.get handler.
func TestQueryRunExecuteRegistersJob(t *testing.T) {
	stream := &fakeQueryResultStream{
		msgs: []*enginepb.QueryResultRow{
			{Schema: &enginepb.TableSchema{
				Fields: []*enginepb.FieldSchema{{Name: "v", Type: "INT64"}},
			}},
			{Cells: []*enginepb.Cell{
				{Value: &enginepb.Cell_StringValue{StringValue: "42"}},
			}},
		},
	}
	fake := &fakeQueryClient{
		executeQueryFn: func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return stream, nil
		},
	}
	registry := jobs.NewRegistry()
	deps := Dependencies{Query: fake, Jobs: registry}
	rec := runQueryWithDeps(t, "proj", deps, `{"query":"SELECT 42","useLegacySql":false}`)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}

	var resp bqtypes.QueryResponse
	if err := json.NewDecoder(rec.Body).Decode(&resp); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if resp.JobReference == nil {
		t.Fatal("jobReference missing from response")
	}
	stored, ok := registry.Get(resp.JobReference.JobID)
	if !ok {
		t.Fatalf("registry has no job for %q", resp.JobReference.JobID)
	}
	if stored.Status.State != "DONE" {
		t.Errorf("stored job state = %q, want %q", stored.Status.State, "DONE")
	}
	if stored.JobReference.ProjectID != "proj" {
		t.Errorf("stored job project = %q, want %q", stored.JobReference.ProjectID, "proj")
	}
}

// TestQueryRunExecuteUnimplementedFromEngine asserts UNIMPLEMENTED
// from the engine (`--googlesql=off` build, etc.) surfaces as HTTP
// 501 notImplemented rather than the generic 400 invalidQuery
// mapping the parse/analysis branch uses.
func TestQueryRunExecuteUnimplementedFromEngine(t *testing.T) {
	fake := &fakeQueryClient{
		executeQueryFn: func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return nil, status.Error(codes.Unimplemented,
				"Query.ExecuteQuery is not implemented in this build")
		},
	}
	deps := Dependencies{Query: fake}
	rec := runQueryWithDeps(t, "proj", deps,
		`{"query":"SELECT 1","useLegacySql":false}`)
	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want %d; body=%s",
			rec.Code, http.StatusNotImplemented, rec.Body.String())
	}
}
