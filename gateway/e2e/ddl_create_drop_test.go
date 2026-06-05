//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"sort"
	"strconv"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestDDLCreateTableAsSelectRoundTrip is the end-to-end story for
// `CREATE TABLE AS SELECT` on the local execution coordinator over
// DuckDB storage. CTAS is dispatched by the route classifier to the
// `control_op` executor (see
// `backend/engine/control/control_op_executor.cc`); reads through the
// inner SELECT continue to lower through the DuckDB transpiler. The
// engine-policy contract this test pins lives at
// `docs/ENGINE_POLICY.md` and the route plan in
// `.cursor/plans/local-exec-01-ddl-catalog.plan.md`.
//
// The test:
//
//  1. Creates a `src` table and seeds it with three rows via
//     `tabledata.insertAll` (the canonical seed path; `INSERT VALUES`
//     is also supported via `local-exec-14-dml-system.plan.md` but
//     `tabledata.insertAll` keeps the test focused on the DDL story).
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
		dataDir: t.TempDir(),
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

	// Seed via `tabledata.insertAll` because INSERT VALUES is
	// UNIMPLEMENTED on the DuckDB engine today. The insertAll path
	// stringifies INT64 cells (see `dml_insert_test.go` for the
	// historical note), but the CTAS step copies cells through
	// DuckDB which renormalizes them in the inner SELECT.
	seedBody := `{"kind":"bigquery#tableDataInsertAllRequest","rows":[
        {"json":{"id":1,"name":"ada"}},
        {"json":{"id":2,"name":"linus"}},
        {"json":{"id":3,"name":"grace"}}
    ]}`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables/"+srcTable+"/insertAll",
		[]byte(seedBody))
	if status != http.StatusOK {
		t.Fatalf("tabledata.insertAll (seed) -> %d: %s", status, string(body))
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
	// `Job.statistics.query.statementType` must match BigQuery's
	// REST canonical value for CTAS. The gateway populates the
	// envelope from the trailing `statement_type` marker the
	// engine emits per
	// `.cursor/plans/local-exec-01-ddl-catalog.plan.md` Item 5.
	if run.Statistics == nil || run.Statistics.Query == nil {
		t.Fatalf("statistics.query missing on CTAS response: %+v",
			run.Statistics)
	}
	if got := run.Statistics.Query.StatementType; got != "CREATE_TABLE_AS_SELECT" {
		t.Errorf("statistics.query.statementType = %q, want %q",
			got, "CREATE_TABLE_AS_SELECT")
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

// TestDDLStatementTypeEnvelopeMatchesBigQueryRESTValues exercises
// the per-statement `Job.statistics.query.statementType` plumbing
// the control-op executor (`backend/engine/control/control_op_executor.cc`)
// + frontend (`frontend/handlers/query.cc::StatementTypeFor`) +
// gateway (`gateway/handlers/queries.go`) ship together. Per
// `.cursor/plans/local-exec-01-ddl-catalog.plan.md` Done Criterion #3,
// every supported DDL / metadata / catalog statement must surface
// the BigQuery REST canonical statement-type string. This test
// pins three representative shapes (CREATE_TABLE, DROP_TABLE,
// SELECT) so a regression in the trailing-message contract or the
// gateway's `streamQueryResults` decoder breaks the gateway/e2e
// suite immediately.
func TestDDLStatementTypeEnvelopeMatchesBigQueryRESTValues(t *testing.T) {
	env := startEmulatorWithFlags(t, emulatorFlags{
		dataDir: t.TempDir(),
	})

	const (
		projectID = "proj-stmt-type"
		datasetID = "ds_stmt_type"
		tableID   = "people"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	runQuery := func(t *testing.T, sql string) bqtypes.QueryResponse {
		t.Helper()
		status, body := doJSON(t, http.MethodPost, base+"/queries",
			[]byte(`{"query":`+strconv.Quote(sql)+`,"useLegacySql":false}`))
		if status != http.StatusOK {
			t.Fatalf("jobs.query(%q) -> %d: %s", sql, status, string(body))
		}
		var resp bqtypes.QueryResponse
		if err := json.Unmarshal(body, &resp); err != nil {
			t.Fatalf("decode QueryResponse for %q: %v (body=%s)",
				sql, err, string(body))
		}
		if !resp.JobComplete {
			t.Fatalf("jobComplete=false for %q: %s", sql, string(body))
		}
		return resp
	}
	wantStatementType := func(t *testing.T, label, want string,
		resp bqtypes.QueryResponse,
	) {
		t.Helper()
		if resp.Statistics == nil || resp.Statistics.Query == nil {
			t.Fatalf("%s: statistics.query missing on response: %+v",
				label, resp.Statistics)
		}
		if got := resp.Statistics.Query.StatementType; got != want {
			t.Errorf("%s: statistics.query.statementType = %q, want %q",
				label, got, want)
		}
	}

	createResp := runQuery(t,
		"CREATE TABLE "+datasetID+"."+tableID+
			" (id INT64, name STRING)")
	wantStatementType(t, "CREATE TABLE", "CREATE_TABLE", createResp)

	selectResp := runQuery(t,
		"SELECT id FROM "+datasetID+"."+tableID)
	wantStatementType(t, "SELECT", "SELECT", selectResp)

	dropResp := runQuery(t, "DROP TABLE "+datasetID+"."+tableID)
	wantStatementType(t, "DROP TABLE", "DROP_TABLE", dropResp)
}
