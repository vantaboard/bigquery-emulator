//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"sort"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestDDLCreateTableAsSelectRoundTrip is the Plan-35 end-to-end story
// for `CREATE TABLE AS SELECT` on the canonical DuckDB engine + DuckDB
// storage configuration (`--engine=duckdb --storage=duckdb
// --on_unknown_fn=fallback`). The DDL surface is implemented entirely
// on the DuckDB engine (see
// `backend/engine/duckdb/duckdb_engine.cc::ExecuteDdl`); the
// reference-impl engine returns UNIMPLEMENTED and the FallbackEngine
// wrapper retries against DuckDB so DDL still works on
// `--engine=reference_impl --on_unknown_fn=fallback`. Plan 35's
// engine-policy decision (extending HANDOFF.md §4.3 path 3's
// "DuckDB-only MERGE" pattern) is what this test pins.
//
// The test:
//
//  1. Creates a `src` table and seeds it with three rows via INSERT
//     VALUES (which the FallbackEngine routes to the reference-impl
//     engine under the canonical Phase 5i `--engine=duckdb
//     --on_unknown_fn=fallback` shape, since INSERT lives on
//     reference-impl today).
//  2. Runs `CREATE TABLE ds.copy AS SELECT id, name FROM ds.src`
//     through `jobs.query` and checks the response surfaces
//     `jobComplete=true` with no schema / rows / dmlStats (the
//     gateway shape for a successful DDL).
//  3. Reads the resulting table back via `tabledata.list` and
//     verifies the storage round-trip lands the inner-SELECT rows
//     under the BigQuery-typed schema the analyzer's
//     `column_definition_list` carried.
func TestDDLCreateTableAsSelectRoundTrip(t *testing.T) {
	env := startEmulatorWithFlags(t, emulatorFlags{
		engine:      "duckdb",
		storage:     "duckdb",
		onUnknownFn: "fallback",
		dataDir:     t.TempDir(),
	})

	const (
		projectID = "proj-ddl-ctas"
		datasetID = "ds_ddl_ctas"
		srcTable  = "src"
		dstTable  = "people_copy"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	srcBody := `{
        "tableReference":{"projectId":"` + projectID +
		`","datasetId":"` + datasetID +
		`","tableId":"` + srcTable + `"},
        "schema":{"fields":[
            {"name":"id","type":"INT64","mode":"REQUIRED"},
            {"name":"name","type":"STRING","mode":"NULLABLE"}
        ]}
    }`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables", []byte(srcBody))
	if status != http.StatusOK {
		t.Fatalf("tables.insert(src) -> %d: %s", status, string(body))
	}

	// Seed via INSERT VALUES so the type fidelity matches end-to-end;
	// `tabledata.insertAll` would stringify the INT64 id (see the
	// matching note in `dml_insert_test.go`).
	seedBody := `{"query":"INSERT INTO ` + datasetID + `.` + srcTable +
		` (id, name) VALUES (1, 'ada'), (2, 'linus'), (3, 'grace')",` +
		`"useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost,
		base+"/queries", []byte(seedBody))
	if status != http.StatusOK {
		t.Fatalf("jobs.query (seed) -> %d: %s", status, string(body))
	}

	ctasBody := `{"query":"CREATE TABLE ` + datasetID + `.` + dstTable +
		` AS SELECT id, name FROM ` + datasetID + `.` + srcTable +
		`","useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost,
		base+"/queries", []byte(ctasBody))
	if status != http.StatusOK {
		t.Fatalf("jobs.query (CTAS) -> %d: %s", status, string(body))
	}

	var run bqtypes.QueryResponse
	if err := json.Unmarshal(body, &run); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if !run.JobComplete {
		t.Errorf("jobComplete = false, want true for completed CTAS")
	}
	// DDL reply: no schema / rows / dmlStats. We only check
	// jobComplete + the storage round-trip below.
	if run.Schema != nil {
		t.Errorf("schema = %+v, want nil for DDL response", run.Schema)
	}
	if len(run.Rows) != 0 {
		t.Errorf("rows = %d, want 0 for DDL response", len(run.Rows))
	}

	// Storage round-trip: tabledata.list against the new table must
	// surface the three rows we seeded into the source.
	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables/"+dstTable+"/data", nil)
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
	pairs := make([]string, 0, len(list.Rows))
	for _, r := range list.Rows {
		pairs = append(pairs, stringCell(r.F[0])+":"+stringCell(r.F[1]))
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
