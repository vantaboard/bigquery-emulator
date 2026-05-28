//go:build integration

package e2e

import (
	"encoding/json"
	"math"
	"net/http"
	"reflect"
	"strconv"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// Phase 5m parity tests: drive the same query suite through both
// engines and confirm the wire shape lines up. The existing
// `TestQueryDuckDBArrowWireParity` pinned the single-query case; this
// suite extends coverage to a representative slice of the SHAPE_TRACKER
// dispositions (passthrough TableScan, FilterScan, OrderByScan,
// LimitOffsetScan, JOIN, AggregateScan, UNNEST/ArrowScan, AnalyticScan,
// plus shapes that fall back like SELECT 1, column-reordering project
// scans, and CASE expressions).
//
// Both fixtures share the same `tabledata.insertAll` seed payload and
// the same query text. The DuckDB fixture runs with
// `--on_unknown_fn=fallback` so transpiler-uncovered shapes route to
// the reference-impl engine through the FallbackEngine wrapper -- the
// parity invariant is "byte-identical wire shape", not "identical
// engine path", so a fallback that produces the same rows is a pass.
//
// FLOAT64 columns are compared with a numeric tolerance: the two
// engines parse and re-emit the same source bytes, but Phase 5l's
// Arrow path lowers `%.17g` while the reference-impl engine round-
// trips through `Value::SqlLiteral`, and the textual shapes can drift
// by a ULP on values like `0.1 + 0.2`. The tolerance keeps the
// invariant honest without rewriting the wire encoding for both
// engines mid-Phase-5.

// duckdbParityFloatToleranceULP caps the per-cell numeric drift we
// accept on FLOAT64 columns. A value of 1e-9 is well above the
// reference-impl/DuckDB Arrow round-trip drift in practice (single
// ULPs near the seed values) and well below the resolution of the
// test inputs.
const duckdbParityFloatToleranceULP = 1e-9

// duckdbParityCase is one query in the suite. `floatColumns` lists
// the zero-based column indices that must be compared with numeric
// tolerance (default: exact match).
type duckdbParityCase struct {
	name         string
	query        string
	floatColumns []int
}

// duckdbParityFixture is one engine configuration we drive the suite
// against. `engine == ""` leaves the flag off so the emulator's
// default (`reference_impl`) applies.
type duckdbParityFixture struct {
	label string
	flags emulatorFlags
}

// TestDuckDBParityQuerySuite drives a representative suite of read
// queries through both engines and asserts the resulting rows are
// the same. The DuckDB fixture runs with `--on_unknown_fn=fallback`
// so transpiler-uncovered shapes route to the reference-impl engine
// through the FallbackEngine wrapper; parity is enforced at the
// wire-shape boundary, not the engine path.
//
// Coverage targets the SHAPE_TRACKER dispositions plus a handful of
// known-fallback shapes so a future tightening (or regression) of
// either engine surfaces immediately. The seed data is intentionally
// small; the test is a parity smoke test, not a perf benchmark.
func TestDuckDBParityQuerySuite(t *testing.T) {
	const (
		projectID  = "proj-duckdb-parity"
		datasetID  = "ds_parity"
		usersTable = "users"
		eventsTbl  = "events"
	)

	// Both fixtures use `--storage=duckdb` so the underlying row
	// storage hands back natively-typed cells (Parquet enforces the
	// schema). The InMemoryStorage path returns every cell as
	// `Value::String` regardless of the column's declared type, and
	// the reference-impl evaluator's comparison machinery rejects a
	// `INT64 > STRING` shape with `Unsupported comparison function`
	// -- a wedge that breaks parity for queries like `WHERE id > 1`
	// without exercising any actual engine divergence. Each fixture
	// gets its own data_dir so the two emulator subprocesses do not
	// race on the same on-disk catalog.
	fixtures := []duckdbParityFixture{
		{
			label: "duckdb-fallback",
			flags: emulatorFlags{
				engine:      "duckdb",
				storage:     "duckdb",
				onUnknownFn: "fallback",
			},
		},
		{
			label: "reference-impl",
			flags: emulatorFlags{
				engine:  "reference_impl",
				storage: "duckdb",
			},
		},
	}

	cases := []duckdbParityCase{
		{
			name:  "select_constant",
			query: "SELECT 1",
		},
		{
			name:  "select_star_users_order_by",
			query: "SELECT * FROM " + datasetID + "." + usersTable + " ORDER BY id",
		},
		{
			name: "select_star_filter",
			query: "SELECT * FROM " + datasetID + "." + usersTable +
				" WHERE id > 1 ORDER BY id",
		},
		{
			name: "select_star_limit",
			query: "SELECT * FROM " + datasetID + "." + usersTable +
				" ORDER BY id LIMIT 2",
		},
		{
			name: "select_star_limit_offset",
			query: "SELECT * FROM " + datasetID + "." + usersTable +
				" ORDER BY id LIMIT 2 OFFSET 1",
		},
		{
			name:  "select_count_star",
			query: "SELECT COUNT(*) AS c FROM " + datasetID + "." + usersTable,
		},
		{
			name: "select_group_by",
			query: "SELECT kind, COUNT(*) AS c FROM " + datasetID + "." +
				eventsTbl + " GROUP BY kind ORDER BY kind",
		},
		{
			name: "select_join",
			query: "SELECT u.id, u.name, e.kind FROM " + datasetID + "." +
				usersTable + " AS u JOIN " + datasetID + "." + eventsTbl +
				" AS e ON u.id = e.user_id ORDER BY e.id",
		},
		{
			name:  "select_unnest_literal_array",
			query: "SELECT * FROM UNNEST([10, 20, 30]) AS x ORDER BY x",
		},
		{
			name: "select_struct_field_via_order_by",
			query: "SELECT * FROM UNNEST([STRUCT('a' AS name, 1 AS score), " +
				"STRUCT('b' AS name, 2 AS score)]) AS s ORDER BY s.score",
		},
		{
			name: "select_window_row_number",
			query: "SELECT *, ROW_NUMBER() OVER (ORDER BY id) AS rn FROM " +
				datasetID + "." + usersTable + " ORDER BY id",
		},
		{
			name: "select_case_expression",
			query: "SELECT id, CASE WHEN id = 1 THEN 'first' ELSE 'rest' END" +
				" AS label FROM " + datasetID + "." + usersTable + " ORDER BY id",
		},
		{
			name: "select_avg_score",
			query: "SELECT AVG(score) AS avg_score FROM " + datasetID +
				"." + usersTable,
			floatColumns: []int{0},
		},
	}

	results := make(map[string][]bqtypes.QueryResponse)

	for _, fx := range fixtures {
		fx := fx
		t.Run("seed_"+fx.label, func(t *testing.T) {
			flags := fx.flags
			// `--storage=duckdb` resolves `--data_dir` to
			// `$HOME/.bigquery-emulator` by default, which persists
			// across runs and makes the second fixture see catalog
			// rows from the first. Always hand it a hermetic
			// per-subtest tempdir.
			if flags.storage == "duckdb" {
				flags.dataDir = t.TempDir()
			}
			env := startEmulatorWithFlags(t, flags)
			seedDuckDBParityFixture(t, env, projectID, datasetID,
				usersTable, eventsTbl)

			runs := make([]bqtypes.QueryResponse, len(cases))
			for i, c := range cases {
				runs[i] = runDuckDBParityQuery(t, env, projectID, c.query)
			}
			results[fx.label] = runs
		})
	}

	duck, haveDuck := results["duckdb-fallback"]
	ref, haveRef := results["reference-impl"]
	if !haveDuck || !haveRef {
		t.Skip("one or both engine fixtures were skipped " +
			"(emulator_main not built); cannot compare wire shape")
	}

	for i, c := range cases {
		c := c
		i := i
		t.Run("compare_"+c.name, func(t *testing.T) {
			compareDuckDBParity(t, c, duck[i], ref[i])
		})
	}
}

// TestDuckDBParityAnalyticsNoFallback drives a small slice of
// analytics-shaped queries through the DuckDB engine WITHOUT the
// fallback wrapper: `--on_unknown_fn=unimplemented` (the default),
// so the gateway surfaces UNIMPLEMENTED as HTTP 501 if the engine
// has to fall back. A 200 response with the expected row shape
// proves the DuckDB engine handled the query end-to-end via the
// transpiler + libduckdb path -- exactly the invariant Phase 5m
// pins for the SHAPE_TRACKER `done` rows that cover ArrayScan
// (UNNEST), GetStructField, and AnalyticScan (window functions).
func TestDuckDBParityAnalyticsNoFallback(t *testing.T) {
	env := startEmulatorWithFlags(t, emulatorFlags{
		engine:  "duckdb",
		storage: "duckdb",
		dataDir: t.TempDir(),
	})

	const (
		projectID  = "proj-duckdb-analytics"
		datasetID  = "ds_analytics"
		usersTable = "users"
	)

	// Probe with `SELECT * FROM ds.t` after seeding so a build
	// without ExecuteQuery (the CMake-only checkout) skips cleanly.
	// `SELECT 1` is not a viable probe under
	// `--on_unknown_fn=unimplemented` because the DuckDB engine
	// returns UNIMPLEMENTED for the wrapping computed-column
	// ProjectScan; the probe needs a shape the transpiler covers.
	seedDuckDBAnalyticsFixture(t, env, projectID, datasetID, usersTable)

	probeQuery := "SELECT * FROM " + datasetID + "." + usersTable
	probeStatus, probeBody := doJSON(t, http.MethodPost,
		env.URL()+"/bigquery/v2/projects/"+projectID+"/queries",
		[]byte(`{"query":"`+probeQuery+`","useLegacySql":false}`))
	if probeStatus == http.StatusNotImplemented {
		t.Skipf("emulator_main was built without an ExecuteQuery "+
			"implementation (returns 501 notImplemented). "+
			"Probe body: %s", string(probeBody))
	}
	if probeStatus != http.StatusOK {
		t.Fatalf("probe SELECT * -> %d: %s", probeStatus, string(probeBody))
	}

	// Each query below MUST return HTTP 200 with the expected row
	// count: a 501 here means the DuckDB engine reported
	// UNIMPLEMENTED and `--on_unknown_fn=unimplemented` surfaced it
	// straight to the client (no fallback to reference-impl). That
	// is the regression we want to catch.
	cases := []struct {
		name      string
		query     string
		wantRows  int
		wantTotal string
	}{
		{
			name:      "unnest_int_array_order_by",
			query:     "SELECT * FROM UNNEST([10, 20, 30]) AS x ORDER BY x",
			wantRows:  3,
			wantTotal: "3",
		},
		{
			// `ROW_NUMBER() OVER (...)` exercises the AnalyticScan
			// emit (`SHAPE_TRACKER.md` row "ResolvedAnalyticScan ->
			// done (subset)"). The wrapping ProjectScan is
			// pass-through (its column_list matches the inner
			// AnalyticScan's column_list one-for-one because the
			// `rn` synthetic column is produced by the
			// AnalyticScan, not by an `expr_list` entry on the
			// ProjectScan), so `StripPassThroughProjectScans` peels
			// it off and DuckDB executes the lowered SQL. Wrapping
			// `ORDER BY id` keeps the row order stable across runs.
			name: "window_row_number_over_order_by",
			query: "SELECT *, ROW_NUMBER() OVER (ORDER BY id) AS rn FROM " +
				datasetID + "." + usersTable + " ORDER BY id",
			wantRows:  3,
			wantTotal: "3",
		},
		// STRUCT field access is exercised through
		// `TestDuckDBParityQuerySuite/compare_select_struct_field_via_order_by`
		// rather than here. The shape that lands a `GetStructField`
		// in an emitted scan needs an intermediate computed-column
		// ProjectScan ("ORDER BY s.score" lifts the field access
		// onto a synthetic column), which the engine cannot strip
		// today -- the transpiler-emit-project-stmt plan flips that
		// row. The parity suite covers the wire-shape contract via
		// fallback to the reference-impl evaluator; promoting this
		// to a no-fallback test waits on the project-stmt emit.
	}

	for _, tc := range cases {
		tc := tc
		t.Run(tc.name, func(t *testing.T) {
			body := []byte(`{"query":"` + tc.query + `","useLegacySql":false}`)
			status, raw := doJSON(t, http.MethodPost,
				env.URL()+"/bigquery/v2/projects/"+projectID+"/queries", body)
			if status == http.StatusNotImplemented {
				t.Fatalf("query %q routed to fallback (HTTP 501 with "+
					"--on_unknown_fn=unimplemented); the DuckDB engine "+
					"should handle this shape end-to-end. Body: %s",
					tc.query, string(raw))
			}
			if status != http.StatusOK {
				t.Fatalf("query %q -> %d: %s", tc.query, status, string(raw))
			}
			var run bqtypes.QueryResponse
			if err := json.Unmarshal(raw, &run); err != nil {
				t.Fatalf("decode response: %v (body=%s)", err, string(raw))
			}
			if !run.JobComplete {
				t.Errorf("jobComplete = false, want true")
			}
			if run.TotalRows != tc.wantTotal {
				t.Errorf("totalRows = %q, want %q",
					run.TotalRows, tc.wantTotal)
			}
			if len(run.Rows) != tc.wantRows {
				t.Fatalf("rows = %d, want %d: %+v",
					len(run.Rows), tc.wantRows, run.Rows)
			}
		})
	}
}

// seedDuckDBParityFixture creates the `users` and `events` tables
// shared by `TestDuckDBParityQuerySuite`. The schema is intentionally
// kept simple and STRUCT-free at the table level: STRUCT data going
// through `tabledata.insertAll` lowers via Go's `range` over a
// `map[string]interface{}` (see `gateway/handlers/tabledata.go` ::
// `jsonToCell`), so the field order in the stored cells is
// non-deterministic and parity between two independent fixtures would
// flake. STRUCT shapes are exercised via SQL literals in the queries
// instead.
func seedDuckDBParityFixture(t *testing.T, env *emulatorEnv,
	projectID, datasetID, usersTable, eventsTbl string) {
	t.Helper()
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	usersSchema := `{
        "tableReference":{"projectId":"` + projectID +
		`","datasetId":"` + datasetID +
		`","tableId":"` + usersTable + `"},
        "schema":{"fields":[
            {"name":"id","type":"INT64","mode":"REQUIRED"},
            {"name":"name","type":"STRING","mode":"NULLABLE"},
            {"name":"score","type":"FLOAT64","mode":"NULLABLE"}
        ]}
    }`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables", []byte(usersSchema))
	if status != http.StatusOK {
		t.Fatalf("tables.insert(users) -> %d: %s", status, string(body))
	}

	eventsSchema := `{
        "tableReference":{"projectId":"` + projectID +
		`","datasetId":"` + datasetID +
		`","tableId":"` + eventsTbl + `"},
        "schema":{"fields":[
            {"name":"id","type":"INT64","mode":"REQUIRED"},
            {"name":"user_id","type":"INT64","mode":"REQUIRED"},
            {"name":"kind","type":"STRING","mode":"REQUIRED"}
        ]}
    }`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables", []byte(eventsSchema))
	if status != http.StatusOK {
		t.Fatalf("tables.insert(events) -> %d: %s", status, string(body))
	}

	usersRows := `{
        "rows":[
            {"insertId":"u1","json":{"id":1,"name":"ada","score":0.5}},
            {"insertId":"u2","json":{"id":2,"name":"linus","score":1.5}},
            {"insertId":"u3","json":{"id":3,"name":"grace","score":2.5}}
        ]
    }`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables/"+usersTable+"/insertAll",
		[]byte(usersRows))
	if status != http.StatusOK {
		t.Fatalf("insertAll(users) -> %d: %s", status, string(body))
	}

	eventsRows := `{
        "rows":[
            {"insertId":"e1","json":{"id":10,"user_id":1,"kind":"login"}},
            {"insertId":"e2","json":{"id":11,"user_id":1,"kind":"click"}},
            {"insertId":"e3","json":{"id":12,"user_id":2,"kind":"login"}},
            {"insertId":"e4","json":{"id":13,"user_id":3,"kind":"click"}}
        ]
    }`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables/"+eventsTbl+"/insertAll",
		[]byte(eventsRows))
	if status != http.StatusOK {
		t.Fatalf("insertAll(events) -> %d: %s", status, string(body))
	}
}

