package handlers

import (
	"context"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"google.golang.org/grpc"
)

// runJobInsert is the symmetric helper to `runQueryWithDeps`. Issues
// a POST against `bigquery.jobs.insert` with the supplied body.
func runJobInsert(t *testing.T, deps Dependencies, body string) *httptest.ResponseRecorder {
	t.Helper()
	rec := httptest.NewRecorder()
	req := httptest.NewRequest(http.MethodPost,
		"/bigquery/v2/projects/"+testProjectID+"/jobs", strings.NewReader(body))
	req.Header.Set("Content-Type", "application/json")
	req.SetPathValue("projectId", testProjectID)
	JobInsert(deps)(rec, req)
	return rec
}

// TestJobInsertSyncQueryRegisters drives the happy path: a
// configuration.query body executes through the engine, the returned
// Job has a fresh DONE status, and the registry now contains the
// minted entry so the next `jobs.get` / `jobs.list` finds it.
func TestJobInsertSyncQueryRegisters(t *testing.T) {
	t.Parallel()
	fake := &fakeQueryClient{
		executeQueryFn: func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return twoRowExecuteStream(), nil
		},
	}
	reg := jobs.NewRegistry()
	deps := Dependencies{Query: fake, Jobs: reg}
	body := `{"configuration":{"query":{"query":"SELECT 1","useLegacySql":false}}}`

	rec := runJobInsert(t, deps, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	var got jobs.Job
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.JobReference.JobID == "" {
		t.Fatal("jobReference.jobId missing on jobs.insert response")
	}
	if got.Status.State != jobs.JobStateDone {
		t.Errorf("status.state = %q, want %q", got.Status.State, jobs.JobStateDone)
	}
	stored, ok := reg.Get(got.JobReference.JobID)
	if !ok {
		t.Fatalf("registry missing job %q after JobInsert; want it discoverable by jobs.get",
			got.JobReference.JobID)
	}
	if stored.Configuration == nil || stored.Configuration.Query == nil {
		t.Fatal("stored Job.configuration.query missing; round-trip would lose the SQL")
	}
	if stored.Configuration.Query.Query != testSQLSelectOne {
		t.Errorf("stored sql = %q, want %q",
			stored.Configuration.Query.Query, testSQLSelectOne)
	}
}

// TestJobInsertDryRunTotalBytesProcessed pins jobs.insert with
// configuration.dryRun=true: the returned Job must carry
// statistics.totalBytesProcessed as a decimal string.
func TestJobInsertDryRunTotalBytesProcessed(t *testing.T) {
	t.Parallel()
	fake := &fakeQueryClient{
		dryRunFn: func(_ context.Context, _ *enginepb.QueryRequest) (*enginepb.DryRunResponse, error) {
			return &enginepb.DryRunResponse{EstimatedBytesProcessed: 8192}, nil
		},
	}
	deps := Dependencies{Query: fake, Jobs: jobs.NewRegistry()}
	body := `{"configuration":{"dryRun":true,"query":{"query":"SELECT 1","useLegacySql":false}}}`

	rec := runJobInsert(t, deps, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	var got jobs.Job
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Status.State != jobs.JobStateDone {
		t.Errorf("status.state = %q, want %q", got.Status.State, jobs.JobStateDone)
	}
	if got.Statistics.TotalBytesProcessed != "8192" {
		t.Errorf("statistics.totalBytesProcessed = %q, want %q",
			got.Statistics.TotalBytesProcessed, "8192")
	}
}

// TestJobInsertSyncQueryCapturesEngineError pins the
// "errors-as-status" contract: an engine analysis failure must NOT
// surface as a 4xx -- it lands on `Status.ErrorResult` so the client
// can poll `jobs.get` for the failure shape the same way BigQuery
// reports it.
func TestJobInsertSyncQueryCapturesEngineError(t *testing.T) {
	t.Parallel()
	stream := &fakeQueryResultStream{tailErr: grpcUnavailable("engine unavailable")}
	fake := &fakeQueryClient{
		executeQueryFn: func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return stream, nil
		},
	}
	reg := jobs.NewRegistry()
	deps := Dependencies{Query: fake, Jobs: reg}
	body := `{"configuration":{"query":{"query":"SELECT *","useLegacySql":false}}}`

	rec := runJobInsert(t, deps, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200 (engine failures must fold into status)", rec.Code)
	}
	var got jobs.Job
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Status.State != jobs.JobStateDone {
		t.Errorf("status.state = %q, want %q (failure is still terminal)",
			got.Status.State, jobs.JobStateDone)
	}
	if got.Status.ErrorResult == nil {
		t.Fatal("status.errorResult missing; engine failure must surface on the Job")
	}
	if !strings.Contains(got.Status.ErrorResult.Message, "engine unavailable") {
		t.Errorf("error message = %q, want to contain %q",
			got.Status.ErrorResult.Message, "engine unavailable")
	}
}

// TestJobInsertWithoutEngineFallsBack501 documents the nil-Query
// behavior so unit-mode runs (`--engine_binary=""`) keep the
// structured 501 envelope rather than panicking on a nil deref.
func TestJobInsertWithoutEngineFallsBack501(t *testing.T) {
	t.Parallel()
	body := `{"configuration":{"query":{"query":"SELECT 1","useLegacySql":false}}}`
	rec := runJobInsert(t, Dependencies{}, body)
	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want 501", rec.Code)
	}
}

// grpcUnavailable mints a gRPC UNAVAILABLE error with the supplied
// message. The cancel-engine test uses it instead of plain errors.New
// so the gateway's gRPC->HTTP mapping does not need to be exercised.
func grpcUnavailable(msg string) error {
	return statusErr{code: "UNAVAILABLE", msg: msg}
}

// statusErr is a minimal `error` impl that mimics what a gRPC client
// returns. We do not need full `google.golang.org/grpc/status`
// semantics here -- the JobInsert path only reads `Error()` for the
// captured message.
type statusErr struct {
	code, msg string
}

func (s statusErr) Error() string { return s.code + ": " + s.msg }
