package handlers

import (
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
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

// runQueryAndGetResults issues a `jobs.query` POST and the subsequent
// `jobs.getQueryResults` GET against the same registry, returning the
// GetQueryResultsResponse shape directly. Both calls share the same
// Dependencies (and therefore the same Registry) so the registered
// job is reachable on the read.
func runQueryAndGetResults(t *testing.T, deps Dependencies, queryBody, query string) *bqtypes.QueryResponse {
	t.Helper()
	rec := runQueryWithDeps(t, testProjectID, deps, queryBody)
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
	getReq.SetPathValue("projectId", testProjectID)
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
				{Value: &enginepb.Cell_StringValue{StringValue: "bob"}},
			}},
		},
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
