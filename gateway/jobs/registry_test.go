package jobs

import (
	"encoding/json"
	"strconv"
	"strings"
	"sync"
	"testing"
	"time"
)

// TestNewJobIDUnique pins down the id format and asserts that two
// successive calls -- including from concurrent goroutines -- always
// produce distinct ids. The format guarantee lets `jobs.get` parse a
// jobId out of the URL without any extra state on the registry side.
func TestNewJobIDUnique(t *testing.T) {
	t.Parallel()

	r := NewRegistry()
	id := r.NewJobID()
	if !strings.HasPrefix(id, "job_") {
		t.Fatalf("jobId %q must start with %q (BigQuery auto-id convention)",
			id, "job_")
	}
	parts := strings.Split(strings.TrimPrefix(id, "job_"), "_")
	if len(parts) != 2 {
		t.Fatalf("jobId %q must be job_<nanos>_<seq>; got %d underscore parts",
			id, len(parts))
	}
	if _, err := strconv.ParseInt(parts[0], 10, 64); err != nil {
		t.Fatalf("jobId nanos component %q is not a decimal int: %v", parts[0], err)
	}
	if _, err := strconv.ParseUint(parts[1], 10, 64); err != nil {
		t.Fatalf("jobId seq component %q is not a decimal int: %v", parts[1], err)
	}

	const goroutines, perG = 32, 64
	got := make(chan string, goroutines*perG)
	var wg sync.WaitGroup
	for i := 0; i < goroutines; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for j := 0; j < perG; j++ {
				got <- r.NewJobID()
			}
		}()
	}
	wg.Wait()
	close(got)

	seen := make(map[string]struct{}, goroutines*perG)
	for id := range got {
		if _, dup := seen[id]; dup {
			t.Fatalf("duplicate jobId from concurrent NewJobID: %q", id)
		}
		seen[id] = struct{}{}
	}
}

// TestCompleteQueryStoresDoneJob walks the happy path: a synchronous
// query records a DONE job whose Status / Statistics carry the
// BigQuery-shaped timestamps and bytes-processed string, and the
// stored Job can be retrieved verbatim via Get.
func TestCompleteQueryStoresDoneJob(t *testing.T) {
	t.Parallel()

	r := NewRegistry()
	start := time.Date(2025, 1, 2, 3, 4, 5, 0, time.UTC)
	end := start.Add(750 * time.Millisecond)

	job := r.CompleteQuery("test-proj", "US", 1024, start, end)

	if job.Kind != JobKind {
		t.Errorf("kind = %q, want %q", job.Kind, JobKind)
	}
	if job.JobReference.ProjectID != "test-proj" {
		t.Errorf("jobReference.projectId = %q, want %q",
			job.JobReference.ProjectID, "test-proj")
	}
	if job.JobReference.Location != "US" {
		t.Errorf("jobReference.location = %q, want %q",
			job.JobReference.Location, "US")
	}
	if job.ID != "test-proj:"+job.JobReference.JobID {
		t.Errorf("job.id = %q, want %q",
			job.ID, "test-proj:"+job.JobReference.JobID)
	}
	if job.Status.State != JobStateDone {
		t.Errorf("status.state = %q, want %q", job.Status.State, JobStateDone)
	}
	if got, want := job.Statistics.CreationTime, strconv.FormatInt(start.UnixMilli(), 10); got != want {
		t.Errorf("statistics.creationTime = %q, want %q", got, want)
	}
	if got, want := job.Statistics.StartTime, strconv.FormatInt(start.UnixMilli(), 10); got != want {
		t.Errorf("statistics.startTime = %q, want %q", got, want)
	}
	if got, want := job.Statistics.EndTime, strconv.FormatInt(end.UnixMilli(), 10); got != want {
		t.Errorf("statistics.endTime = %q, want %q", got, want)
	}
	if got, want := job.Statistics.TotalBytesProcessed, "1024"; got != want {
		t.Errorf("statistics.totalBytesProcessed = %q, want %q", got, want)
	}

	loaded, ok := r.Get(job.JobReference.JobID)
	if !ok {
		t.Fatalf("Get(%q) missing after CompleteQuery", job.JobReference.JobID)
	}
	if loaded != job {
		t.Errorf("Get returned %p, want stored %p", loaded, job)
	}

	if _, ok := r.Get("job_does_not_exist"); ok {
		t.Error("Get returned a hit for an unknown jobId")
	}
}

// TestCompleteQueryJSONShape pins the on-the-wire JSON for a stored
// Job so `jobs.get` round-trips through the registry without an
// intermediate transform layer. The keys here mirror the upstream
// `Job` resource we cross-check against in
// docs/bigquery/docs/reference/rest/v2/jobs/get.md.
func TestCompleteQueryJSONShape(t *testing.T) {
	t.Parallel()

	r := NewRegistry()
	start := time.UnixMilli(1700000000000).UTC()
	end := start.Add(time.Second)
	job := r.CompleteQuery("proj", "", 0, start, end)

	raw, err := json.Marshal(job)
	if err != nil {
		t.Fatalf("json.Marshal: %v", err)
	}
	var out map[string]any
	if err := json.Unmarshal(raw, &out); err != nil {
		t.Fatalf("json.Unmarshal: %v", err)
	}

	wantTop := []string{"kind", "id", "jobReference", "status", "statistics"}
	for _, k := range wantTop {
		if _, ok := out[k]; !ok {
			t.Errorf("missing top-level key %q in %s", k, raw)
		}
	}

	ref, _ := out["jobReference"].(map[string]any)
	if ref == nil {
		t.Fatalf("jobReference is not an object: %s", raw)
	}
	if got := ref["projectId"]; got != "proj" {
		t.Errorf("jobReference.projectId = %v, want %q", got, "proj")
	}
	if _, ok := ref["location"]; ok {
		t.Errorf("jobReference.location should be omitted when empty; got %v", ref["location"])
	}

	stats, _ := out["statistics"].(map[string]any)
	if stats == nil {
		t.Fatalf("statistics is not an object: %s", raw)
	}
	if got, want := stats["totalBytesProcessed"], "0"; got != want {
		t.Errorf("statistics.totalBytesProcessed = %v, want %q", got, want)
	}
}
