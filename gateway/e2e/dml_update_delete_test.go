//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"os"
	"sort"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestDMLUpdateRoundTrip is the Phase 6b end-to-end story for
// UPDATE: create a table, seed it via INSERT VALUES, run an UPDATE
// through `jobs.query`, verify the response carries the expected
// `dmlStats.updatedRowCount` / `numDmlAffectedRows`, and read the
// rows back via `tabledata.list` to confirm the storage round-trip
// landed.
func TestDMLUpdateRoundTrip(t *testing.T) {
	env := startEmulator(t)
	const (
		projectID = "proj-dml-update"
		datasetID = "ds_dml_update"
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

	// Seed via INSERT VALUES so the column types match end-to-end
	// (tabledata.insertAll stringifies numeric values; see the
	// matching note in dml_insert_test.go::TestDMLInsertSelectRoundTrip).
	seedBody := `{"query":"INSERT INTO ` + datasetID + `.` + tableID +
		` (id, name) VALUES (1, 'ada'), (2, 'linus'), (3, 'grace')",` +
		`"useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost,
		base+"/queries", []byte(seedBody))
	if status != http.StatusOK {
		t.Fatalf("jobs.query (seed) -> %d: %s", status, string(body))
	}

	updateBody := `{"query":"UPDATE ` + datasetID + `.` + tableID +
		` SET name = 'augusta' WHERE id = 1","useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost,
		base+"/queries", []byte(updateBody))
	if status != http.StatusOK {
		t.Fatalf("jobs.query (UPDATE) -> %d: %s", status, string(body))
	}

	var run bqtypes.QueryResponse
	if err := json.Unmarshal(body, &run); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if !run.JobComplete {
		t.Errorf("jobComplete = false, want true for completed UPDATE")
	}
	if run.Schema != nil {
		t.Errorf("schema = %+v, want nil for DML response", run.Schema)
	}
	if len(run.Rows) != 0 {
		t.Errorf("rows = %d, want 0 for DML response", len(run.Rows))
	}
	if run.NumDmlAffectedRows != "1" {
		t.Errorf("numDmlAffectedRows = %q, want %q",
			run.NumDmlAffectedRows, "1")
	}
	if run.DmlStats == nil {
		t.Fatalf("dmlStats missing on UPDATE response: %s", string(body))
	}
	if run.DmlStats.UpdatedRowCount != "1" {
		t.Errorf("dmlStats.updatedRowCount = %q, want %q",
			run.DmlStats.UpdatedRowCount, "1")
	}

	// Storage round-trip: id=1 now reads as 'augusta' and the other
	// two rows are unchanged.
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
	pairs := []string{
		stringCell(list.Rows[0].F[0]) + ":" + stringCell(list.Rows[0].F[1]),
		stringCell(list.Rows[1].F[0]) + ":" + stringCell(list.Rows[1].F[1]),
		stringCell(list.Rows[2].F[0]) + ":" + stringCell(list.Rows[2].F[1]),
	}
	sort.Strings(pairs)
	want := []string{"1:augusta", "2:linus", "3:grace"}
	sort.Strings(want)
	for i := range want {
		if pairs[i] != want[i] {
			t.Errorf("rows[%d] = %q, want %q", i, pairs[i], want[i])
		}
	}
}

// TestDMLDeleteRoundTrip is the Phase 6b end-to-end story for DELETE:
// seed a table, drop one row via `DELETE FROM ...`, and verify both
// the `dmlStats.deletedRowCount` envelope and the `tabledata.list`
// storage snapshot.
func TestDMLDeleteRoundTrip(t *testing.T) {
	env := startEmulator(t)
	const (
		projectID = "proj-dml-delete"
		datasetID = "ds_dml_delete"
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

	seedBody := `{"query":"INSERT INTO ` + datasetID + `.` + tableID +
		` (id, name) VALUES (1, 'ada'), (2, 'linus'), (3, 'grace')",` +
		`"useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost,
		base+"/queries", []byte(seedBody))
	if status != http.StatusOK {
		t.Fatalf("jobs.query (seed) -> %d: %s", status, string(body))
	}

	deleteBody := `{"query":"DELETE FROM ` + datasetID + `.` + tableID +
		` WHERE id = 2","useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost,
		base+"/queries", []byte(deleteBody))
	if status != http.StatusOK {
		t.Fatalf("jobs.query (DELETE) -> %d: %s", status, string(body))
	}

	var run bqtypes.QueryResponse
	if err := json.Unmarshal(body, &run); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if run.NumDmlAffectedRows != "1" {
		t.Errorf("numDmlAffectedRows = %q, want %q",
			run.NumDmlAffectedRows, "1")
	}
	if run.DmlStats == nil || run.DmlStats.DeletedRowCount != "1" {
		t.Errorf("dmlStats = %+v, want deletedRowCount=1", run.DmlStats)
	}

	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables/"+tableID+"/data", nil)
	if status != http.StatusOK {
		t.Fatalf("tabledata.list -> %d: %s", status, string(body))
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
	pairs := []string{
		stringCell(list.Rows[0].F[0]) + ":" + stringCell(list.Rows[0].F[1]),
		stringCell(list.Rows[1].F[0]) + ":" + stringCell(list.Rows[1].F[1]),
	}
	sort.Strings(pairs)
	want := []string{"1:ada", "3:grace"}
	sort.Strings(want)
	for i := range want {
		if pairs[i] != want[i] {
			t.Errorf("rows[%d] = %q, want %q", i, pairs[i], want[i])
		}
	}
}

// TestDMLDeleteWhereTrueClearsTable pins the empty-table case: a
// `DELETE FROM t WHERE TRUE` (or any always-true predicate) must
// remove every row and surface a matching dmlStats count.
func TestDMLDeleteWhereTrueClearsTable(t *testing.T) {
	env := startEmulator(t)
	const (
		projectID = "proj-dml-delete-all"
		datasetID = "ds_dml_delete_all"
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
            {"name":"id","type":"INT64","mode":"REQUIRED"}
        ]}
    }`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables", []byte(tableBody))
	if status != http.StatusOK {
		t.Fatalf("tables.insert -> %d: %s", status, string(body))
	}

	seedBody := `{"query":"INSERT INTO ` + datasetID + `.` + tableID +
		` (id) VALUES (10), (20), (30)","useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost,
		base+"/queries", []byte(seedBody))
	if status != http.StatusOK {
		t.Fatalf("jobs.query (seed) -> %d: %s", status, string(body))
	}

	deleteBody := `{"query":"DELETE FROM ` + datasetID + `.` + tableID +
		` WHERE TRUE","useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost,
		base+"/queries", []byte(deleteBody))
	if status != http.StatusOK {
		t.Fatalf("jobs.query (DELETE) -> %d: %s", status, string(body))
	}

	var run bqtypes.QueryResponse
	if err := json.Unmarshal(body, &run); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if run.DmlStats == nil || run.DmlStats.DeletedRowCount != "3" {
		t.Errorf("dmlStats = %+v, want deletedRowCount=3", run.DmlStats)
	}

	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables/"+tableID+"/data", nil)
	if status != http.StatusOK {
		t.Fatalf("tabledata.list -> %d: %s", status, string(body))
	}
	var list bqtypes.TableDataList
	if err := json.Unmarshal(body, &list); err != nil {
		t.Fatalf("decode TableDataList: %v (body=%s)", err, string(body))
	}
	if list.TotalRows != "0" {
		t.Errorf("tabledata.list totalRows = %q, want %q",
			list.TotalRows, "0")
	}
	if len(list.Rows) != 0 {
		t.Errorf("tabledata.list rows = %d, want 0", len(list.Rows))
	}
}

// TestDMLMergeOnDuckDB is the Plan 34 positive end-to-end story for
// MERGE on the canonical DuckDB engine + DuckDB storage configuration
// (`--engine=duckdb --storage=duckdb --on_unknown_fn=fallback`). The
// test seeds a target table, runs a MERGE that covers both branches
// of the asymmetry pinned in `TestDMLMergeOnReferenceImplIsUnimplemented`:
//
//   - WHEN MATCHED -> UPDATE: id=2's name flips from 'linus' to
//     'linus-updated'.
//   - WHEN NOT MATCHED -> INSERT: id=4 (not in the target) is
//     inserted with name 'rust'.
//
// We then assert on (a) the dmlStats envelope (updatedRowCount=1,
// insertedRowCount=1, deletedRowCount=0) and (b) the storage
// round-trip via `tabledata.list` so the post-MERGE state is the
// union of the surviving original rows plus the merged-in row.
//
// Engine asymmetry pinning lives one test below in
// `TestDMLMergeOnReferenceImplIsUnimplemented`; the pair documents
// the Phase 6b decision (HANDOFF.md §4.3 path 3) to ship MERGE on the
// DuckDB engine only and leave the reference-impl path as a
// documented UNIMPLEMENTED until plans 40-42's conformance harness
// surfaces a divergence worth filling in.
func TestDMLMergeOnDuckDB(t *testing.T) {
	env := startEmulatorWithFlags(t, emulatorFlags{
		engine:      "duckdb",
		storage:     "duckdb",
		onUnknownFn: "fallback",
		dataDir:     t.TempDir(),
	})
	const (
		projectID = "proj-dml-merge-duckdb"
		datasetID = "ds_dml_merge_duckdb"
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

	// Seed via INSERT VALUES so the column types match end-to-end;
	// `tabledata.insertAll` would stringify the INT64 id (see the
	// comment in dml_insert_test.go::TestDMLInsertSelectRoundTrip).
	// INSERT on the DuckDB engine path returns UNIMPLEMENTED and the
	// FallbackEngine wrapper hands the seed query off to the
	// reference-impl engine; we rely on that handoff here.
	seedBody := `{"query":"INSERT INTO ` + datasetID + `.` + tableID +
		` (id, name) VALUES (1, 'ada'), (2, 'linus'), (3, 'grace')",` +
		`"useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost,
		base+"/queries", []byte(seedBody))
	if status != http.StatusOK {
		t.Fatalf("jobs.query (seed) -> %d: %s", status, string(body))
	}

	mergeBody := `{"query":"MERGE INTO ` + datasetID + `.` + tableID +
		` T USING (SELECT 2 AS id, 'linus-updated' AS name UNION ALL ` +
		`SELECT 4 AS id, 'rust' AS name) S ON T.id = S.id ` +
		`WHEN MATCHED THEN UPDATE SET name = S.name ` +
		`WHEN NOT MATCHED THEN INSERT (id, name) ` +
		`VALUES (S.id, S.name)","useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost,
		base+"/queries", []byte(mergeBody))
	if status != http.StatusOK {
		t.Fatalf("jobs.query (MERGE) -> %d: %s", status, string(body))
	}

	var run bqtypes.QueryResponse
	if err := json.Unmarshal(body, &run); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if !run.JobComplete {
		t.Errorf("jobComplete = false, want true for completed MERGE")
	}
	if run.Schema != nil {
		t.Errorf("schema = %+v, want nil for DML response", run.Schema)
	}
	if len(run.Rows) != 0 {
		t.Errorf("rows = %d, want 0 for DML response", len(run.Rows))
	}
	if run.DmlStats == nil {
		t.Fatalf("dmlStats missing on MERGE response: %s", string(body))
	}
	if run.DmlStats.UpdatedRowCount != "1" {
		t.Errorf("dmlStats.updatedRowCount = %q, want %q",
			run.DmlStats.UpdatedRowCount, "1")
	}
	if run.DmlStats.InsertedRowCount != "1" {
		t.Errorf("dmlStats.insertedRowCount = %q, want %q",
			run.DmlStats.InsertedRowCount, "1")
	}
	if run.DmlStats.DeletedRowCount != "" &&
		run.DmlStats.DeletedRowCount != "0" {
		t.Errorf("dmlStats.deletedRowCount = %q, want empty or %q",
			run.DmlStats.DeletedRowCount, "0")
	}

	// Storage round-trip: post-MERGE state has id=2 updated to
	// 'linus-updated' and id=4 newly inserted alongside the
	// untouched id=1 and id=3 rows.
	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables/"+tableID+"/data", nil)
	if status != http.StatusOK {
		t.Fatalf("tabledata.list -> %d: %s", status, string(body))
	}
	var list bqtypes.TableDataList
	if err := json.Unmarshal(body, &list); err != nil {
		t.Fatalf("decode TableDataList: %v (body=%s)", err, string(body))
	}
	if list.TotalRows != "4" {
		t.Errorf("tabledata.list totalRows = %q, want %q",
			list.TotalRows, "4")
	}
	if len(list.Rows) != 4 {
		t.Fatalf("tabledata.list rows = %d, want 4: %+v",
			len(list.Rows), list.Rows)
	}
	pairs := make([]string, 0, len(list.Rows))
	for _, r := range list.Rows {
		pairs = append(pairs, stringCell(r.F[0])+":"+stringCell(r.F[1]))
	}
	sort.Strings(pairs)
	want := []string{"1:ada", "2:linus-updated", "3:grace", "4:rust"}
	sort.Strings(want)
	for i := range want {
		if pairs[i] != want[i] {
			t.Errorf("rows[%d] = %q, want %q", i, pairs[i], want[i])
		}
	}
}

// TestDMLMergeOnReferenceImplIsUnimplemented pins the engine
// asymmetry around MERGE: the reference-impl engine returns
// UNIMPLEMENTED today because the GoogleSQL reference-impl algebrizer
// does not algebrize `ResolvedMergeStmt` at the statement root (see
// the `// TODO: Add MERGE support.` comment in
// `googlesql/reference_impl/algebrizer.cc::AlgebrizeStatement`). Plan
// 34's engine-policy decision (HANDOFF.md §4.3 path 3, "DuckDB-only
// MERGE") was to land MERGE on the DuckDB engine and leave the
// reference-impl path as the documented UNIMPLEMENTED -- the gateway
// maps that to HTTP 501.
//
// This test runs against the default `reference_impl + memory`
// configuration (the engine selected when no `--engine` flag is
// passed); the matching positive case for the DuckDB engine lives
// above in `TestDMLMergeOnDuckDB`. The pair, taken together,
// documents the asymmetry rather than pinning failure as success.
//
// Run only when the active engine is the reference-impl engine: gate
// on the same `BIGQUERY_EMULATOR_E2E_ENGINE` env var the orchestrator
// scripts can set when they want to flip the default engine for a
// sweep; the empty / unset case is the legacy `reference_impl`
// default `startEmulator` uses, so the test runs there too.
func TestDMLMergeOnReferenceImplIsUnimplemented(t *testing.T) {
	if engine := os.Getenv("BIGQUERY_EMULATOR_E2E_ENGINE"); engine != "" &&
		engine != "reference_impl" {
		t.Skipf("BIGQUERY_EMULATOR_E2E_ENGINE=%q; this test pins MERGE "+
			"behavior only for the reference-impl engine (see "+
			"TestDMLMergeOnDuckDB for the DuckDB-engine round-trip)",
			engine)
	}
	env := startEmulator(t)
	const (
		projectID = "proj-dml-merge"
		datasetID = "ds_dml_merge"
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

	mergeBody := `{"query":"MERGE INTO ` + datasetID + `.` + tableID +
		` T USING (SELECT 1 AS id, 'ada' AS name) S ON T.id = S.id ` +
		`WHEN NOT MATCHED THEN INSERT (id, name) VALUES (S.id, S.name)",` +
		`"useLegacySql":false}`
	status, _ = doJSON(t, http.MethodPost,
		base+"/queries", []byte(mergeBody))
	if status != http.StatusNotImplemented {
		t.Fatalf("jobs.query (MERGE on reference-impl) -> %d, want 501",
			status)
	}
}