// seedDuckDBAnalyticsFixture creates the `users` table used by
// `TestDuckDBParityAnalyticsNoFallback`. Schema and contents mirror
// the parity suite's `users` table so the analytics queries hit
// known shape boundaries (passthrough ProjectScan with AnalyticScan
// inside).
func seedDuckDBAnalyticsFixture(t *testing.T, env *emulatorEnv,
	projectID, datasetID, usersTable string) {
	t.Helper()
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
		`","tableId":"` + usersTable + `"},
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

	rows := `{
        "rows":[
            {"insertId":"a","json":{"id":1,"name":"ada"}},
            {"insertId":"b","json":{"id":2,"name":"linus"}},
            {"insertId":"c","json":{"id":3,"name":"grace"}}
        ]
    }`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables/"+usersTable+"/insertAll",
		[]byte(rows))
	if status != http.StatusOK {
		t.Fatalf("insertAll -> %d: %s", status, string(body))
	}
}

// runDuckDBParityQuery POSTs `query` to `jobs.query` and returns the
// decoded `QueryResponse`. A non-200 fails the test loudly so the
// caller does not have to repeat the boilerplate per case.
func runDuckDBParityQuery(t *testing.T, env *emulatorEnv,
	projectID, query string) bqtypes.QueryResponse {
	t.Helper()
	body := []byte(`{"query":"` + query + `","useLegacySql":false}`)
	status, raw := doJSON(t, http.MethodPost,
		env.URL()+"/bigquery/v2/projects/"+projectID+"/queries", body)
	if status != http.StatusOK {
		t.Fatalf("query %q -> %d: %s", query, status, string(raw))
	}
	var run bqtypes.QueryResponse
	if err := json.Unmarshal(raw, &run); err != nil {
		t.Fatalf("decode response for %q: %v (body=%s)",
			query, err, string(raw))
	}
	if !run.JobComplete {
		t.Errorf("query %q jobComplete = false, want true", query)
	}
	return run
}

// compareDuckDBParity asserts the two engine fixtures produced
// equivalent rows for the same query. Cells in `floatColumns` are
// compared with `duckdbParityFloatToleranceULP`; everything else is
// compared with `reflect.DeepEqual` so a stray difference in scalar
// formatting or nesting jumps out immediately.
func compareDuckDBParity(t *testing.T, c duckdbParityCase,
	duck, ref bqtypes.QueryResponse) {
	t.Helper()

	if duck.TotalRows != ref.TotalRows {
		t.Errorf("totalRows differ: duckdb=%q reference=%q",
			duck.TotalRows, ref.TotalRows)
	}
	if len(duck.Rows) != len(ref.Rows) {
		t.Fatalf("row count differs: duckdb=%d reference=%d "+
			"(duck=%+v ref=%+v)",
			len(duck.Rows), len(ref.Rows), duck.Rows, ref.Rows)
	}

	floatCols := make(map[int]bool, len(c.floatColumns))
	for _, idx := range c.floatColumns {
		floatCols[idx] = true
	}

	for i, dr := range duck.Rows {
		rr := ref.Rows[i]
		if len(dr.F) != len(rr.F) {
			t.Errorf("rows[%d] cell count differs: duckdb=%d reference=%d",
				i, len(dr.F), len(rr.F))
			continue
		}
		for j := range dr.F {
			if floatCols[j] {
				if !floatCellsApproxEqual(dr.F[j], rr.F[j]) {
					t.Errorf("rows[%d].f[%d] FLOAT64 cells differ "+
						"beyond tolerance: duckdb=%#v reference=%#v",
						i, j, dr.F[j].V, rr.F[j].V)
				}
				continue
			}
			if !reflect.DeepEqual(dr.F[j], rr.F[j]) {
				t.Errorf("rows[%d].f[%d] differs: duckdb=%#v reference=%#v",
					i, j, dr.F[j].V, rr.F[j].V)
			}
		}
	}
}

// floatCellsApproxEqual returns true when `a` and `b` are both NULL
// or both decode to floats within `duckdbParityFloatToleranceULP` of
// each other. The wire shape for FLOAT64 is a decimal string (see
// docs/REST_API.md "Type wire encoding"), so both cells unmarshal
// to `string` -- we parse with `strconv.ParseFloat` and compare in
// floating-point space.
func floatCellsApproxEqual(a, b bqtypes.Cell) bool {
	if a.V == nil && b.V == nil {
		return true
	}
	if a.V == nil || b.V == nil {
		return false
	}
	as, aok := a.V.(string)
	bs, bok := b.V.(string)
	if !aok || !bok {
		return reflect.DeepEqual(a, b)
	}
	if as == bs {
		return true
	}
	af, err := strconv.ParseFloat(as, 64)
	if err != nil {
		return false
	}
	bf, err := strconv.ParseFloat(bs, 64)
	if err != nil {
		return false
	}
	if math.IsNaN(af) && math.IsNaN(bf) {
		return true
	}
	if math.IsInf(af, 0) || math.IsInf(bf, 0) {
		return af == bf
	}
	diff := math.Abs(af - bf)
	if diff <= duckdbParityFloatToleranceULP {
		return true
	}
	mag := math.Max(math.Abs(af), math.Abs(bf))
	return diff <= duckdbParityFloatToleranceULP*mag
}
