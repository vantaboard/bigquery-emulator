// Long CTAS: opt-in wall-clock profiling of the in-place (Dst) path with a heavy
// cross-join aggregate (see testfixtures/long_ctas_tortoise.sql).
//
// Set BQ_TORTOISE=1 to run. Optional:
//
//	BQ_TORTOISE_OUTER / BQ_TORTOISE_INNER — default 5000, 300 (1.5M join pairs; minutes on some machines)
//	BQ_TORTOISE_PRAGMAS=1 — run a second subtest with BQ_EMULATOR_SQLITE_PRAGMA_* (see runTortoisePragmas)
//
// Triage: run the emulator with --log-level info, then
//
//	grep "job_id=YOUR" emulator.log | ./scripts/ctas_perf_triage.sh
//
// Or: JOB_ID=dataform-... ./scripts/ctas_perf_triage.sh emulator.log
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
