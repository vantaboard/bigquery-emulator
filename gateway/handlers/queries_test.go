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
				"Query.DryRun is not implemented yet")
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

// runQueryAndGetResults submits a query through QueryRun, then calls
// QueryGetResults on the returned jobReference. The result-side
// recorder is returned to the caller so the test can inspect the
// GetQueryResultsResponse shape directly. Both calls share the same
// Dependencies (and therefore the same Registry) so the registered
// job is reachable on the read.
func runQueryAndGetResults(t *testing.T, deps Dependencies, queryBody, query string) *bqtypes.QueryResponse {
	t.Helper()
	rec := runQueryWithDeps(t, "proj", deps, queryBody)
	if rec.Code != http.StatusOK {
		t.Fatalf("QueryRun status = %d, want 200; body=%s",
			rec.Code, rec.Body.String())
	}
	var run bqtypes.QueryResponse
	if err := json.NewDecoder(rec.Body).Decode(&run); err != nil {
		t.Fatalf("decode run body: %v", err)
	}
	if run.JobReference == nil {
		t.Fatalf("QueryRun did not emit a jobReference; body=%s", rec.Body.String())
	}

	getRec := httptest.NewRecorder()
	getReq := httptest.NewRequest(http.MethodGet,
		"/bigquery/v2/projects/proj/queries/"+run.JobReference.JobID+query, nil)
	getReq.SetPathValue("projectId", "proj")
	getReq.SetPathValue("jobId", run.JobReference.JobID)
	QueryGetResults(deps)(getRec, getReq)
	if getRec.Code != http.StatusOK {
		t.Fatalf("QueryGetResults status = %d, want 200; body=%s",
			getRec.Code, getRec.Body.String())
	}
	var get bqtypes.QueryResponse
	if err := json.NewDecoder(getRec.Body).Decode(&get); err != nil {
		t.Fatalf("decode getResults body: %v", err)
	}
	return &get
}

// twoRowExecuteStream is the standard fixture used by the
// QueryGetResults tests: a schema message followed by two row
// messages, mirroring the proto contract on QueryResultRow.
func twoRowExecuteStream() *fakeQueryResultStream {
	return &fakeQueryResultStream{
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
				{Value: &enginepb.Cell_StringValue{StringValue: "bob"}},
			}},
		},
	}
}

// TestQueryGetResultsReturnsCachedRows asserts the happy path:
// jobs.query stores schema+rows on the registry, and a follow-up
// jobs.getQueryResults replays them with a getQueryResultsResponse-
// shaped body (kind, jobReference, schema, totalRows, jobComplete,
// rows). This pins the "single-page replay" contract.
func TestQueryGetResultsReturnsCachedRows(t *testing.T) {
	fake := &fakeQueryClient{
		executeQueryFn: func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return twoRowExecuteStream(), nil
		},
	}
	deps := Dependencies{Query: fake, Jobs: jobs.NewRegistry()}
	body := `{"query":"SELECT id, name FROM ds.t","useLegacySql":false,"location":"US"}`
	resp := runQueryAndGetResults(t, deps, body, "")

	if resp.Kind != getQueryResultsKind {
		t.Errorf("kind = %q, want %q", resp.Kind, getQueryResultsKind)
	}
	if !resp.JobComplete {
		t.Error("jobComplete = false, want true")
	}
	if resp.TotalRows != "2" {
		t.Errorf("totalRows = %q, want %q", resp.TotalRows, "2")
	}
	if resp.JobReference == nil {
		t.Fatal("jobReference missing on getResults response")
	}
	if resp.JobReference.ProjectID != "proj" {
		t.Errorf("jobReference.projectId = %q, want %q",
			resp.JobReference.ProjectID, "proj")
	}
	if resp.JobReference.Location != "US" {
		t.Errorf("jobReference.location = %q, want %q",
			resp.JobReference.Location, "US")
	}
	if resp.Schema == nil || len(resp.Schema.Fields) != 2 {
		t.Fatalf("schema = %+v, want 2 fields", resp.Schema)
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
	if got := resp.Rows[1].F[1].V; got != "bob" {
		t.Errorf("rows[1].f[1].v = %v, want %q", got, "bob")
	}
}

