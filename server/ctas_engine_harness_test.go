// CTAS engine harness: compare async job cost for two emulator paths:
//
//  1. Materialize: CREATE TABLE ... AS in the query text, no [bigquery.QueryConfig].Dst on the job.
//     Uses [contentdata.Repository.Query] (row materialization + destination write / AddTableData).
//
//  2. In-place: same CTAS SQL plus Dst set to the table being created.
//     Uses [contentdata.Repository.QueryCTASInPlace] (googlesqlengine Exec). At debug log level, look for
//     "content query CTAS in-place (Exec)" in internal/contentdata/ctas.go.
//
// A plain SELECT with only Dst and no CREATE TABLE in the query string does NOT use the in-place path;
// [contentdata.IsCTASQuery] must see a CREATE TABLE ... AS statement.
//
// Tunables (local profiling):
//
//	BQ_HARNESS_OUTER / BQ_HARNESS_INNER — cross-join sizes (Cartesian product = outer*inner rows counted).
//	BQ_EMULATOR_CONN_METRICS=1 — pool metrics (see internal/connection).
//	BQ_HARNESS_MEM=1 — log runtime.MemStats delta after job Wait (cost test only).
//	Very long opt-in: BQ_TORTOISE=1 — TestTortoise_LongCTAS_InPlace; BQ_TORTOISE=1 and BQ_TORTOISE_RICH=1 —
//	TestTortoise_LongCTAS_Rich. Fixture SQL: testfixtures/long_ctas_tortoise.sql, testfixtures/long_ctas_tortoise_rich.sql.
//
// Run smoke + cost tests:
//
//	go test -tags "$GOOGLESQL_BUILD_TAGS" -run TestHarness_CTAS -v ./server/...
//
// Benchmarks (skipped under -short):
//
//	go test -tags "$GOOGLESQL_BUILD_TAGS" -bench BenchmarkCTAS_JobWait -benchtime 1s ./server/...
//	go test -tags "$GOOGLESQL_BUILD_TAGS" -bench BenchmarkCTAS_Rich_JobWait -benchtime 1s ./server/...
package server_test

import (
	"context"
	"fmt"
	"os"
	"runtime"
	"strconv"
	"sync/atomic"
	"testing"
	"time"

	"cloud.google.com/go/bigquery"
	"github.com/vantaboard/bigquery-emulator/internal/connection"
	"github.com/vantaboard/bigquery-emulator/server"
	"github.com/vantaboard/bigquery-emulator/types"
	"google.golang.org/api/iterator"
	"google.golang.org/api/option"
)

const (
	harnessProjectID = "harnessproj"
	harnessDatasetID = "harnessds"
)

func harnessDimsFromEnv(defaultOuter, defaultInner int) (outer, inner int) {
	outer, inner = defaultOuter, defaultInner
	if s := os.Getenv("BQ_HARNESS_OUTER"); s != "" {
		if v, err := strconv.Atoi(s); err == nil && v > 0 {
			outer = v
		}
	}
	if s := os.Getenv("BQ_HARNESS_INNER"); s != "" {
		if v, err := strconv.Atoi(s); err == nil && v > 0 {
			inner = v
		}
	}
	return outer, inner
}

func harnessCTASSQL(datasetID, tableID string, outer, inner int) string {
	return fmt.Sprintf(
		"CREATE TABLE `%s.%s` AS "+
			"SELECT COUNT(*) AS c FROM UNNEST(GENERATE_ARRAY(1, %d)) a "+
			"CROSS JOIN UNNEST(GENERATE_ARRAY(1, %d)) b",
		datasetID, tableID, outer, inner,
	)
}

