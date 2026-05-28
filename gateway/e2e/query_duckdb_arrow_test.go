//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"reflect"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestQueryDuckDBArrowWireParity is the Phase 5l parity check: with
// the DuckDB engine now pulling Arrow-style columnar chunks via
// `duckdb_fetch_chunk` (see `backend/engine/duckdb/arrow_to_bq.cc`),
// the f/v wire shape returned by the gateway must be byte-identical
// to what the reference-impl engine produces for the same query. If
// the two diverge we've broken the parity invariant the conformance
// harness relies on -- the whole point of the Arrow path is to be
// faster, not different.
//
// The test seeds the same dataset twice (once with `--engine=duckdb`,
// once with `--engine=reference_impl`) and runs `SELECT * FROM t
// ORDER BY id`. The DuckDB engine takes the new Arrow path because
// the analyzer wraps the TableScan in a pass-through ProjectScan we
// strip before handing off to the transpiler, then the chunked
// vector accessors in `arrow_to_bq` render the rows. The
// reference-impl engine takes its row-by-row evaluator path. Both
// pipelines lower their result rows through the same
// `frontend/handlers/query.cc::ValueToCell` -> proto Cell ->
// `bqtypes.ValueToCell` boundary, so the rendered Rows[] must match
// exactly.
//
// We compare via `reflect.DeepEqual` on the decoded `Rows` slice
// (rather than re-marshaling to JSON) so a stray difference in
// scalar formatting or nesting jumps out immediately.
func TestQueryDuckDBArrowWireParity(t *testing.T) {
	const (
		projectID = "proj-arrow-parity"
		datasetID = "ds_arrow"
		tableID   = "people"
	)
	queryBody := `{"query":"SELECT * FROM ` + datasetID + `.` + tableID +
		` ORDER BY id","useLegacySql":false}`

	type fixture struct {
		flags emulatorFlags
		label string
	}
	fixtures := []fixture{
		{
			label: "duckdb-arrow",
			flags: emulatorFlags{
				engine:      "duckdb",
				storage:     "duckdb",
				onUnknownFn: "fallback",
			},
		},
		{
			label: "reference-impl",
			flags: emulatorFlags{
				storage: "memory",
			},
		},
	}

	results := make(map[string]bqtypes.QueryResponse)
	for _, f := range fixtures {
		f := f
		t.Run(f.label, func(t *testing.T) {
			flags := f.flags
			if flags.engine == "duckdb" {
				flags.dataDir = t.TempDir()
			}
			env := startEmulatorWithFlags(t, flags)

			base := env.URL() + "/bigquery/v2/projects/" + projectID
			status, body := doJSON(t, http.MethodPost, base+"/datasets",
				[]byte(`{"datasetReference":{"projectId":"`+projectID+
					`","datasetId":"`+datasetID+`"},"location":"US"}`))
			if status != http.StatusOK {
				t.Fatalf("datasets.insert -> %d: %s", status, string(body))
			}

			tableSchema := `{
                "tableReference":{"projectId":"` + projectID +
				`","datasetId":"` + datasetID +
				`","tableId":"` + tableID + `"},
                "schema":{"fields":[
                    {"name":"id","type":"INT64","mode":"REQUIRED"},
                    {"name":"name","type":"STRING","mode":"NULLABLE"}
                ]}
            }`
			status, body = doJSON(t, http.MethodPost,
				base+"/datasets/"+datasetID+"/tables", []byte(tableSchema))
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
				t.Errorf("%s jobComplete = false, want true", f.label)
			}
			if run.TotalRows != "3" {
				t.Errorf("%s totalRows = %q, want %q",
					f.label, run.TotalRows, "3")
			}
			if len(run.Rows) != 3 {
				t.Fatalf("%s rows = %d, want 3: %+v",
					f.label, len(run.Rows), run.Rows)
			}
			results[f.label] = run
		})
	}

	duck, haveDuck := results["duckdb-arrow"]
	ref, haveRef := results["reference-impl"]
	if !haveDuck || !haveRef {
		t.Skip("one or both engine fixtures were skipped " +
			"(emulator_main not built); cannot compare wire shape")
	}

	// Compare Rows[] only; the surrounding envelope carries
	// per-job timestamps and job IDs that legitimately differ
	// between runs. The Rows slice is what the Storage Read API +
	// REST tabledata APIs both surface, so it is the contract
	// parity we care about.
	if !reflect.DeepEqual(duck.Rows, ref.Rows) {
		t.Errorf("wire shape divergence:\nduckdb-arrow rows = %+v\n"+
			"reference-impl rows = %+v", duck.Rows, ref.Rows)
	}

	// And the per-cell type-tag should match too: the wire shape
	// uses Go's interface{} for cell values and DeepEqual treats
	// (string)\"1\" and (float64)1.0 as different, so this also
	// pins the scalar encoding rule (INT64 -> decimal string).
	for i, r := range duck.Rows {
		for j, c := range r.F {
			gotKind := reflect.TypeOf(c.V)
			wantKind := reflect.TypeOf(ref.Rows[i].F[j].V)
			if gotKind != wantKind {
				t.Errorf("rows[%d].f[%d] type = %v, want %v "+
					"(duckdb cell = %#v, reference cell = %#v)",
					i, j, gotKind, wantKind, c.V, ref.Rows[i].F[j].V)
			}
		}
	}
}
