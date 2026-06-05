package handlers

import (
	"context"
	"encoding/json"
	"net/http"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"google.golang.org/grpc"
)

func sessionTestQueryClient(t *testing.T) *fakeQueryClient {
	t.Helper()
	return &fakeQueryClient{
		executeQueryFn: func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return twoRowExecuteStream(), nil
		},
	}
}

func TestJobInsertCreateSessionSetsStatisticsSessionInfo(t *testing.T) {
	t.Parallel()
	deps := Dependencies{
		Jobs:     jobs.NewRegistry(),
		Sessions: NewSessionStore(),
		Query:    sessionTestQueryClient(t),
	}
	body := `{"configuration":{"query":{"query":"SELECT 1","useLegacySql":false,"createSession":true}}}`
	rec := runJobInsert(t, deps, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("JobInsert -> %d: %s", rec.Code, rec.Body.String())
	}
	var job jobs.Job
	if err := json.Unmarshal(rec.Body.Bytes(), &job); err != nil {
		t.Fatalf("decode job: %v", err)
	}
	if job.Statistics.SessionInfo == nil || job.Statistics.SessionInfo.SessionID == "" {
		t.Fatalf("statistics.sessionInfo missing: %+v", job.Statistics)
	}
}

func TestQueryRunCreateSessionSetsStatisticsSessionInfo(t *testing.T) {
	t.Parallel()
	deps := Dependencies{
		Jobs:     jobs.NewRegistry(),
		Sessions: NewSessionStore(),
		Query:    sessionTestQueryClient(t),
	}
	rec := runQueryWithDeps(t, "test", deps, `{
		"query": "SELECT 1",
		"useLegacySql": false,
		"createSession": true
	}`)
	if rec.Code != http.StatusOK {
		t.Fatalf("QueryRun -> %d: %s", rec.Code, rec.Body.String())
	}
	var out bqtypes.QueryResponse
	if err := json.Unmarshal(rec.Body.Bytes(), &out); err != nil {
		t.Fatalf("decode response: %v", err)
	}
	if out.Statistics == nil || out.Statistics.SessionInfo == nil ||
		out.Statistics.SessionInfo.SessionID == "" {
		t.Fatalf("statistics.sessionInfo missing: %+v", out.Statistics)
	}
}

func TestQueryRunConnectionPropertiesReusesSessionID(t *testing.T) {
	t.Parallel()
	deps := Dependencies{
		Jobs:     jobs.NewRegistry(),
		Sessions: NewSessionStore(),
		Query:    sessionTestQueryClient(t),
	}
	createRec := runQueryWithDeps(t, "test", deps, `{
		"query": "SELECT 1",
		"useLegacySql": false,
		"createSession": true
	}`)
	var created bqtypes.QueryResponse
	if err := json.Unmarshal(createRec.Body.Bytes(), &created); err != nil {
		t.Fatalf("decode create response: %v", err)
	}
	if created.Statistics == nil || created.Statistics.SessionInfo == nil {
		t.Fatal("expected sessionInfo on createSession query")
	}
	sessionID := created.Statistics.SessionInfo.SessionID

	followRec := runQueryWithDeps(t, "test", deps, `{
		"query": "SELECT 2",
		"useLegacySql": false,
		"connectionProperties": [
			{"key": "session_id", "value": "`+sessionID+`"}
		]
	}`)
	if followRec.Code != http.StatusOK {
		t.Fatalf("follow-up QueryRun -> %d: %s", followRec.Code, followRec.Body.String())
	}
	var follow bqtypes.QueryResponse
	if err := json.Unmarshal(followRec.Body.Bytes(), &follow); err != nil {
		t.Fatalf("decode follow response: %v", err)
	}
	if follow.Statistics == nil || follow.Statistics.SessionInfo == nil {
		t.Fatalf("expected sessionInfo on follow-up query: %+v", follow.Statistics)
	}
	if follow.Statistics.SessionInfo.SessionID != sessionID {
		t.Fatalf("sessionId = %q, want %q", follow.Statistics.SessionInfo.SessionID, sessionID)
	}
}

func TestQueryRunAbortSession(t *testing.T) {
	t.Parallel()
	deps := Dependencies{
		Jobs:     jobs.NewRegistry(),
		Sessions: NewSessionStore(),
		Query:    sessionTestQueryClient(t),
	}
	rec := runQueryWithDeps(t, "test", deps, `{
		"query": "CALL BQ.ABORT_SESSION('abc123')",
		"useLegacySql": false
	}`)
	if rec.Code != http.StatusOK {
		t.Fatalf("QueryRun ABORT_SESSION -> %d: %s", rec.Code, rec.Body.String())
	}
	var out bqtypes.QueryResponse
	if err := json.Unmarshal(rec.Body.Bytes(), &out); err != nil {
		t.Fatalf("decode response: %v", err)
	}
	if !out.JobComplete {
		t.Fatal("expected jobComplete=true for ABORT_SESSION stub")
	}
}
