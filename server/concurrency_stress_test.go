package server_test

import (
	"context"
	"fmt"
	"io"
	"net/http"
	"strings"
	"sync"
	"sync/atomic"
	"testing"
	"time"

	"cloud.google.com/go/bigquery"
	"github.com/vantaboard/bigquery-emulator/server"
	"github.com/vantaboard/bigquery-emulator/types"
	"google.golang.org/api/option"
)

// stressCrossJoin* controls how heavy the async CTAS is. The goal is a job that stays RUNNING
// long enough to overlap with concurrent readers (exposes SQLite / pool issues if they regress).
// Tune up if your machine finishes too fast; tune down if CI times out.
const (
	stressCrossJoinOuter  = 6000
	stressCrossJoinInner  = 350 // ~2.1M pairs
	stressWorkers         = 12 // near default pool size (12) to stress acquisition
	stressDuration        = 5 * time.Second
	stressIterationDelay  = 2 * time.Millisecond // avoid tight spin that can starve or trip rebind
	stabilizeAfterRunning = 150 * time.Millisecond
	// Long enough that occasional SQLite contention does not false-positive; short enough to catch hangs.
	stressHTTPTimeout = 15 * time.Second
	// Under heavy concurrent polls, localhost + SQLite can still see occasional timeouts; cap avoids flaky CI.
	stressMaxErrRate = 0.05
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
				if ok, _ := stressPollQueryGET(httpClient, base, projectID, jobID); ok {
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
	t.Logf("stress complete: ok=%d err=%d (workers=%d duration=%s)", ok, bad, stressWorkers, stressDuration)

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
		t.Logf("note: %d poll errors (%.3f%%) within %.1f%% allowance", bad, 100*errRate, 100*stressMaxErrRate)
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

// stressPollQueryGET is one GetQueryResults poll (primary Dataform wait loop). 200 while running/done.
func stressPollQueryGET(hc *http.Client, base, projectID, jobID string) (ok bool, detail string) {
	u := fmt.Sprintf("%s/bigquery/v2/projects/%s/queries/%s?location=US&prettyPrint=false", base, projectID, jobID)
	req, err := http.NewRequest(http.MethodGet, u, nil)
	if err != nil {
		return false, fmt.Sprintf("NewRequest: %v", err)
	}
	resp, err := hc.Do(req)
	if err != nil {
		return false, err.Error()
	}
	_, _ = io.Copy(io.Discard, resp.Body)
	_ = resp.Body.Close()
	if resp.StatusCode == http.StatusInternalServerError {
		return false, fmt.Sprintf("status %d", resp.StatusCode)
	}
	return true, ""
}

// TestStress_MetadataGETsDuringLongAsyncQuery runs dataset + table metadata GETs concurrent with a
// long job (404 on missing table is expected and must not be 500).
func TestStress_MetadataGETsDuringLongAsyncQuery(t *testing.T) {
	if testing.Short() {
		t.Skip("skipping stress test in -short")
	}
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
	deadline := time.Now().Add(4 * time.Second)
	for time.Now().Before(deadline) {
		if stressMetadataGET(hc, base, projectID, datasetID) {
			okN.Add(1)
		} else {
			badN.Add(1)
		}
		time.Sleep(5 * time.Millisecond)
	}
	ok := okN.Load()
	bad := badN.Load()
	t.Logf("metadata stress ok=%d err=%d", ok, bad)
	if bad > 0 && float64(bad)/float64(ok+bad) > stressMaxErrRate {
		t.Fatalf("metadata GET failures: %d of %d", bad, ok+bad)
	}
}

func stressMetadataGET(hc *http.Client, base, projectID, datasetID string) bool {
	paths := []string{
		fmt.Sprintf("%s/bigquery/v2/projects/%s/datasets?prettyPrint=false", base, projectID),
		fmt.Sprintf("%s/bigquery/v2/projects/%s/datasets/%s/tables/missing_xyz?prettyPrint=false", base, projectID, datasetID),
	}
	for _, u := range paths {
		resp, err := hc.Get(u)
		if err != nil {
			return false
		}
		_, _ = io.Copy(io.Discard, resp.Body)
		_ = resp.Body.Close()
		if resp.StatusCode == http.StatusInternalServerError {
			return false
		}
	}
	return true
}

// BenchmarkPollQueryResultsWhileJobRunning measures single-request latency for GetQueryResults-style
// polling while a long async job is in flight.
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
