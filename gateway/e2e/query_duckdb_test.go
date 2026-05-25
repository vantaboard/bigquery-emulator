//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestQueryDuckDBEngineEndToEnd is the Phase 5i end-to-end story for
// the DuckDB engine: spin up `emulator_main` with
// `--engine=duckdb --storage=duckdb --on_unknown_fn=fallback`, seed
// rows over `tabledata.insertAll`, and confirm a handful of query
// shapes round-trip through the gateway. The shapes are chosen to
// cover the engine and its fallback wrapper simultaneously:
//
//   - `SELECT 1` exercises the FallbackEngine: the DuckDB engine
//     returns UNIMPLEMENTED (transpiler does not lower
//     ResolvedProjectScan with computed columns yet, see
//     `backend/engine/duckdb/transpiler/SHAPE_TRACKER.md`), the
//     wrapper retries against the reference-impl evaluator.
//   - `SELECT * FROM ds.t` exercises the DuckDB engine directly:
//     the analyzer wraps the TableScan in a pass-through ProjectScan
//     the engine strips before handing the inner scan to the
//     transpiler. DuckDB executes the SQL against the materialized
//     table.
//   - `SELECT COUNT(*)` exercises a GROUP BY-shaped aggregate. The
//     transpiler covers the implicit-grouping case directly through
//     the AggregateScan emit, so the row count comes back without
//     hitting the fallback path.
//
// The DuckDB storage backend persists Parquet under `--data_dir`; we
// give it `t.TempDir()` so the test is hermetic across runs.
func TestQueryDuckDBEngineEndToEnd(t *testing.T) {
	env := startEmulatorWithFlags(t, emulatorFlags{
		engine:      "duckdb",
		storage:     "duckdb",
		onUnknownFn: "fallback",
		dataDir:     t.TempDir(),
	})
	skipIfExecuteQueryUnimplemented(t, env)

	const (
		projectID = "proj-duckdb"
		datasetID = "ds_duckdb"
		tableID   = "people"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	// 1. Seed catalog + rows so the SELECT shapes have data.
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
            {"insertId":"a","json":{"id":1,"name":"ada"}},
            {"insertId":"b","json":{"id":2,"name":"linus"}},
            {"insertId":"c","json":{"id":3,"name":"grace"}}
        ]
    }`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables/"+tableID+"/insertAll",
		[]byte(insertBody))
	if status != http.StatusOK {
		t.Fatalf("tabledata.insertAll -> %d: %s", status, string(body))
	}

	// 2. SELECT 1 — exercises the FallbackEngine: DuckDB engine
	// returns UNIMPLEMENTED for the wrapping computed-column
	// ProjectScan, the wrapper retries against reference-impl.
	status, body = doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":"SELECT 1","useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("SELECT 1 -> %d: %s", status, string(body))
	}
	var sel1 bqtypes.QueryResponse
	if err := json.Unmarshal(body, &sel1); err != nil {
		t.Fatalf("decode SELECT 1: %v (body=%s)", err, string(body))
	}
	if !sel1.JobComplete {
		t.Error("SELECT 1 jobComplete = false, want true")
	}
	if sel1.TotalRows != "1" {
		t.Errorf("SELECT 1 totalRows = %q, want %q", sel1.TotalRows, "1")
	}
	if len(sel1.Rows) != 1 || len(sel1.Rows[0].F) != 1 {
		t.Fatalf("SELECT 1 rows shape = %+v, want one row with one cell",
			sel1.Rows)
	}
	if v := sel1.Rows[0].F[0].V; v != "1" {
		t.Errorf("SELECT 1 rows[0].f[0].v = %v, want %q", v, "1")
	}

	// 3. SELECT * FROM ds.t — exercises the DuckDB engine directly:
	// the pass-through ProjectScan strips, the transpiler emits the
	// table scan, DuckDB executes against the materialized table.
	queryBody := `{"query":"SELECT * FROM ` + datasetID +
		`.` + tableID + ` ORDER BY id","useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost, base+"/queries",
		[]byte(queryBody))
	if status != http.StatusOK {
		t.Fatalf("SELECT * -> %d: %s", status, string(body))
	}
	var selStar bqtypes.QueryResponse
	if err := json.Unmarshal(body, &selStar); err != nil {
		t.Fatalf("decode SELECT *: %v (body=%s)", err, string(body))
	}
	if !selStar.JobComplete {
		t.Error("SELECT * jobComplete = false, want true")
	}
	if selStar.TotalRows != "3" {
		t.Errorf("SELECT * totalRows = %q, want %q", selStar.TotalRows, "3")
	}
	if len(selStar.Rows) != 3 {
		t.Fatalf("SELECT * rows = %d, want 3: %+v",
			len(selStar.Rows), selStar.Rows)
	}
	wantPairs := []string{"1:ada", "2:linus", "3:grace"}
	for i, want := range wantPairs {
		if len(selStar.Rows[i].F) != 2 {
			t.Fatalf("SELECT * row[%d] cells = %d, want 2: %+v",
				i, len(selStar.Rows[i].F), selStar.Rows[i].F)
		}
		got := stringCell(selStar.Rows[i].F[0]) + ":" +
			stringCell(selStar.Rows[i].F[1])
		if got != want {
			t.Errorf("SELECT * row[%d] = %q, want %q", i, got, want)
		}
	}

	// 4. SELECT COUNT(*) — exercises the AggregateScan emit path.
	// The transpiler lowers implicit-grouping aggregates directly;
	// DuckDB returns a single-row, single-column result we round-trip
	// back through the gateway's f/v cell envelope.
	countBody := `{"query":"SELECT COUNT(*) AS c FROM ` + datasetID +
		`.` + tableID + `","useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost, base+"/queries",
		[]byte(countBody))
	if status != http.StatusOK {
		t.Fatalf("SELECT COUNT(*) -> %d: %s", status, string(body))
	}
	var count bqtypes.QueryResponse
	if err := json.Unmarshal(body, &count); err != nil {
		t.Fatalf("decode SELECT COUNT(*): %v (body=%s)", err, string(body))
	}
	if !count.JobComplete {
		t.Error("SELECT COUNT(*) jobComplete = false, want true")
	}
	if count.TotalRows != "1" {
		t.Errorf("SELECT COUNT(*) totalRows = %q, want %q",
			count.TotalRows, "1")
	}
	if len(count.Rows) != 1 || len(count.Rows[0].F) != 1 {
		t.Fatalf("SELECT COUNT(*) rows shape = %+v, want one row, one cell",
			count.Rows)
	}
	if v := count.Rows[0].F[0].V; v != "3" {
		t.Errorf("SELECT COUNT(*) rows[0].f[0].v = %v, want %q", v, "3")
	}
}
