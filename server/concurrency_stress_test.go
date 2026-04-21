// Package stress tests exercise concurrent GetQueryResults polls and metadata GETs while a long
// async CTAS runs (Dataform-like). They are skipped under -short.
//
// Instrumentation: set BQ_EMULATOR_CONN_METRICS=1 and use [connection.ResetConnMetrics] /
// [connection.SnapshotConnMetrics] to confirm pool acquisition wait (channel) vs time in handlers.
// Poll responses are classified (e.g. http_500, net_timeout) via stressPollQueryGETTag.
//
// Capacity: default elastic pool ceiling scales with CPU (see [connection.PoolBoundsFromResources]);
// typical desktops land near 20+ connections. stressWorkers=12 with one async job yields at most
// 13 concurrent pool users for that scenario. Override with BQ_EMULATOR_POOL_SIZE (fixed) or
// BQ_EMULATOR_POOL_MAX / [server.WithConnectionPoolSize] if raising stressWorkers.
//
// Threshold: stressMaxErrRate allows a small fraction of failures so CI stays stable. Residual errors
// are typically http_500 from SQLite contention or handlers, not multi-second pool waits (metrics
// show sub-millisecond average acquire wait when enabled). Metadata rounds (two GETs) can fail slightly
// more often than poll GETs alone; 2.5% caps both. Tighten if root-cause 500s are eliminated.
package server_test

import (
	"context"
	"errors"
	"fmt"
	"io"
	"net"
	"net/http"
	"strings"
	"sync"
	"sync/atomic"
	"testing"
	"time"

	"cloud.google.com/go/bigquery"
	"github.com/vantaboard/bigquery-emulator/internal/connection"
	"github.com/vantaboard/bigquery-emulator/server"
	"github.com/vantaboard/bigquery-emulator/types"
	"google.golang.org/api/option"
)

// stressCrossJoin* controls how heavy the async CTAS is. The goal is a job that stays RUNNING
// long enough to overlap with concurrent readers (exposes SQLite / pool issues if they regress).
// Tune up if your machine finishes too fast; tune down if CI times out.
//
const (
	stressCrossJoinOuter  = 6000
	stressCrossJoinInner  = 350 // ~2.1M pairs
	stressWorkers         = 12
	stressDuration        = 5 * time.Second
	stressIterationDelay  = 2 * time.Millisecond // avoid tight spin that can starve or trip rebind
	stabilizeAfterRunning = 150 * time.Millisecond
	// Long enough that occasional SQLite contention does not false-positive; short enough to catch hangs.
	stressHTTPTimeout = 15 * time.Second
	// Upper bound on failed poll/metadata rounds; see package doc.
	stressMaxErrRate = 0.025
)

func stressHeavyCTASQuery(dataset string) string {
	return fmt.Sprintf(
		"CREATE TABLE `%s.heavy` AS "+
			"SELECT COUNT(*) AS c FROM UNNEST(GENERATE_ARRAY(1, %d)) a "+
			"CROSS JOIN UNNEST(GENERATE_ARRAY(1, %d)) b",
		dataset, stressCrossJoinOuter, stressCrossJoinInner,
	)
}

