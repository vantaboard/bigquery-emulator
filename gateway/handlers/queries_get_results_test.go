package handlers

import (
	"context"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"google.golang.org/grpc"
)

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
	if resp.JobReference.ProjectID != testProjectID {
		t.Errorf("jobReference.projectId = %q, want %q",
			resp.JobReference.ProjectID, testProjectID)
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
	if got := resp.Rows[0].F[1].V; got != testUserAlice {
		t.Errorf("rows[0].f[1].v = %v, want %q", got, testUserAlice)
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
	req.SetPathValue("projectId", testProjectID)
	req.SetPathValue("jobId", "job_does_not_exist")
	QueryGetResults(deps)(rec, req)
	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404; body=%s", rec.Code, rec.Body.String())
	}
	var env errorEnvelope
	if err := json.NewDecoder(rec.Body).Decode(&env); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if env.Error.Status != reasonNotFound {
		t.Errorf("error.status = %q, want %q", env.Error.Status, reasonNotFound)
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
	rec := runQueryWithDeps(t, testProjectID, deps,
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
	getReq.SetPathValue("projectId", testProjectID)
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
	req.SetPathValue("projectId", testProjectID)
	req.SetPathValue("jobId", "job_anything")
	QueryGetResults(Dependencies{})(rec, req)
	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404; body=%s",
			rec.Code, rec.Body.String())
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
	rec := runQueryWithDeps(t, testProjectID, deps,
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