// harnessTortoiseRichCTASSQL is a CTAS with multiple CTEs (including an unused CTE, GROUP BY,
// HAVING, window+filter via subquery (QUALIFY is avoided: it prevents [contentdata.IsCTASQuery] on the
// outer CreateTable+Query in current googlesql), and pivot-style / unpivot-style branches via CASE+GROUP
// and UNION) around the same tunable cross-join "heavy" stage as [harnessCTASSQL]. The final row has
// column c = outer*inner.
//
// The "heavy" CTE is the same cardinality stress as the minimal tortoise. GROUP BY / HAVING / window
// stages use a *small* UNNEST bucket so the rich harness does not add a second full scan + hash-aggregate
// over millions of rows (which dominated wall time and hid other features). A BigQuery-faithful GROUP BY
// over the large heavy table is documented in [../testfixtures/long_ctas_tortoise_rich.sql] for manual
// / engine work.
func harnessTortoiseRichCTASSQL(datasetID, tableID string, outer, inner int) string {
	return fmt.Sprintf(
		"CREATE TABLE `%s.%s` AS "+
			"WITH dead_cte AS ( "+
			"SELECT 999 AS never_used, 'unused' AS marker "+
			"), heavy AS ( "+
			"SELECT a, b FROM UNNEST(GENERATE_ARRAY(1, %d)) a CROSS JOIN UNNEST(GENERATE_ARRAY(1, %d)) b "+
			"), bucketed AS ( "+
			"SELECT MOD(x, 3) AS bucket, COUNT(*) AS n FROM UNNEST(GENERATE_ARRAY(1, 99)) x GROUP BY 1 HAVING COUNT(*) > 0 "+
			"), bucket_qualified AS ( "+
			"SELECT bucket, n FROM ( "+
			"SELECT bucket, n, ROW_NUMBER() OVER (ORDER BY n DESC) AS rn FROM bucketed "+
			") z WHERE z.rn <= 3 "+
			"), q_sales AS ( "+
			"SELECT 'item' AS product, 10 AS s, 'Q1' AS quarter UNION ALL SELECT 'item', 20, 'Q2' "+
			"), pivotish AS ( "+
			"SELECT product, SUM(CASE quarter WHEN 'Q1' THEN s END) AS q1, "+
			"SUM(CASE quarter WHEN 'Q2' THEN s END) AS q2 FROM q_sales GROUP BY product "+
			"), unpivish AS ( "+
			"SELECT v AS val, t AS qname FROM ( "+
			"SELECT q1 AS v, 'Q1' AS t FROM pivotish UNION ALL SELECT q2, 'Q2' FROM pivotish "+
			") "+
			") "+
			"SELECT (SELECT COUNT(*) AS cnt FROM heavy) + "+
			"0 * COALESCE((SELECT MAX(n) FROM bucket_qualified), 0) + "+
			"0 * COALESCE((SELECT MAX(val) FROM unpivish), 0) AS c",
		datasetID, tableID, outer, inner,
	)
}

// harnessManyRowCTASSQL returns a CTAS that materializes n output rows (one column "c") for stress
// that differs from the aggregate cross-join harness.
func harnessManyRowCTASSQL(datasetID, tableID string, n int) string {
	return fmt.Sprintf(
		"CREATE TABLE `%s.%s` AS "+
			"SELECT x AS c FROM UNNEST(GENERATE_ARRAY(1, %d)) x",
		datasetID, tableID, n,
	)
}

func harnessReadTableCount(ctx context.Context, client *bigquery.Client, datasetID, tableID string) (int64, error) {
	q := client.Query(fmt.Sprintf("SELECT COUNT(*) AS cnt FROM `%s.%s`", datasetID, tableID))
	it, err := q.Read(ctx)
	if err != nil {
		return 0, err
	}
	var row []bigquery.Value
	if err := it.Next(&row); err != nil {
		return 0, err
	}
	if err := it.Next(&row); err != iterator.Done {
		return 0, fmt.Errorf("expected one row for COUNT(*)")
	}
	switch v := row[0].(type) {
	case int64:
		return v, nil
	case int:
		return int64(v), nil
	default:
		return 0, fmt.Errorf("unexpected count type %T", row[0])
	}
}

func harnessRunQuery(
	ctx context.Context,
	endpoint, projectID, sql string,
	dst *bigquery.Table,
) (*bigquery.Job, error) {
	client, err := bigquery.NewClient(ctx, projectID, option.WithEndpoint(endpoint), option.WithoutAuthentication())
	if err != nil {
		return nil, err
	}
	defer client.Close()
	q := client.Query(sql)
	if dst != nil {
		q.QueryConfig.Dst = dst
	}
	return q.Run(ctx)
}

