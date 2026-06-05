//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestQuerySelectOneRoundTrip is the end-to-end story for trivial
// constant queries: a `SELECT 1` against a real running
// emulator_main returns a single-row, single-column page on the
// `jobs.query` response, and the same row replays on
// `jobs.getQueryResults` keyed on the freshly-minted jobReference.
//
// Exercises the full REST -> gRPC -> Query.ExecuteQuery -> semantic
// executor path plus the gateway's per-process job registry that
// backs the synchronous query API. After
// `googlesqlite-07-semantic-core-expr.plan.md` landed the route classifier
// promotes scalar-only SELECT to the local semantic executor, so
// `SELECT 1` returns the same wire envelope the DuckDB fast path
// would have produced for the same query.
func TestQuerySelectOneRoundTrip(t *testing.T) {
	env := startEmulator(t)

	const projectID = "proj-select1"
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":"SELECT 1","useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("jobs.query -> %d: %s", status, string(body))
	}

	var run bqtypes.QueryResponse
	if err := json.Unmarshal(body, &run); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if run.Kind != "bigquery#queryResponse" {
		t.Errorf("kind = %q, want %q", run.Kind, "bigquery#queryResponse")
	}
	if !run.JobComplete {
		t.Error("jobComplete = false, want true for SELECT 1")
	}
	if run.JobReference == nil || run.JobReference.JobID == "" {
		t.Fatalf("jobReference missing on QueryResponse: %+v", run)
	}
	if run.JobReference.ProjectID != projectID {
		t.Errorf("jobReference.projectId = %q, want %q",
			run.JobReference.ProjectID, projectID)
	}
	if run.TotalRows != "1" {
		t.Errorf("totalRows = %q, want %q", run.TotalRows, "1")
	}
	if len(run.Rows) != 1 || len(run.Rows[0].F) != 1 {
		t.Fatalf("rows shape = %+v, want one row with one cell", run.Rows)
	}
	if v := run.Rows[0].F[0].V; v != "1" {
		t.Errorf("rows[0].f[0].v = %v, want %q", v, "1")
	}

	// jobs.getQueryResults must replay the same row off the
	// registry, with a getQueryResultsResponse-shaped envelope.
	status, body = doJSON(t, http.MethodGet,
		base+"/queries/"+run.JobReference.JobID, nil)
	if status != http.StatusOK {
		t.Fatalf("jobs.getQueryResults -> %d: %s", status, string(body))
	}
	var get bqtypes.QueryResponse
	if err := json.Unmarshal(body, &get); err != nil {
		t.Fatalf("decode getQueryResults body: %v (body=%s)", err, string(body))
	}
	if get.Kind != "bigquery#getQueryResultsResponse" {
		t.Errorf("getQueryResults kind = %q, want %q",
			get.Kind, "bigquery#getQueryResultsResponse")
	}
	if !get.JobComplete {
		t.Error("getQueryResults jobComplete = false, want true")
	}
	if get.TotalRows != "1" {
		t.Errorf("getQueryResults totalRows = %q, want %q",
			get.TotalRows, "1")
	}
	if len(get.Rows) != 1 || len(get.Rows[0].F) != 1 {
		t.Fatalf("getQueryResults rows shape = %+v, want one row with one cell",
			get.Rows)
	}
	if v := get.Rows[0].F[0].V; v != "1" {
		t.Errorf("getQueryResults rows[0].f[0].v = %v, want %q", v, "1")
	}
}

// TestQuerySelectStarAfterInsertAll is the end-to-end story for
// table reads: stream rows in via tabledata.insertAll, then run
// `SELECT * FROM ds.t` over the synchronous query API and confirm
// the rows round-trip with the same cell values. Exercises the full
// REST -> gRPC -> Query.ExecuteQuery -> DuckDB engine ->
// DuckDBStorage path plus the f/v cell encoding on both reads.
func TestQuerySelectStarAfterInsertAll(t *testing.T) {
	env := startEmulator(t)

	const (
		projectID = "proj-selectstar"
		datasetID = "ds_selectstar"
		tableID   = "people"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	tableBody := `{
        "tableReference":{"projectId":"` + projectID +
		`","datasetId":"` + datasetID +
		`","tableId":"` + tableID + `"},
        "schema":{"fields":[
            {"name":"id","type":"INT64","mode":"REQUIRED"},
            {"name":"name","type":"STRING","mode":"NULLABLE"}
        ]}
    }`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables", []byte(tableBody))
	if status != http.StatusOK {
		t.Fatalf("tables.insert -> %d: %s", status, string(body))
	}

	insertBody := `{
        "rows":[
            {"insertId":"a","json":{"id":1,"name":"alice"}},
            {"insertId":"b","json":{"id":2,"name":"bob"}}
        ]
    }`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables/"+tableID+"/insertAll",
		[]byte(insertBody))
	if status != http.StatusOK {
		t.Fatalf("tabledata.insertAll -> %d: %s", status, string(body))
	}

	queryBody := `{"query":"SELECT id, name FROM ` + datasetID +
		`.` + tableID + ` ORDER BY id","useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost, base+"/queries",
		[]byte(queryBody))
	if status != http.StatusOK {
		t.Fatalf("jobs.query -> %d: %s", status, string(body))
	}

	var run bqtypes.QueryResponse
	if err := json.Unmarshal(body, &run); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if !run.JobComplete {
		t.Error("jobComplete = false, want true")
	}
	if run.TotalRows != "2" {
		t.Errorf("totalRows = %q, want %q", run.TotalRows, "2")
	}
	if len(run.Rows) != 2 {
		t.Fatalf("rows = %d, want 2: %+v", len(run.Rows), run.Rows)
	}
	pairs := []string{
		stringCell(run.Rows[0].F[0]) + ":" + stringCell(run.Rows[0].F[1]),
		stringCell(run.Rows[1].F[0]) + ":" + stringCell(run.Rows[1].F[1]),
	}
	wantPairs := map[string]bool{"1:alice": true, "2:bob": true}
	for _, p := range pairs {
		if !wantPairs[p] {
			t.Errorf("unexpected row pair %q (want one of %v)", p, wantPairs)
		}
		delete(wantPairs, p)
	}
	if len(wantPairs) != 0 {
		t.Errorf("missing rows: %v", wantPairs)
	}

	if run.JobReference == nil {
		t.Fatal("jobReference missing on QueryResponse")
	}
	status, body = doJSON(t, http.MethodGet,
		base+"/queries/"+run.JobReference.JobID, nil)
	if status != http.StatusOK {
		t.Fatalf("jobs.getQueryResults -> %d: %s", status, string(body))
	}
	var get bqtypes.QueryResponse
	if err := json.Unmarshal(body, &get); err != nil {
		t.Fatalf("decode getQueryResults: %v (body=%s)", err, string(body))
	}
	if get.TotalRows != "2" {
		t.Errorf("getQueryResults totalRows = %q, want %q",
			get.TotalRows, "2")
	}
	if len(get.Rows) != 2 {
		t.Fatalf("getQueryResults rows = %d, want 2: %+v",
			len(get.Rows), get.Rows)
	}
}

// stringCell extracts the string form of a cell value. The wire
// shape for scalar cells is always a string regardless of underlying
// SQL type (see docs/REST_API.md "Type wire encoding"); this helper
// keeps the row-pair assertions readable.
func stringCell(c bqtypes.Cell) string {
	if s, ok := c.V.(string); ok {
		return s
	}
	return ""
}
