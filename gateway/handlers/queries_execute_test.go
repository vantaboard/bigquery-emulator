package handlers

import (
	"context"
	"encoding/json"
	"net/http"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

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
					{Name: "id", Type: sqlTypeINT64, Mode: sqlModeRequired},
					{Name: testColumnName, Type: sqlTypeSTRING, Mode: sqlModeNullable},
				},
			}},
			{Cells: []*enginepb.Cell{
				{Value: &enginepb.Cell_StringValue{StringValue: "1"}},
				{Value: &enginepb.Cell_StringValue{StringValue: testUserAlice}},
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
	rec := runQueryWithDeps(t, testProjectID, deps, body)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	if fake.lastExecuteQuery == nil {
		t.Fatal("Query.ExecuteQuery was not called")
	}
	if got := fake.lastExecuteQuery.GetProjectId(); got != testProjectID {
		t.Errorf("project_id forwarded = %q, want %q", got, testProjectID)
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
	if resp.JobReference.ProjectID != testProjectID {
		t.Errorf("jobReference.projectId = %q, want %q",
			resp.JobReference.ProjectID, testProjectID)
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
	if resp.Schema.Fields[0].Name != "id" || resp.Schema.Fields[0].Type != sqlTypeINT64 {
		t.Errorf("schema field[0] mismatch: %+v", resp.Schema.Fields[0])
	}
	if resp.Schema.Fields[1].Name != testColumnName || resp.Schema.Fields[1].Type != sqlTypeSTRING {
		t.Errorf("schema field[1] mismatch: %+v", resp.Schema.Fields[1])
	}
	if len(resp.Rows) != 2 {
		t.Fatalf("rows = %d, want 2", len(resp.Rows))
	}
	if got := resp.Rows[0].F[0].V; got != "1" {
		t.Errorf("rows[0].f[0].v = %v, want %q", got, "1")
	}
	if got := resp.Rows[0].F[1].V; got != testUserAlice {
		t.Errorf("rows[0].f[1].v = %v, want %q", got, testUserAlice)
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
					{Name: "n", Type: sqlTypeINT64, Mode: sqlModeRequired},
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
	rec := runQueryWithDeps(t, testProjectID, deps, `{"query":"SELECT n FROM t","useLegacySql":false}`)

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
	if ref["projectId"] != testProjectID {
		t.Errorf("jobReference.projectId = %v, want %q", ref["projectId"], testProjectID)
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
				Fields: []*enginepb.FieldSchema{{Name: "v", Type: sqlTypeINT64}},
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
	rec := runQueryWithDeps(t, testProjectID, deps, `{"query":"SELECT 42","useLegacySql":false}`)
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
	if stored.JobReference.ProjectID != testProjectID {
		t.Errorf("stored job project = %q, want %q", stored.JobReference.ProjectID, testProjectID)
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
	rec := runQueryWithDeps(t, testProjectID, deps,
		`{"query":"SELECT 1","useLegacySql":false}`)
	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want %d; body=%s",
			rec.Code, http.StatusNotImplemented, rec.Body.String())
	}
}