func harnessReadCountC(ctx context.Context, client *bigquery.Client, datasetID, tableID string) (int64, error) {
	q := client.Query(fmt.Sprintf("SELECT c FROM `%s.%s`", datasetID, tableID))
	it, err := q.Read(ctx)
	if err != nil {
		return 0, err
	}
	var row []bigquery.Value
	if err := it.Next(&row); err != nil {
		return 0, err
	}
	if err := it.Next(&row); err != iterator.Done {
		return 0, fmt.Errorf("expected one row")
	}
	switch v := row[0].(type) {
	case int64:
		return v, nil
	case int:
		return int64(v), nil
	default:
		return 0, fmt.Errorf("unexpected c type %T", row[0])
	}
}

func TestHarness_CTAS_JobWait_Smoke(t *testing.T) {
	ctx := context.Background()
	// Fixed size so BQ_HARNESS_* (used by cost test) do not make smoke slow in CI.
	const outer, inner = 28, 28
	want := int64(outer * inner)

	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		t.Fatal(err)
	}
	project := types.NewProject(harnessProjectID, types.NewDataset(harnessDatasetID))
	if err := bqServer.Load(server.StructSource(project)); err != nil {
		t.Fatal(err)
	}
	ts := bqServer.TestServer()
	client, err := bigquery.NewClient(ctx, harnessProjectID, option.WithEndpoint(ts.URL), option.WithoutAuthentication())
	if err != nil {
		ts.Close()
		_ = bqServer.Stop(ctx)
		t.Fatal(err)
	}
	defer client.Close()

	t.Cleanup(func() {
		ts.Close()
		_ = bqServer.Stop(context.Background())
	})

	t.Run("materialize_no_api_destination", func(t *testing.T) {
		const tableID = "harness_smoke_m"
		sql := harnessCTASSQL(harnessDatasetID, tableID, outer, inner)
		job, err := harnessRunQuery(ctx, ts.URL, harnessProjectID, sql, nil)
		if err != nil {
			t.Fatal(err)
		}
		if _, err := job.Wait(ctx); err != nil {
			t.Fatal(err)
		}
		got, err := harnessReadCountC(ctx, client, harnessDatasetID, tableID)
		if err != nil {
			t.Fatal(err)
		}
		if got != want {
			t.Fatalf("c: got %d want %d (outer=%d inner=%d)", got, want, outer, inner)
		}
	})

	t.Run("in_place_with_api_destination", func(t *testing.T) {
		const tableID = "harness_smoke_p"
		sql := harnessCTASSQL(harnessDatasetID, tableID, outer, inner)
		dst := &bigquery.Table{
			ProjectID: harnessProjectID,
			DatasetID: harnessDatasetID,
			TableID:   tableID,
		}
		job, err := harnessRunQuery(ctx, ts.URL, harnessProjectID, sql, dst)
		if err != nil {
			t.Fatal(err)
		}
		if _, err := job.Wait(ctx); err != nil {
			t.Fatal(err)
		}
		got, err := harnessReadCountC(ctx, client, harnessDatasetID, tableID)
		if err != nil {
			t.Fatal(err)
		}
		if got != want {
			t.Fatalf("c: got %d want %d (outer=%d inner=%d)", got, want, outer, inner)
		}
	})
}

// TestHarness_CTAS_Rich_JobWait_Smoke is the "tortoise rich" query ([harnessTortoiseRichCTASSQL]) at a
// tiny size so CI exercises multi-CTEs, window+filter, pivot-style and in-place and materialize paths.
func TestHarness_CTAS_Rich_JobWait_Smoke(t *testing.T) {
	ctx := context.Background()
	const outer, inner = 5, 5
	want := int64(outer * inner)

	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		t.Fatal(err)
	}
	project := types.NewProject(harnessProjectID, types.NewDataset(harnessDatasetID))
	if err := bqServer.Load(server.StructSource(project)); err != nil {
		t.Fatal(err)
	}
	ts := bqServer.TestServer()
	client, err := bigquery.NewClient(ctx, harnessProjectID, option.WithEndpoint(ts.URL), option.WithoutAuthentication())
	if err != nil {
		ts.Close()
		_ = bqServer.Stop(ctx)
		t.Fatal(err)
	}
	defer client.Close()
	t.Cleanup(func() {
		ts.Close()
		_ = bqServer.Stop(context.Background())
	})

	t.Run("materialize_no_api_destination", func(t *testing.T) {
		const tableID = "harness_rich_m"
		sql := harnessTortoiseRichCTASSQL(harnessDatasetID, tableID, outer, inner)
		job, err := harnessRunQuery(ctx, ts.URL, harnessProjectID, sql, nil)
		if err != nil {
			t.Fatal(err)
		}
		if _, err := job.Wait(ctx); err != nil {
			t.Fatal(err)
		}
		got, err := harnessReadCountC(ctx, client, harnessDatasetID, tableID)
		if err != nil {
			t.Fatal(err)
		}
		if got != want {
			t.Fatalf("c: got %d want %d (rich CTAS materialize)", got, want)
		}
	})
	t.Run("in_place_with_api_destination", func(t *testing.T) {
		const tableID = "harness_rich_p"
		sql := harnessTortoiseRichCTASSQL(harnessDatasetID, tableID, outer, inner)
		dst := &bigquery.Table{
			ProjectID: harnessProjectID,
			DatasetID: harnessDatasetID,
			TableID:   tableID,
		}
		job, err := harnessRunQuery(ctx, ts.URL, harnessProjectID, sql, dst)
		if err != nil {
			t.Fatal(err)
		}
		if _, err := job.Wait(ctx); err != nil {
			t.Fatal(err)
		}
		got, err := harnessReadCountC(ctx, client, harnessDatasetID, tableID)
		if err != nil {
			t.Fatal(err)
		}
		if got != want {
			t.Fatalf("c: got %d want %d (rich CTAS in-place)", got, want)
		}
	})
}

