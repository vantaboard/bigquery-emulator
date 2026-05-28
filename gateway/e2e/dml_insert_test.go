//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"sort"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestDMLInsertValuesRoundTrip is the Phase 6a end-to-end story for
// INSERT VALUES: create a dataset and a typed table over REST, run
// `INSERT INTO ds.t (...) VALUES (...)` through `jobs.query`, confirm
// the response carries the expected `dmlStats` / `numDmlAffectedRows`
// counters, and read the rows back via `tabledata.list` to verify
// the storage round-trip lands as expected.
func TestDMLInsertValuesRoundTrip(t *testing.T) {
	env := startEmulator(t)

	const (
		projectID = "proj-dml-insert"
		datasetID = "ds_dml_insert"
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

	queryBody := `{"query":"INSERT INTO ` + datasetID + `.` + tableID +
		` (id, name) VALUES (1, 'ada'), (2, 'linus'), (3, 'grace')",` +
		`"useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost,
		base+"/queries", []byte(queryBody))
	if status != http.StatusOK {
		t.Fatalf("jobs.query (INSERT) -> %d: %s", status, string(body))
	}

	var run bqtypes.QueryResponse
	if err := json.Unmarshal(body, &run); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if !run.JobComplete {
		t.Errorf("jobComplete = false, want true for completed INSERT")
	}
	// DML reply: no schema / rows, just the dmlStats envelope and
	// numDmlAffectedRows.
	if run.Schema != nil {
		t.Errorf("schema = %+v, want nil for DML response", run.Schema)
	}
	if len(run.Rows) != 0 {
		t.Errorf("rows = %d, want 0 for DML response", len(run.Rows))
	}
	if run.NumDmlAffectedRows != "3" {
		t.Errorf("numDmlAffectedRows = %q, want %q",
			run.NumDmlAffectedRows, "3")
	}
	if run.DmlStats == nil {
		t.Fatalf("dmlStats missing on DML response: %s", string(body))
	}
	if run.DmlStats.InsertedRowCount != "3" {
		t.Errorf("dmlStats.insertedRowCount = %q, want %q",
			run.DmlStats.InsertedRowCount, "3")
	}

	// Storage round-trip: tabledata.list must surface the same three
	// rows after the INSERT lands. We sort the returned ids so the
	// test stays robust against any future iteration-order change in
	// the in-memory store.
	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables/"+tableID+"/data", nil)
	if status != http.StatusOK {
		t.Fatalf("tabledata.list -> %d: %s", status, string(body))
	}
	var list bqtypes.TableDataList
	if err := json.Unmarshal(body, &list); err != nil {
		t.Fatalf("decode TableDataList: %v (body=%s)", err, string(body))
	}
	if list.TotalRows != "3" {
		t.Errorf("tabledata.list totalRows = %q, want %q",
			list.TotalRows, "3")
	}
	if len(list.Rows) != 3 {
		t.Fatalf("tabledata.list rows = %d, want 3: %+v",
			len(list.Rows), list.Rows)
	}
	pairs := []string{
		stringCell(list.Rows[0].F[0]) + ":" + stringCell(list.Rows[0].F[1]),
		stringCell(list.Rows[1].F[0]) + ":" + stringCell(list.Rows[1].F[1]),
		stringCell(list.Rows[2].F[0]) + ":" + stringCell(list.Rows[2].F[1]),
	}
	sort.Strings(pairs)
	want := []string{"1:ada", "2:linus", "3:grace"}
	sort.Strings(want)
	for i := range want {
		if pairs[i] != want[i] {
			t.Errorf("rows[%d] = %q, want %q", i, pairs[i], want[i])
		}
	}
}

// TestDMLInsertSelectRoundTrip exercises the INSERT INTO ... SELECT
// path: rows are streamed in via tabledata.insertAll, then a second
// table is filled via INSERT INTO ds.dst SELECT * FROM ds.src and
// verified through tabledata.list. Pins the engine's handling of the
// `query()` branch of `ResolvedInsertStmt` (as opposed to the VALUES
// `row_list()` branch covered above).
func TestDMLInsertSelectRoundTrip(t *testing.T) {
	env := startEmulator(t)

	const (
		projectID = "proj-dml-insert-select"
		datasetID = "ds_dml_insert_select"
		srcTable  = "src"
		dstTable  = "dst"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	for _, name := range []string{srcTable, dstTable} {
		tableBody := `{
            "tableReference":{"projectId":"` + projectID +
			`","datasetId":"` + datasetID +
			`","tableId":"` + name + `"},
            "schema":{"fields":[
                {"name":"id","type":"INT64","mode":"REQUIRED"},
                {"name":"name","type":"STRING","mode":"NULLABLE"}
            ]}
        }`
		status, body = doJSON(t, http.MethodPost,
			base+"/datasets/"+datasetID+"/tables", []byte(tableBody))
		if status != http.StatusOK {
			t.Fatalf("tables.insert(%s) -> %d: %s", name, status, string(body))
		}
	}

	// Populate the source table via INSERT VALUES rather than
	// `tabledata.insertAll`. The latter goes through the gateway's
	// `jsonToCell` shim that stringifies every numeric value (Phase
	// 3's intentional simplification, see
	// `gateway/handlers/tabledata.go::jsonToCell`); rows it lands
	// would carry STRING cells in the INT64 `id` column and trip
	// the analyzer's struct-type check on the inner SELECT during
	// `INSERT ... SELECT`. Going through DML keeps the
	// types honest end-to-end.
	seedBody := `{"query":"INSERT INTO ` + datasetID + `.` + srcTable +
		` (id, name) VALUES (1, 'alice'), (2, 'bob')",` +
		`"useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost,
		base+"/queries", []byte(seedBody))
	if status != http.StatusOK {
		t.Fatalf("jobs.query (seed src) -> %d: %s", status, string(body))
	}

	queryBody := `{"query":"INSERT INTO ` + datasetID + `.` + dstTable +
		` (id, name) SELECT id, name FROM ` + datasetID + `.` + srcTable +
		`","useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost,
		base+"/queries", []byte(queryBody))
	if status != http.StatusOK {
		t.Fatalf("jobs.query (INSERT SELECT) -> %d: %s", status, string(body))
	}
	var run bqtypes.QueryResponse
	if err := json.Unmarshal(body, &run); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if run.NumDmlAffectedRows != "2" {
		t.Errorf("numDmlAffectedRows = %q, want %q",
			run.NumDmlAffectedRows, "2")
	}
	if run.DmlStats == nil || run.DmlStats.InsertedRowCount != "2" {
		t.Errorf("dmlStats = %+v, want insertedRowCount=2", run.DmlStats)
	}

	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables/"+dstTable+"/data", nil)
	if status != http.StatusOK {
		t.Fatalf("tabledata.list(dst) -> %d: %s", status, string(body))
	}
	var list bqtypes.TableDataList
	if err := json.Unmarshal(body, &list); err != nil {
		t.Fatalf("decode TableDataList: %v (body=%s)", err, string(body))
	}
	if list.TotalRows != "2" {
		t.Errorf("tabledata.list totalRows = %q, want %q",
			list.TotalRows, "2")
	}
	if len(list.Rows) != 2 {
		t.Fatalf("tabledata.list rows = %d, want 2: %+v",
			len(list.Rows), list.Rows)
	}
}
