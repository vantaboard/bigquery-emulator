package handlers

import (
	"encoding/json"
	"net/http"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
)

// TestJobInsertDispatchesLoadConfig accepts load bodies at the HTTP layer
// and folds deferred data-plane work into job.status.errorResult.
func TestJobInsertDispatchesLoadConfig(t *testing.T) {
	t.Parallel()
	reg := jobs.NewRegistry()
	deps := Dependencies{Query: &fakeQueryClient{}, Jobs: reg}
	body := `{"configuration":{"load":{"sourceUris":["gs://bucket/file.csv"],"destinationTable":{"projectId":"dev","datasetId":"ds","tableId":"t"}}}}`
	rec := runJobInsert(t, deps, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200 (jobs.insert always returns Job)", rec.Code)
	}
	var got jobs.Job
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Configuration == nil || got.Configuration.Load == nil {
		t.Fatal("configuration.load not round-tripped")
	}
	if got.Configuration.JobType != "LOAD" {
		t.Errorf("jobType = %q, want LOAD", got.Configuration.JobType)
	}
	if got.Status.ErrorResult == nil {
		t.Fatal("expected deferred data-plane errorResult on load job")
	}
	if got.Statistics.Load == nil {
		t.Fatal("statistics.load missing on completed load job")
	}
	if got.Statistics.Load.InputFiles != "1" {
		t.Errorf("statistics.load.inputFiles = %q, want 1", got.Statistics.Load.InputFiles)
	}
}

func TestJobInsertDispatchesCopyConfig(t *testing.T) {
	t.Parallel()
	deps := Dependencies{Jobs: jobs.NewRegistry()}
	body := `{"configuration":{"copy":{"sourceTables":[{"projectId":"dev","datasetId":"ds","tableId":"src"}],"destinationTable":{"projectId":"dev","datasetId":"ds","tableId":"dst"}}}}`
	rec := runJobInsert(t, deps, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200", rec.Code)
	}
	var got jobs.Job
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Configuration.JobType != "COPY" {
		t.Errorf("jobType = %q, want COPY", got.Configuration.JobType)
	}
	if got.Statistics.Copy == nil {
		t.Fatal("statistics.copy missing on completed copy job")
	}
}

func TestJobInsertDispatchesExtractConfig(t *testing.T) {
	t.Parallel()
	deps := Dependencies{Jobs: jobs.NewRegistry()}
	body := `{"configuration":{"extract":{"sourceTable":{"projectId":"dev","datasetId":"ds","tableId":"t"},"destinationUris":["gs://bucket/out.csv"],"destinationFormat":"CSV"}}}`
	rec := runJobInsert(t, deps, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200", rec.Code)
	}
	var got jobs.Job
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Configuration.JobType != "EXTRACT" {
		t.Errorf("jobType = %q, want EXTRACT", got.Configuration.JobType)
	}
	if got.Statistics.Extract == nil {
		t.Fatal("statistics.extract missing on completed extract job")
	}
	if len(got.Statistics.Extract.DestinationURIFileCounts) != 1 {
		t.Errorf("destinationUriFileCounts len = %d, want 1",
			len(got.Statistics.Extract.DestinationURIFileCounts))
	}
}

// TestJobInsertRejectsUnknownConfig keeps HTTP 501 only for bodies that
// do not select a recognized configuration branch.
func TestJobInsertRejectsUnknownConfig(t *testing.T) {
	t.Parallel()
	body := `{"configuration":{"labels":{"k":"v"}}}`
	rec := runJobInsert(t, Dependencies{Jobs: jobs.NewRegistry()}, body)
	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want 501 for unrecognized configuration", rec.Code)
	}
}