// TestHarness_CTAS_JobWait_Smoke_ManyRows writes a multi-row result (not a single aggregate row).
func TestHarness_CTAS_JobWait_Smoke_ManyRows(t *testing.T) {
	ctx := context.Background()
	const n = 200
	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		t.Fatal(err)
	}
	project := types.NewProject(harnessProjectID, types.NewDataset(harnessDatasetID))
	if err := bqServer.Load(server.StructSource(project)); err != nil {
		t.Fatal(err)
	}
	ts := bqServer.TestServer()
	client, err := bigquery.NewClient(ctx, harnessProjectID, option.WithEndpoint(ts.URL), option.WithoutAuthentication())
	if err != nil {
		ts.Close()
		_ = bqServer.Stop(ctx)
		t.Fatal(err)
	}
	defer client.Close()

	t.Cleanup(func() {
		ts.Close()
		_ = bqServer.Stop(context.Background())
	})

	t.Run("materialize", func(t *testing.T) {
		const tableID = "harness_smoke_rows_m"
		sql := harnessManyRowCTASSQL(harnessDatasetID, tableID, n)
		job, err := harnessRunQuery(ctx, ts.URL, harnessProjectID, sql, nil)
		if err != nil {
			t.Fatal(err)
		}
		if _, err := job.Wait(ctx); err != nil {
			t.Fatal(err)
		}
		got, err := harnessReadTableCount(ctx, client, harnessDatasetID, tableID)
		if err != nil {
			t.Fatal(err)
		}
		if got != int64(n) {
			t.Fatalf("row count: got %d want %d", got, n)
		}
	})
	t.Run("in_place", func(t *testing.T) {
		const tableID = "harness_smoke_rows_p"
		sql := harnessManyRowCTASSQL(harnessDatasetID, tableID, n)
		dst := &bigquery.Table{
			ProjectID: harnessProjectID,
			DatasetID: harnessDatasetID,
			TableID:   tableID,
		}
		job, err := harnessRunQuery(ctx, ts.URL, harnessProjectID, sql, dst)
		if err != nil {
			t.Fatal(err)
		}
		if _, err := job.Wait(ctx); err != nil {
			t.Fatal(err)
		}
		got, err := harnessReadTableCount(ctx, client, harnessDatasetID, tableID)
		if err != nil {
			t.Fatal(err)
		}
		if got != int64(n) {
			t.Fatalf("row count: got %d want %d", got, n)
		}
	})
}