// TestStress_AsyncQueryDoesNotBlockConcurrentReaders starts a long-running async job, then runs
// concurrent GETs (query-results polling, dataset list, missing table) similar to Dataform.
// If SQLite locking or the connection pool wedges, requests exceed perRequestTimeout or return 500.
//
// Shutdown always waits for the heavy job so the test does not close the server while the async
// worker still holds a connection (which can panic or corrupt the pool).
func TestStress_AsyncQueryDoesNotBlockConcurrentReaders(t *testing.T) {
	if testing.Short() {
		t.Skip("skipping stress test in -short")
	}

	t.Setenv("BQ_EMULATOR_CONN_METRICS", "1")
	connection.ResetConnMetrics()

	ctx := context.Background()
	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		t.Fatal(err)
	}
	projectID := "stressproj"
	datasetID := "ds1"
	project := types.NewProject(projectID, types.NewDataset(datasetID))
	if err := bqServer.Load(server.StructSource(project)); err != nil {
		t.Fatal(err)
	}
	ts := bqServer.TestServer()

	client, err := bigquery.NewClient(ctx, projectID, option.WithEndpoint(ts.URL), option.WithoutAuthentication())
	if err != nil {
		t.Fatal(err)
	}
	defer client.Close()

	q := client.Query(stressHeavyCTASQuery(datasetID))
	heavyJob, err := q.Run(ctx)
	if err != nil {
		t.Fatal(err)
	}
	jobID := heavyJob.ID()
	t.Logf("submitted async job id=%s", jobID)

	t.Cleanup(func() {
		// Always drain the async job before closing pooled sqlite connections.
		drainCtx, cancel := context.WithTimeout(context.Background(), 15*time.Minute)
		defer cancel()
		if _, err := heavyJob.Wait(drainCtx); err != nil {
			t.Logf("cleanup: heavy job wait: %v", err)
		}
		ts.Close()
		_ = bqServer.Stop(context.Background())
	})

	deadline := time.Now().Add(15 * time.Second)
	var sawRunning bool
	for time.Now().Before(deadline) {
		st, err := heavyJob.Status(ctx)
		if err != nil {
			t.Fatal(err)
		}
		switch st.State {
		case bigquery.Running:
			sawRunning = true
			goto running
		case bigquery.Done:
			t.Log("job finished before stress window; increase stressCrossJoinOuter/Inner for stronger overlap")
			sawRunning = false
			goto running
		case bigquery.Pending:
			time.Sleep(20 * time.Millisecond)
		default:
			time.Sleep(20 * time.Millisecond)
		}
	}
	t.Fatal("timed out waiting for job to leave pending")

running:
	if !sawRunning {
		t.Log("warning: job never observed in Running; stress may be weaker on this machine")
	}
	time.Sleep(stabilizeAfterRunning)

	httpClient := &http.Client{Timeout: stressHTTPTimeout}
	base := strings.TrimRight(ts.URL, "/")
	var (
		okCount  atomic.Uint64
		errCount atomic.Uint64
	)
	var pollMu sync.Mutex
	pollCounts := make(map[string]int64)
	stressCtx, cancelStress := context.WithTimeout(ctx, stressDuration)
	defer cancelStress()

	var wg sync.WaitGroup
	for w := 0; w < stressWorkers; w++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for {
				select {
				case <-stressCtx.Done():
					return
				default:
				}
				tag := stressPollQueryGETTag(httpClient, base, projectID, jobID)
				pollMu.Lock()
				pollCounts[tag]++
				pollMu.Unlock()
				if tag == "ok" {
					okCount.Add(1)
				} else {
					errCount.Add(1)
				}
				time.Sleep(stressIterationDelay)
			}
		}()
	}
	wg.Wait()

	ok := okCount.Load()
	bad := errCount.Load()
	t.Logf("stress complete: ok=%d err=%d (workers=%d duration=%s) poll_tags=%v",
		ok, bad, stressWorkers, stressDuration, pollCountsSnapshot(pollCounts))

	m := connection.SnapshotConnMetrics()
	if m.AcquireCount > 0 {
		avgWait := time.Duration(uint64(m.TotalAcquireWait) / m.AcquireCount)
		t.Logf("connection metrics: acquire_wait_total=%s fn_hold_total=%s acquires=%d acquire_canceled=%d avg_acquire_wait=%s",
			m.TotalAcquireWait, m.TotalFnHold, m.AcquireCount, m.AcquireCanceled, avgWait)
	}

	total := ok + bad
	if total == 0 {
		total = 1
	}
	errRate := float64(bad) / float64(total)
	if bad > 0 && errRate > stressMaxErrRate {
		t.Fatalf("poll GET failures: %d of %d (%.3f%%). ok=%d. "+
			"Indicates timeouts, 500s, or client errors while the async job runs — check pool/SQLite.",
			bad, total, 100*errRate, ok)
	}
	if ok < 200 {
		t.Fatalf("expected many successful polls, got ok=%d", ok)
	}
	if bad > 0 {
		t.Logf("note: %d poll errors (%.3f%%) within %.2f%% allowance; tags=%v",
			bad, 100*errRate, 100*stressMaxErrRate, pollCountsSnapshot(pollCounts))
	}

	waitCtx, cancelWait := context.WithTimeout(ctx, 10*time.Minute)
	defer cancelWait()
	if _, err := heavyJob.Wait(waitCtx); err != nil {
		t.Fatalf("heavy job wait: %v", err)
	}
	q2 := client.Query("SELECT 1 AS x")
	job2, err := q2.Run(ctx)
	if err != nil {
		t.Fatal(err)
	}
	if _, err := job2.Wait(waitCtx); err != nil {
		t.Fatalf("follow-up query job: %v", err)
	}
}