// TestQueryGetResultsRespectsStartIndexAndMaxResults pins the
// pagination knobs: startIndex skips the leading row, maxResults
// caps the trailing one. totalRows still reports the full count
// because that field describes the underlying result set, not the
// emitted page.
func TestQueryGetResultsRespectsStartIndexAndMaxResults(t *testing.T) {
	fake := &fakeQueryClient{
		executeQueryFn: func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return twoRowExecuteStream(), nil
		},
	}
	deps := Dependencies{Query: fake, Jobs: jobs.NewRegistry()}
	body := `{"query":"SELECT id, name FROM ds.t","useLegacySql":false}`

	got := runQueryAndGetResults(t, deps, body, "?startIndex=1&maxResults=10")
	if got.TotalRows != "2" {
		t.Errorf("totalRows = %q, want %q", got.TotalRows, "2")
	}
	if len(got.Rows) != 1 {
		t.Fatalf("rows = %d, want 1 (after startIndex=1)", len(got.Rows))
	}
	if v := got.Rows[0].F[0].V; v != "2" {
		t.Errorf("rows[0].f[0].v = %v, want %q", v, "2")
	}

	got = runQueryAndGetResults(t, deps, body, "?maxResults=1")
	if len(got.Rows) != 1 {
		t.Fatalf("rows = %d, want 1 (after maxResults=1)", len(got.Rows))
	}
	if v := got.Rows[0].F[0].V; v != "1" {
		t.Errorf("rows[0].f[0].v = %v, want %q", v, "1")
	}

	got = runQueryAndGetResults(t, deps, body, "?startIndex=99")
	if len(got.Rows) != 0 {
		t.Errorf("rows = %d, want 0 (startIndex past end)", len(got.Rows))
	}
}

// TestQueryGetResultsPageTokenReturnsEmptyPage documents the
// single-page-only behavior: the emulator never mints pageTokens, so
// any non-empty token yields an empty terminal page. Polling clients
// see jobComplete=true and exit cleanly instead of looping forever.
func TestQueryGetResultsPageTokenReturnsEmptyPage(t *testing.T) {
	fake := &fakeQueryClient{
		executeQueryFn: func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return twoRowExecuteStream(), nil
		},
	}
	deps := Dependencies{Query: fake, Jobs: jobs.NewRegistry()}
	body := `{"query":"SELECT id, name FROM ds.t","useLegacySql":false}`

	got := runQueryAndGetResults(t, deps, body, "?pageToken=stale")
	if !got.JobComplete {
		t.Error("jobComplete = false, want true even with a stale pageToken")
	}
	if got.TotalRows != "2" {
		t.Errorf("totalRows = %q, want %q (totalRows reflects the full set)",
			got.TotalRows, "2")
	}
	if len(got.Rows) != 0 {
		t.Errorf("rows = %d, want 0 (single-page emulator never mints a token)",
			len(got.Rows))
	}
	if got.PageToken != "" {
		t.Errorf("pageToken = %q, want empty (single-page replay never mints one)",
			got.PageToken)
	}
}

// dmlStatsExecuteStream is the standard DML fixture: no schema, no
// row cells, exactly one trailing `dml_stats` message carrying the
// engine's per-operation counts. Mirrors the proto contract on
// `QueryResultRow.dml_stats`.
func dmlStatsExecuteStream(inserted, updated, deleted int64) *fakeQueryResultStream {
	return &fakeQueryResultStream{
		msgs: []*enginepb.QueryResultRow{
			{DmlStats: &enginepb.DmlStats{
				InsertedRowCount: inserted,
				UpdatedRowCount:  updated,
				DeletedRowCount:  deleted,
			}},
		},
	}
}