func TestHarness_CTAS_JobWait_Cost(t *testing.T) {
	if testing.Short() {
		t.Skip("skipping cost harness in -short; run full tests or set BQ_HARNESS_OUTER/INNER for local tuning")
	}

	ctx := context.Background()
	t.Setenv("BQ_EMULATOR_CONN_METRICS", "1")

	outer, inner := harnessDimsFromEnv(200, 20)
	memOn := os.Getenv("BQ_HARNESS_MEM") == "1"
	want := int64(outer * inner)

	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		t.Fatal(err)
	}
	project := types.NewProject(harnessProjectID, types.NewDataset(harnessDatasetID))
	if err := bqServer.Load(server.StructSource(project)); err != nil {
		t.Fatal(err)
	}
	ts := bqServer.TestServer()
	client, err := bigquery.NewClient(ctx, harnessProjectID, option.WithEndpoint(ts.URL), option.WithoutAuthentication())
	if err != nil {
		ts.Close()
		_ = bqServer.Stop(ctx)
		t.Fatal(err)
	}
	defer client.Close()

	t.Cleanup(func() {
		ts.Close()
		_ = bqServer.Stop(context.Background())
	})

	run := func(t *testing.T, name, tableID string, dst *bigquery.Table) {
		t.Helper()
		sql := harnessCTASSQL(harnessDatasetID, tableID, outer, inner)

		var ms0, ms1 runtime.MemStats
		if memOn {
			runtime.GC()
			runtime.ReadMemStats(&ms0)
		}

		t0 := time.Now()
		job, err := harnessRunQuery(ctx, ts.URL, harnessProjectID, sql, dst)
		if err != nil {
			t.Fatalf("%s: Run: %v", name, err)
		}
		if _, err := job.Wait(ctx); err != nil {
			t.Fatalf("%s: Wait: %v", name, err)
		}
		elapsed := time.Since(t0)
		t.Logf("%s: job_wait=%s outer=%d inner=%d product=%d", name, elapsed, outer, inner, outer*inner)

		if memOn {
			runtime.ReadMemStats(&ms1)
			t.Logf("%s: mem_alloc_delta=%d bytes_total_alloc_delta=%d heap_sys=%d",
				name, int64(ms1.Alloc-ms0.Alloc), int64(ms1.TotalAlloc-ms0.TotalAlloc), int64(ms1.HeapSys))
		}

		got, err := harnessReadCountC(ctx, client, harnessDatasetID, tableID)
		if err != nil {
			t.Fatalf("%s: read c: %v", name, err)
		}
		if got != want {
			t.Fatalf("%s: c: got %d want %d", name, got, want)
		}
	}

	t.Run("materialize_no_api_destination", func(t *testing.T) {
		connection.ResetConnMetrics()
		run(t, "materialize", "harness_cost_m", nil)
		m := connection.SnapshotConnMetrics()
		if m.AcquireCount > 0 {
			avgWait := time.Duration(uint64(m.TotalAcquireWait) / m.AcquireCount)
			t.Logf("materialize: conn acquire_wait_total=%s fn_hold_total=%s acquires=%d acquire_canceled=%d avg_acquire_wait=%s",
				m.TotalAcquireWait, m.TotalFnHold, m.AcquireCount, m.AcquireCanceled, avgWait)
		}
	})
	t.Run("in_place_with_api_destination", func(t *testing.T) {
		connection.ResetConnMetrics()
		run(t, "in_place", "harness_cost_p", &bigquery.Table{
			ProjectID: harnessProjectID,
			DatasetID: harnessDatasetID,
			TableID:   "harness_cost_p",
		})
		m := connection.SnapshotConnMetrics()
		if m.AcquireCount > 0 {
			avgWait := time.Duration(uint64(m.TotalAcquireWait) / m.AcquireCount)
			t.Logf("in_place: conn acquire_wait_total=%s fn_hold_total=%s acquires=%d acquire_canceled=%d avg_acquire_wait=%s",
				m.TotalAcquireWait, m.TotalFnHold, m.AcquireCount, m.AcquireCanceled, avgWait)
		}
	})
}

var benchCTASTableSeq atomic.Uint64