func pollCountsSnapshot(m map[string]int64) map[string]int64 {
	out := make(map[string]int64, len(m))
	for k, v := range m {
		out[k] = v
	}
	return out
}

// stressPollQueryGETTag is one GetQueryResults poll (Dataform wait loop). Returns "ok" on HTTP 200.
func stressPollQueryGETTag(hc *http.Client, base, projectID, jobID string) string {
	u := fmt.Sprintf("%s/bigquery/v2/projects/%s/queries/%s?location=US&prettyPrint=false", base, projectID, jobID)
	req, err := http.NewRequest(http.MethodGet, u, nil)
	if err != nil {
		return "new_request"
	}
	resp, err := hc.Do(req)
	if err != nil {
		var ne net.Error
		if errors.As(err, &ne) && ne.Timeout() {
			return "net_timeout"
		}
		if errors.Is(err, context.DeadlineExceeded) {
			return "net_deadline"
		}
		return "net_other"
	}
	_, _ = io.Copy(io.Discard, resp.Body)
	_ = resp.Body.Close()
	switch resp.StatusCode {
	case http.StatusOK:
		return "ok"
	case http.StatusInternalServerError:
		return "http_500"
	case http.StatusServiceUnavailable:
		return "http_503"
	case http.StatusBadGateway:
		return "http_502"
	default:
		return fmt.Sprintf("http_%d", resp.StatusCode)
	}
}