// TestQueryRunDmlStatsResponseShape pins the synchronous `jobs.query`
// envelope for a DML statement: `dmlStats` + `numDmlAffectedRows` are
// populated from the engine's trailing `dml_stats` message and the
// SELECT-shape fields (`schema`, `rows`, `totalRows`) are blanked out
// so the response wire matches BigQuery's documented DML shape.
func TestQueryRunDmlStatsResponseShape(t *testing.T) {
	fake := &fakeQueryClient{
		executeQueryFn: func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return dmlStatsExecuteStream(3, 1, 2), nil
		},
	}
	deps := Dependencies{Query: fake, Jobs: jobs.NewRegistry()}
	rec := runQueryWithDeps(t, "proj", deps,
		`{"query":"DELETE FROM ds.t WHERE id < 10","useLegacySql":false}`)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s",
			rec.Code, rec.Body.String())
	}
	var resp bqtypes.QueryResponse
	if err := json.NewDecoder(rec.Body).Decode(&resp); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if resp.DmlStats == nil {
		t.Fatalf("dmlStats missing on DML response; body=%s", rec.Body.String())
	}
	if resp.DmlStats.InsertedRowCount != "3" {
		t.Errorf("dmlStats.insertedRowCount = %q, want %q",
			resp.DmlStats.InsertedRowCount, "3")
	}
	if resp.DmlStats.UpdatedRowCount != "1" {
		t.Errorf("dmlStats.updatedRowCount = %q, want %q",
			resp.DmlStats.UpdatedRowCount, "1")
	}
	if resp.DmlStats.DeletedRowCount != "2" {
		t.Errorf("dmlStats.deletedRowCount = %q, want %q",
			resp.DmlStats.DeletedRowCount, "2")
	}
	if resp.NumDmlAffectedRows != "6" {
		t.Errorf("numDmlAffectedRows = %q, want %q (3+1+2)",
			resp.NumDmlAffectedRows, "6")
	}
	if resp.Schema != nil {
		t.Errorf("schema = %+v, want nil (DML has no result schema)",
			resp.Schema)
	}
	if len(resp.Rows) != 0 {
		t.Errorf("rows = %d, want 0 (DML has no rows)", len(resp.Rows))
	}
	if resp.TotalRows != "0" {
		t.Errorf("totalRows = %q, want %q", resp.TotalRows, "0")
	}
}

// TestQueryGetResultsReplaysDmlStats pins the contract that
// `jobs.getQueryResults` re-emits the same `dmlStats` /
// `numDmlAffectedRows` envelope that `jobs.query` returned at submit
// time. Polling BigQuery clients (e.g. the Go SDK's `JobIterator`)
// read row counts from the replay endpoint, not just from the
// synchronous submit response, so the registry has to cache the
// stats alongside schema + rows.
func TestQueryGetResultsReplaysDmlStats(t *testing.T) {
	fake := &fakeQueryClient{
		executeQueryFn: func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return dmlStatsExecuteStream(5, 0, 0), nil
		},
	}
	deps := Dependencies{Query: fake, Jobs: jobs.NewRegistry()}
	body := `{"query":"INSERT INTO ds.t (id) VALUES (1),(2),(3),(4),(5)","useLegacySql":false}`
	resp := runQueryAndGetResults(t, deps, body, "")

	if resp.Kind != getQueryResultsKind {
		t.Errorf("kind = %q, want %q", resp.Kind, getQueryResultsKind)
	}
	if resp.DmlStats == nil {
		t.Fatalf("dmlStats missing on getResults replay")
	}
	if resp.DmlStats.InsertedRowCount != "5" {
		t.Errorf("dmlStats.insertedRowCount = %q, want %q",
			resp.DmlStats.InsertedRowCount, "5")
	}
	if resp.NumDmlAffectedRows != "5" {
		t.Errorf("numDmlAffectedRows = %q, want %q",
			resp.NumDmlAffectedRows, "5")
	}
	if resp.Schema != nil {
		t.Errorf("schema = %+v, want nil on DML replay", resp.Schema)
	}
	if len(resp.Rows) != 0 {
		t.Errorf("rows = %d, want 0 on DML replay", len(resp.Rows))
	}
	if resp.TotalRows != "0" {
		t.Errorf("totalRows = %q, want %q on DML replay",
			resp.TotalRows, "0")
	}
}

// TestQueryGetResultsUnknownJobReturns404 asserts the unknown-jobId
// branch surfaces as a BigQuery-shaped 404 envelope.
func TestQueryGetResultsUnknownJobReturns404(t *testing.T) {
	deps := Dependencies{Jobs: jobs.NewRegistry()}
	rec := httptest.NewRecorder()
	req := httptest.NewRequest(http.MethodGet,
		"/bigquery/v2/projects/proj/queries/job_does_not_exist", nil)
	req.SetPathValue("projectId", "proj")
	req.SetPathValue("jobId", "job_does_not_exist")
	QueryGetResults(deps)(rec, req)
	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404; body=%s", rec.Code, rec.Body.String())
	}
	var env errorEnvelope
	if err := json.NewDecoder(rec.Body).Decode(&env); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if env.Error.Status != "notFound" {
		t.Errorf("error.status = %q, want %q", env.Error.Status, "notFound")
	}
}

