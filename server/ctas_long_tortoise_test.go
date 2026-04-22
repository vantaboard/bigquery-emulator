// Long CTAS: opt-in wall-clock profiling of the in-place (Dst) path. Minimal cross-join:
// testfixtures/long_ctas_tortoise.sql and [harnessCTASSQL]. Rich pipeline-style: testfixtures
// long_ctas_tortoise_rich.sql and [harnessTortoiseRichCTASSQL] (same tunable cross join, extra CTEs
// and pivot-style / unpivot-style stages).
//
// Set BQ_TORTOISE=1 to run the minimal tortoise, or BQ_TORTOISE=1 and BQ_TORTOISE_RICH=1 for
// [TestTortoise_LongCTAS_Rich]. For large local runs, use a wall clock cap on the test process, e.g.
//	go test -timeout 30m -tags "$GOOGLESQL_BUILD_TAGS" -run 'TestTortoise_Long' -v ./server/...
//
//	BQ_TORTOISE_OUTER / BQ_TORTOISE_INNER — default 5000, 300 (1.5M join pairs; minutes on some machines; stress scale e.g. 6000*350)
//	BQ_TORTOISE_PRAGMAS=1 — after baseline, a second subtest with BQ_EMULATOR_SQLITE_PRAGMA_* to compare
//
// Triage: run the emulator with --log-level info, then filter one job and run:
//	grep "job_id=YOUR" emulator.log | ./scripts/ctas_perf_triage.sh
//	JOB_ID=dataform-... ./scripts/ctas_perf_triage.sh path/to/emulator.log
// Compare exec_ms vs row_count_ms vs metadata_sync (script prints a branch hint).
// If exec_ms dominates, tune SQLite (server/server.go) or profile go-googlesqlite; reduce OUTER*INNER for pipeline SLA.
package server_test

import (
	"context"
	"os"
	"strconv"
	"testing"
	"time"

	"cloud.google.com/go/bigquery"
	"github.com/vantaboard/bigquery-emulator/server"
	"github.com/vantaboard/bigquery-emulator/types"
	"google.golang.org/api/option"
)

func tortoiseDimsFromEnv(defaultOuter, defaultInner int) (outer, inner int) {
	outer, inner = defaultOuter, defaultInner
	if s := os.Getenv("BQ_TORTOISE_OUTER"); s != "" {
		if v, err := strconv.Atoi(s); err == nil && v > 0 {
			outer = v
		}
	}
	if s := os.Getenv("BQ_TORTOISE_INNER"); s != "" {
		if v, err := strconv.Atoi(s); err == nil && v > 0 {
			inner = v
		}
	}
	return outer, inner
}

func runTortoiseInPlaceCTAS(t *testing.T, outer, inner int) (elapsed time.Duration) {
	t.Helper()
	ctx := context.Background()
	const tableID = "tortoise_in_place"
	want := int64(outer * inner)
	sql := harnessCTASSQL(harnessDatasetID, tableID, outer, inner)
	dst := &bigquery.Table{
		ProjectID: harnessProjectID,
		DatasetID: harnessDatasetID,
		TableID:   tableID,
	}
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
	t0 := time.Now()
	job, err := harnessRunQuery(ctx, ts.URL, harnessProjectID, sql, dst)
	if err != nil {
		t.Fatalf("Run: %v", err)
	}
	if _, err := job.Wait(ctx); err != nil {
		t.Fatalf("Wait: %v", err)
	}
	elapsed = time.Since(t0)
	t.Logf("tortoise in_place: job_wait=%s outer=%d inner=%d product=%d", elapsed, outer, inner, outer*inner)
	got, err := harnessReadCountC(ctx, client, harnessDatasetID, tableID)
	if err != nil {
		t.Fatalf("read c: %v", err)
	}
	if got != want {
		t.Fatalf("c: got %d want %d", got, want)
	}
	return elapsed
}

