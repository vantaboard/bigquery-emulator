//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"strconv"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestDMLDDLFullRoundTrip is the Plan-36 (`dml-ddl-e2e`) verification
// matched by `go test -tags=integration ./gateway/e2e/... -run DMLDDL`
// in the plan's `Verification` block. It threads the full DML + DDL
// surface end-to-end against the canonical Phase 5i+ configuration
// (`--engine=duckdb --storage=duckdb --on_unknown_fn=fallback`):
//
//  1. CREATE TABLE  (DDL on DuckDB, plan 35)
//  2. INSERT VALUES (DML on the reference-impl engine, routed via
//     FallbackEngine; pins `dmlStats.insertedRowCount` envelope)
//  3. SELECT COUNT(*)
//     verifies the rows actually landed in storage
//  4. UPDATE ... WHERE         (DML; pins `dmlStats.updatedRowCount`)
//  5. SELECT name WHERE id=...
//     verifies the UPDATE mutated the row in place
//  6. DELETE FROM ... WHERE    (DML; pins `dmlStats.deletedRowCount`)
//  7. SELECT COUNT(*)
//     verifies the DELETE shrank the row set
//  8. CREATE TABLE AS SELECT   (DDL on DuckDB, plan 35)
//  9. SELECT COUNT(*) from the CTAS-produced table
//  10. tabledata.insertAll on the original table (legacy streaming
//     insert path; pins it still works alongside the SQL DML surface)
//  11. SELECT COUNT(*) post-insertAll
//  12. DROP TABLE on the CTAS copy (DDL on DuckDB, plan 35)
//
// The test fails loudly on the first deviation so a regression in
// any of those layers (engine routing, DML stats envelope,
// tabledata.insertAll lowering, FallbackEngine wiring, DDL plumbing,
// storage round-trip) surfaces with a clear failure site rather
// than a downstream symptom.
func TestDMLDDLFullRoundTrip(t *testing.T) {
	env := startEmulatorWithFlags(t, emulatorFlags{
		engine:      "duckdb",
		storage:     "duckdb",
		onUnknownFn: "fallback",
		dataDir:     t.TempDir(),
	})

	const (
		projectID = "proj-dml-ddl-e2e"
		datasetID = "ds_dml_ddl_e2e"
		srcTable  = "people"
		dstTable  = "people_copy"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	mustPost := func(t *testing.T, label, path, body string) []byte {
		t.Helper()
		status, resp := doJSON(t, http.MethodPost, base+path, []byte(body))
		if status != http.StatusOK {
			t.Fatalf("%s -> %d: %s", label, status, string(resp))
		}
		return resp
	}
	mustGet := func(t *testing.T, label, path string) []byte {
		t.Helper()
		status, resp := doJSON(t, http.MethodGet, base+path, nil)
		if status != http.StatusOK {
			t.Fatalf("%s -> %d: %s", label, status, string(resp))
		}
		return resp
	}
	query := func(t *testing.T, sql string) bqtypes.QueryResponse {
		t.Helper()
		body := mustPost(t, "jobs.query("+sql+")", "/queries",
			`{"query":`+strconv.Quote(sql)+`,"useLegacySql":false}`)
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

	mustPost(t, "datasets.insert", "/datasets",
		`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`)

	// Step 1: CREATE TABLE (DDL on DuckDB)
	createResp := query(t,
		"CREATE TABLE "+datasetID+"."+srcTable+
			" (id INT64, name STRING, score FLOAT64)")
	if createResp.DmlStats != nil {
		t.Errorf("CREATE TABLE: dmlStats = %+v, want nil (DDL has no DML stats)",
			createResp.DmlStats)
	}

	// Step 2: INSERT VALUES (DML; verify dmlStats.insertedRowCount)
	insertResp := query(t,
		"INSERT INTO "+datasetID+"."+srcTable+
			" (id, name, score) VALUES (1, 'ada', 1.0), "+
			"(2, 'linus', 2.0), (3, 'grace', 3.0)")
	if insertResp.DmlStats == nil {
		t.Fatalf("INSERT: dmlStats missing on DML response")
	}
	if insertResp.DmlStats.InsertedRowCount != "3" {
		t.Errorf("INSERT: dmlStats.insertedRowCount = %q, want %q",
			insertResp.DmlStats.InsertedRowCount, "3")
	}
	if insertResp.NumDmlAffectedRows != "3" {
		t.Errorf("INSERT: numDmlAffectedRows = %q, want %q",
			insertResp.NumDmlAffectedRows, "3")
	}

	// Step 3: SELECT COUNT(*) -- rows really landed
	countResp := query(t,
		"SELECT COUNT(*) AS n FROM "+datasetID+"."+srcTable)
	if got := stringCell(countResp.Rows[0].F[0]); got != "3" {
		t.Errorf("post-INSERT COUNT(*) = %q, want %q", got, "3")
	}

	// Step 4: UPDATE (DML; verify dmlStats.updatedRowCount)
	updateResp := query(t,
		"UPDATE "+datasetID+"."+srcTable+
			" SET name = 'ada-lovelace' WHERE id = 1")
	if updateResp.DmlStats == nil {
		t.Fatalf("UPDATE: dmlStats missing on DML response")
	}
	if updateResp.DmlStats.UpdatedRowCount != "1" {
		t.Errorf("UPDATE: dmlStats.updatedRowCount = %q, want %q",
			updateResp.DmlStats.UpdatedRowCount, "1")
	}

	// Step 5: SELECT verifies UPDATE mutated the row
	verifyResp := query(t,
		"SELECT name FROM "+datasetID+"."+srcTable+" WHERE id = 1")
	if got := stringCell(verifyResp.Rows[0].F[0]); got != "ada-lovelace" {
		t.Errorf("post-UPDATE name@id=1 = %q, want %q", got, "ada-lovelace")
	}

	// Step 6: DELETE (DML; verify dmlStats.deletedRowCount)
	deleteResp := query(t,
		"DELETE FROM "+datasetID+"."+srcTable+" WHERE id = 3")
	if deleteResp.DmlStats == nil {
		t.Fatalf("DELETE: dmlStats missing on DML response")
	}
	if deleteResp.DmlStats.DeletedRowCount != "1" {
		t.Errorf("DELETE: dmlStats.deletedRowCount = %q, want %q",
			deleteResp.DmlStats.DeletedRowCount, "1")
	}

	// Step 7: SELECT COUNT(*) shrunk to 2
	count2Resp := query(t,
		"SELECT COUNT(*) AS n FROM "+datasetID+"."+srcTable)
	if got := stringCell(count2Resp.Rows[0].F[0]); got != "2" {
		t.Errorf("post-DELETE COUNT(*) = %q, want %q", got, "2")
	}

	// Step 8: CREATE TABLE AS SELECT (DDL on DuckDB)
	ctasResp := query(t,
		"CREATE TABLE "+datasetID+"."+dstTable+
			" AS SELECT id, name FROM "+datasetID+"."+srcTable)
	if ctasResp.DmlStats != nil {
		t.Errorf("CTAS: dmlStats = %+v, want nil", ctasResp.DmlStats)
	}

	// Step 9: SELECT COUNT(*) from the CTAS copy
	ctasCountResp := query(t,
		"SELECT COUNT(*) AS n FROM "+datasetID+"."+dstTable)
	if got := stringCell(ctasCountResp.Rows[0].F[0]); got != "2" {
		t.Errorf("CTAS COUNT(*) = %q, want %q (matches source minus deleted)",
			got, "2")
	}

	// Step 10: tabledata.insertAll on the source table -- the legacy
	// streaming insert path still works alongside the SQL DML
	// surface; the new row should be visible to subsequent SELECTs.
	mustPost(t, "tabledata.insertAll",
		"/datasets/"+datasetID+"/tables/"+srcTable+"/insertAll",
		`{"rows":[{"insertId":"r4",`+
			`"json":{"id":4,"name":"hopper","score":4.0}}]}`)

	// Step 11: SELECT COUNT(*) reflects the streamed row
	count3Resp := query(t,
		"SELECT COUNT(*) AS n FROM "+datasetID+"."+srcTable)
	if got := stringCell(count3Resp.Rows[0].F[0]); got != "3" {
		t.Errorf("post-insertAll COUNT(*) = %q, want %q (UPDATE'd ada + "+
			"linus + streamed hopper, grace deleted)", got, "3")
	}

	// Step 12: DROP TABLE on the CTAS copy (DDL on DuckDB)
	dropResp := query(t,
		"DROP TABLE "+datasetID+"."+dstTable)
	if dropResp.DmlStats != nil {
		t.Errorf("DROP: dmlStats = %+v, want nil", dropResp.DmlStats)
	}

	// And the dropped table is gone -- tables.get returns 404.
	status, _ := doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables/"+dstTable, nil)
	if status != http.StatusNotFound {
		// A 200 means the table is still there; a 404 means DROP did
		// what it said on the tin. Anything else is a transport bug.
		t.Errorf("tables.get(%s) post-DROP -> %d, want 404",
			dstTable, status)
	}

	// And the survivors are visible end-to-end via tabledata.list
	// (not SELECT) so the test also pins the storage layer.
	listBody := mustGet(t, "tabledata.list",
		"/datasets/"+datasetID+"/tables/"+srcTable+"/data")
	var list bqtypes.TableDataList
	if err := json.Unmarshal(listBody, &list); err != nil {
		t.Fatalf("decode TableDataList: %v (body=%s)", err, string(listBody))
	}
	if list.TotalRows != "3" {
		t.Errorf("tabledata.list totalRows = %q, want %q",
			list.TotalRows, "3")
	}
	if len(list.Rows) != 3 {
		t.Fatalf("tabledata.list rows = %d, want 3: %+v",
			len(list.Rows), list.Rows)
	}
}