// TestQueryGetResultsProjectMismatchReturns404 asserts that a job
// minted under one project is not visible from another project. The
// emulator uses 404 (not 403) to match BigQuery's wire shape: cross-
// project jobs are hidden behind the same envelope as missing ones.
func TestQueryGetResultsProjectMismatchReturns404(t *testing.T) {
	fake := &fakeQueryClient{
		executeQueryFn: func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return twoRowExecuteStream(), nil
		},
	}
	deps := Dependencies{Query: fake, Jobs: jobs.NewRegistry()}
	rec := runQueryWithDeps(t, "proj-a", deps,
		`{"query":"SELECT 1","useLegacySql":false}`)
	if rec.Code != http.StatusOK {
		t.Fatalf("setup QueryRun status = %d, want 200; body=%s",
			rec.Code, rec.Body.String())
	}
	var run bqtypes.QueryResponse
	if err := json.NewDecoder(rec.Body).Decode(&run); err != nil {
		t.Fatalf("decode run body: %v", err)
	}

	getRec := httptest.NewRecorder()
	getReq := httptest.NewRequest(http.MethodGet,
		"/bigquery/v2/projects/proj-b/queries/"+run.JobReference.JobID, nil)
	getReq.SetPathValue("projectId", "proj-b")
	getReq.SetPathValue("jobId", run.JobReference.JobID)
	QueryGetResults(deps)(getRec, getReq)
	if getRec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404; body=%s",
			getRec.Code, getRec.Body.String())
	}
}

// TestQueryGetResultsLocationMismatchReturns404 asserts that a
// location query parameter that disagrees with the stored job's
// location surfaces as 404 notFound, mirroring BigQuery's behavior
// when a client routes a getQueryResults to the wrong region.
func TestQueryGetResultsLocationMismatchReturns404(t *testing.T) {
	fake := &fakeQueryClient{
		executeQueryFn: func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return twoRowExecuteStream(), nil
		},
	}
	deps := Dependencies{Query: fake, Jobs: jobs.NewRegistry()}
	rec := runQueryWithDeps(t, "proj", deps,
		`{"query":"SELECT 1","useLegacySql":false,"location":"US"}`)
	if rec.Code != http.StatusOK {
		t.Fatalf("setup QueryRun status = %d, want 200; body=%s",
			rec.Code, rec.Body.String())
	}
	var run bqtypes.QueryResponse
	if err := json.NewDecoder(rec.Body).Decode(&run); err != nil {
		t.Fatalf("decode run body: %v", err)
	}

	getRec := httptest.NewRecorder()
	getReq := httptest.NewRequest(http.MethodGet,
		"/bigquery/v2/projects/proj/queries/"+run.JobReference.JobID+"?location=EU", nil)
	getReq.SetPathValue("projectId", "proj")
	getReq.SetPathValue("jobId", run.JobReference.JobID)
	QueryGetResults(deps)(getRec, getReq)
	if getRec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404; body=%s",
			getRec.Code, getRec.Body.String())
	}
}

// TestQueryGetResultsNilJobsLazyInit asserts the handler builds a
// fallback Registry when `deps.Jobs == nil` (legacy unit-mode wiring)
// rather than panicking on a nil deref. With no jobs registered the
// only reachable branch is "unknown jobId", which must still surface
// as a 404.
func TestQueryGetResultsNilJobsLazyInit(t *testing.T) {
	rec := httptest.NewRecorder()
	req := httptest.NewRequest(http.MethodGet,
		"/bigquery/v2/projects/proj/queries/job_anything", nil)
	req.SetPathValue("projectId", "proj")
	req.SetPathValue("jobId", "job_anything")
	QueryGetResults(Dependencies{})(rec, req)
	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404; body=%s",
			rec.Code, rec.Body.String())
	}
}