func runTortoiseRichInPlaceCTAS(t *testing.T, outer, inner int) (elapsed time.Duration) {
	t.Helper()
	ctx := context.Background()
	const tableID = "tortoise_rich_in_place"
	want := int64(outer * inner)
	sql := harnessTortoiseRichCTASSQL(harnessDatasetID, tableID, outer, inner)
	dst := &bigquery.Table{
		ProjectID: harnessProjectID,
		DatasetID: harnessDatasetID,
		TableID:   tableID,
	}
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
	t0 := time.Now()
	job, err := harnessRunQuery(ctx, ts.URL, harnessProjectID, sql, dst)
	if err != nil {
		t.Fatalf("Run: %v", err)
	}
	if _, err := job.Wait(ctx); err != nil {
		t.Fatalf("Wait: %v", err)
	}
	elapsed = time.Since(t0)
	t.Logf("tortoise rich in_place: job_wait=%s outer=%d inner=%d product=%d", elapsed, outer, inner, outer*inner)
	got, err := harnessReadCountC(ctx, client, harnessDatasetID, tableID)
	if err != nil {
		t.Fatalf("read c: %v", err)
	}
	if got != want {
		t.Fatalf("c: got %d want %d", got, want)
	}
	return elapsed
}

// TestTortoise_LongCTAS_InPlace runs a heavy in-place CTAS when BQ_TORTOISE=1.
// Baseline and optional pragma-tuned subtests use separate [server.New] instances
// (env read at startup) so you can compare wall times in -v output.
func TestTortoise_LongCTAS_InPlace(t *testing.T) {
	if os.Getenv("BQ_TORTOISE") != "1" {
		t.Skip("set BQ_TORTOISE=1 for long in-place CTAS; see testfixtures/long_ctas_tortoise.sql")
	}
	outer, inner := tortoiseDimsFromEnv(5000, 300)
	var base time.Duration
	t.Run("baseline", func(t *testing.T) {
		base = runTortoiseInPlaceCTAS(t, outer, inner)
	})
	if os.Getenv("BQ_TORTOISE_PRAGMAS") == "1" {
		t.Run("sqlite_pragmas", func(t *testing.T) {
			// I/O- and sync-heavy workloads: larger page cache, temp tables in memory, relaxed sync.
			t.Setenv("BQ_EMULATOR_SQLITE_PRAGMA_SYNCHRONOUS", "NORMAL")
			t.Setenv("BQ_EMULATOR_SQLITE_PRAGMA_TEMP_STORE", "MEMORY")
			t.Setenv("BQ_EMULATOR_SQLITE_PRAGMA_CACHE_SIZE", "-2000000")
			prag := runTortoiseInPlaceCTAS(t, outer, inner)
			if base > 0 {
				t.Logf("tortoise: baseline=%s with_pragmas=%s (compare -v line above)", base, prag)
			}
		})
	}
}

// TestTortoise_LongCTAS_Rich runs the multi-CTE harness ([harnessTortoiseRichCTASSQL]) at the same
// tunable dimensions as [TestTortoise_LongCTAS_InPlace] when BQ_TORTOISE=1 and BQ_TORTOISE_RICH=1.
// Optional BQ_TORTOISE_PRAGMAS=1 includes the same pragma A/B as the minimal test.
func TestTortoise_LongCTAS_Rich(t *testing.T) {
	if os.Getenv("BQ_TORTOISE") != "1" || os.Getenv("BQ_TORTOISE_RICH") != "1" {
		t.Skip("set BQ_TORTOISE=1 and BQ_TORTOISE_RICH=1 for long rich in-place CTAS; see testfixtures/long_ctas_tortoise_rich.sql")
	}
	outer, inner := tortoiseDimsFromEnv(5000, 300)
	var base time.Duration
	t.Run("baseline", func(t *testing.T) {
		base = runTortoiseRichInPlaceCTAS(t, outer, inner)
	})
	if os.Getenv("BQ_TORTOISE_PRAGMAS") == "1" {
		t.Run("sqlite_pragmas", func(t *testing.T) {
			t.Setenv("BQ_EMULATOR_SQLITE_PRAGMA_SYNCHRONOUS", "NORMAL")
			t.Setenv("BQ_EMULATOR_SQLITE_PRAGMA_TEMP_STORE", "MEMORY")
			t.Setenv("BQ_EMULATOR_SQLITE_PRAGMA_CACHE_SIZE", "-2000000")
			prag := runTortoiseRichInPlaceCTAS(t, outer, inner)
			if base > 0 {
				t.Logf("tortoise rich: baseline=%s with_pragmas=%s (compare -v line above)", base, prag)
			}
		})
	}
}