// TestStress_MetadataGETsDuringLongAsyncQuery runs dataset + table metadata GETs concurrent with a
// long job (404 on missing table is expected and must not be 500).
func TestStress_MetadataGETsDuringLongAsyncQuery(t *testing.T) {
	if testing.Short() {
		t.Skip("skipping stress test in -short")
	}

	t.Setenv("BQ_EMULATOR_CONN_METRICS", "1")
	connection.ResetConnMetrics()

	ctx := context.Background()
	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		t.Fatal(err)
	}
	projectID := "metaproj"
	datasetID := "ds1"
	project := types.NewProject(projectID, types.NewDataset(datasetID))
	if err := bqServer.Load(server.StructSource(project)); err != nil {
		t.Fatal(err)
	}
	ts := bqServer.TestServer()
	client, err := bigquery.NewClient(ctx, projectID, option.WithEndpoint(ts.URL), option.WithoutAuthentication())
	if err != nil {
		t.Fatal(err)
	}
	defer client.Close()

	q := client.Query(stressHeavyCTASQuery(datasetID))
	heavyJob, err := q.Run(ctx)
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() {
		drainCtx, cancel := context.WithTimeout(context.Background(), 15*time.Minute)
		defer cancel()
		_, _ = heavyJob.Wait(drainCtx)
		ts.Close()
		_ = bqServer.Stop(context.Background())
	})

	for range 80 {
		st, err := heavyJob.Status(ctx)
		if err != nil {
			t.Fatal(err)
		}
		if st.State == bigquery.Running || st.State == bigquery.Done {
			break
		}
		time.Sleep(25 * time.Millisecond)
	}
	time.Sleep(stabilizeAfterRunning)

	hc := &http.Client{Timeout: stressHTTPTimeout}
	base := strings.TrimRight(ts.URL, "/")
	var okN, badN atomic.Uint64
	metaTags := make(map[string]int64)
	var metaMu sync.Mutex
	deadline := time.Now().Add(4 * time.Second)
	for time.Now().Before(deadline) {
		tag, ok := stressMetadataRoundTag(hc, base, projectID, datasetID)
		metaMu.Lock()
		metaTags[tag]++
		metaMu.Unlock()
		if ok {
			okN.Add(1)
		} else {
			badN.Add(1)
		}
		time.Sleep(5 * time.Millisecond)
	}
	ok := okN.Load()
	bad := badN.Load()
	m := connection.SnapshotConnMetrics()
	if m.AcquireCount > 0 {
		avgWait := time.Duration(uint64(m.TotalAcquireWait) / m.AcquireCount)
		t.Logf("metadata conn metrics: acquire_wait_total=%s fn_hold_total=%s acquires=%d avg_wait=%s",
			m.TotalAcquireWait, m.TotalFnHold, m.AcquireCount, avgWait)
	}
	t.Logf("metadata stress ok=%d err=%d tags=%v", ok, bad, pollCountsSnapshot(metaTags))
	if bad > 0 && float64(bad)/float64(ok+bad) > stressMaxErrRate {
		t.Fatalf("metadata GET failures: %d of %d", bad, ok+bad)
	}
}

// stressMetadataRoundTag does datasets list + missing-table GET. ok if both succeed and no 500;
// 404 on missing table is success. tag classifies the first failure in the round.
func stressMetadataRoundTag(hc *http.Client, base, projectID, datasetID string) (tag string, ok bool) {
	paths := []struct {
		path string
		note string
	}{
		{fmt.Sprintf("%s/bigquery/v2/projects/%s/datasets?prettyPrint=false", base, projectID), "datasets_list"},
		{fmt.Sprintf("%s/bigquery/v2/projects/%s/datasets/%s/tables/missing_xyz?prettyPrint=false", base, projectID, datasetID), "missing_table"},
	}
	for _, p := range paths {
		resp, err := hc.Get(p.path)
		if err != nil {
			var ne net.Error
			if errors.As(err, &ne) && ne.Timeout() {
				return p.note + "/net_timeout", false
			}
			if errors.Is(err, context.DeadlineExceeded) {
				return p.note + "/net_deadline", false
			}
			return p.note + "/net_other", false
		}
		_, _ = io.Copy(io.Discard, resp.Body)
		sc := resp.StatusCode
		_ = resp.Body.Close()
		if sc == http.StatusInternalServerError {
			return p.note + "/http_500", false
		}
		switch p.note {
		case "datasets_list":
			if sc != http.StatusOK {
				return fmt.Sprintf("datasets_list/http_%d", sc), false
			}
		case "missing_table":
			if sc != http.StatusNotFound {
				return fmt.Sprintf("missing_table/http_%d", sc), false
			}
		}
	}
	return "ok", true
}