func BenchmarkCTAS_JobWait(b *testing.B) {
	if testing.Short() {
		b.Skip("skipping in -short")
	}
	ctx := context.Background()
	outer, inner := harnessDimsFromEnv(120, 15)
	want := int64(outer * inner)

	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		b.Fatal(err)
	}
	project := types.NewProject(harnessProjectID, types.NewDataset(harnessDatasetID))
	if err := bqServer.Load(server.StructSource(project)); err != nil {
		b.Fatal(err)
	}
	ts := bqServer.TestServer()
	client, err := bigquery.NewClient(ctx, harnessProjectID, option.WithEndpoint(ts.URL), option.WithoutAuthentication())
	if err != nil {
		ts.Close()
		_ = bqServer.Stop(ctx)
		b.Fatal(err)
	}
	defer client.Close()

	b.Cleanup(func() {
		ts.Close()
		_ = bqServer.Stop(context.Background())
	})

	b.Run("materialize_no_api_destination", func(b *testing.B) {
		for range b.N {
			b.StopTimer()
			n := benchCTASTableSeq.Add(1)
			tableID := fmt.Sprintf("hb_m_%d", n)
			sql := harnessCTASSQL(harnessDatasetID, tableID, outer, inner)
			b.StartTimer()

			job, err := harnessRunQuery(ctx, ts.URL, harnessProjectID, sql, nil)
			if err != nil {
				b.Fatal(err)
			}
			if _, err := job.Wait(ctx); err != nil {
				b.Fatal(err)
			}
			b.StopTimer()

			got, err := harnessReadCountC(ctx, client, harnessDatasetID, tableID)
			if err != nil {
				b.Fatal(err)
			}
			if got != want {
				b.Fatalf("c: got %d want %d", got, want)
			}
		}
	})

	b.Run("in_place_with_api_destination", func(b *testing.B) {
		for range b.N {
			b.StopTimer()
			n := benchCTASTableSeq.Add(1)
			tableID := fmt.Sprintf("hb_p_%d", n)
			sql := harnessCTASSQL(harnessDatasetID, tableID, outer, inner)
			dst := &bigquery.Table{
				ProjectID: harnessProjectID,
				DatasetID: harnessDatasetID,
				TableID:   tableID,
			}
			b.StartTimer()

			job, err := harnessRunQuery(ctx, ts.URL, harnessProjectID, sql, dst)
			if err != nil {
				b.Fatal(err)
			}
			if _, err := job.Wait(ctx); err != nil {
				b.Fatal(err)
			}
			b.StopTimer()

			got, err := harnessReadCountC(ctx, client, harnessDatasetID, tableID)
			if err != nil {
				b.Fatal(err)
			}
			if got != want {
				b.Fatalf("c: got %d want %d", got, want)
			}
		}
	})
}

// BenchmarkCTAS_Rich_JobWait is like [BenchmarkCTAS_JobWait] but uses [harnessTortoiseRichCTASSQL].
// Tune with BQ_HARNESS_OUTER/INNER (via [harnessDimsFromEnv]) to match [TestTortoise_LongCTAS_Rich] load.
func BenchmarkCTAS_Rich_JobWait(b *testing.B) {
	if testing.Short() {
		b.Skip("skipping in -short")
	}
	ctx := context.Background()
	outer, inner := harnessDimsFromEnv(120, 15)
	want := int64(outer * inner)

	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		b.Fatal(err)
	}
	project := types.NewProject(harnessProjectID, types.NewDataset(harnessDatasetID))
	if err := bqServer.Load(server.StructSource(project)); err != nil {
		b.Fatal(err)
	}
	ts := bqServer.TestServer()
	client, err := bigquery.NewClient(ctx, harnessProjectID, option.WithEndpoint(ts.URL), option.WithoutAuthentication())
	if err != nil {
		ts.Close()
		_ = bqServer.Stop(ctx)
		b.Fatal(err)
	}
	defer client.Close()

	b.Cleanup(func() {
		ts.Close()
		_ = bqServer.Stop(context.Background())
	})

	b.Run("in_place_with_api_destination", func(b *testing.B) {
		for range b.N {
			b.StopTimer()
			n := benchCTASTableSeq.Add(1)
			tableID := fmt.Sprintf("hb_rich_p_%d", n)
			sql := harnessTortoiseRichCTASSQL(harnessDatasetID, tableID, outer, inner)
			dst := &bigquery.Table{
				ProjectID: harnessProjectID,
				DatasetID: harnessDatasetID,
				TableID:   tableID,
			}
			b.StartTimer()

			job, err := harnessRunQuery(ctx, ts.URL, harnessProjectID, sql, dst)
			if err != nil {
				b.Fatal(err)
			}
			if _, err := job.Wait(ctx); err != nil {
				b.Fatal(err)
			}
			b.StopTimer()

			got, err := harnessReadCountC(ctx, client, harnessDatasetID, tableID)
			if err != nil {
				b.Fatal(err)
			}
			if got != want {
				b.Fatalf("c: got %d want %d", got, want)
			}
		}
	})
}