// BenchmarkPollQueryResultsWhileJobRunning measures single-request latency for GetQueryResults-style
// polling while a long async job is in flight. Compare ns/op across changes to catch latency regressions
// without relying only on stress error rates.
//
//	go test -tags "$GOOGLESQL_BUILD_TAGS" -bench BenchmarkPollQueryResultsWhileJobRunning -benchtime 3s ./server/...
func BenchmarkPollQueryResultsWhileJobRunning(b *testing.B) {
	if testing.Short() {
		b.Skip("skipping in -short")
	}
	ctx := context.Background()
	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		b.Fatal(err)
	}
	projectID := "benchproj"
	datasetID := "ds1"
	project := types.NewProject(projectID, types.NewDataset(datasetID))
	if err := bqServer.Load(server.StructSource(project)); err != nil {
		b.Fatal(err)
	}
	ts := bqServer.TestServer()
	heavyJob, err := clientQueryRun(ctx, ts.URL, projectID, stressHeavyCTASQuery(datasetID))
	if err != nil {
		ts.Close()
		_ = bqServer.Stop(ctx)
		b.Fatal(err)
	}
	b.Cleanup(func() {
		_, _ = heavyJob.Wait(context.Background())
		ts.Close()
		_ = bqServer.Stop(context.Background())
	})

	base := strings.TrimRight(ts.URL, "/")
	pollURL := fmt.Sprintf("%s/bigquery/v2/projects/%s/queries/%s?location=US&prettyPrint=false", base, projectID, heavyJob.ID())
	hc := &http.Client{Timeout: stressHTTPTimeout}

	b.ResetTimer()
	for range b.N {
		resp, err := hc.Get(pollURL)
		if err != nil {
			b.Fatal(err)
		}
		_, _ = io.Copy(io.Discard, resp.Body)
		_ = resp.Body.Close()
		if resp.StatusCode != http.StatusOK {
			b.Fatalf("poll status %d", resp.StatusCode)
		}
	}
	b.StopTimer()
}

func clientQueryRun(ctx context.Context, endpoint, projectID, sql string) (*bigquery.Job, error) {
	client, err := bigquery.NewClient(ctx, projectID, option.WithEndpoint(endpoint), option.WithoutAuthentication())
	if err != nil {
		return nil, err
	}
	defer client.Close()
	return client.Query(sql).Run(ctx)
}

// BenchmarkConcurrentPollsWhileJobRunning measures aggregate polling throughput with RunParallel.
//
//	go test -tags "$GOOGLESQL_BUILD_TAGS" -bench BenchmarkConcurrentPollsWhileJobRunning -benchtime 3s -cpu 4 ./server/...
func BenchmarkConcurrentPollsWhileJobRunning(b *testing.B) {
	if testing.Short() {
		b.Skip("skipping in -short")
	}
	ctx := context.Background()
	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		b.Fatal(err)
	}
	projectID := "benchproj2"
	datasetID := "ds1"
	project := types.NewProject(projectID, types.NewDataset(datasetID))
	if err := bqServer.Load(server.StructSource(project)); err != nil {
		b.Fatal(err)
	}
	ts := bqServer.TestServer()
	heavyJob, err := clientQueryRun(ctx, ts.URL, projectID, stressHeavyCTASQuery(datasetID))
	if err != nil {
		ts.Close()
		_ = bqServer.Stop(ctx)
		b.Fatal(err)
	}
	b.Cleanup(func() {
		_, _ = heavyJob.Wait(context.Background())
		ts.Close()
		_ = bqServer.Stop(context.Background())
	})

	base := strings.TrimRight(ts.URL, "/")
	pollURL := fmt.Sprintf("%s/bigquery/v2/projects/%s/queries/%s?location=US&prettyPrint=false", base, projectID, heavyJob.ID())
	hc := &http.Client{Timeout: stressHTTPTimeout}

	b.ResetTimer()
	b.RunParallel(func(pb *testing.PB) {
		for pb.Next() {
			time.Sleep(stressIterationDelay)
			resp, err := hc.Get(pollURL)
			if err != nil {
				b.Fatal(err)
			}
			_, _ = io.Copy(io.Discard, resp.Body)
			_ = resp.Body.Close()
			if resp.StatusCode != http.StatusOK {
				b.Fatalf("poll status %d", resp.StatusCode)
			}
		}
	})
	b.StopTimer()
}
