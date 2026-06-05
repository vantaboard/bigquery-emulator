package handlers

import (
	"encoding/json"
	"net/http"
	"os"
	"path/filepath"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
)

func TestJobInsertLoadFromLocalCSV(t *testing.T) {
	t.Parallel()
	dir := t.TempDir()
	csvPath := filepath.Join(dir, "states.csv")
	csvBody := "name,post_abbr\nAlabama,AL\nAlaska,AK\n"
	if err := os.WriteFile(csvPath, []byte(csvBody), 0o644); err != nil {
		t.Fatal(err)
	}

	cat := &fakeCatalogClient{}
	reg := jobs.NewRegistry()
	deps := Dependencies{Catalog: cat, Jobs: reg}
	body := `{"configuration":{"load":{"sourceUris":["file://` + csvPath + `"],` +
		`"destinationTable":{"projectId":"dev","datasetId":"ds","tableId":"states"},` +
		`"sourceFormat":"CSV","skipLeadingRows":"1",` +
		`"schema":{"fields":[{"name":"name","type":"STRING"},{"name":"post_abbr","type":"STRING"}]}}}}`

	rec := runJobInsert(t, deps, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, body=%s", rec.Code, rec.Body.String())
	}
	var got jobs.Job
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Status.ErrorResult != nil {
		t.Fatalf("errorResult = %#v", got.Status.ErrorResult)
	}
	if got.Statistics.Load == nil || got.Statistics.Load.OutputRows != "2" {
		t.Fatalf("statistics.load = %#v", got.Statistics.Load)
	}
	if cat.lastInsertRows == nil || len(cat.lastInsertRows.Rows) != 2 {
		t.Fatalf("InsertRows rows = %d, want 2", len(cat.lastInsertRows.GetRows()))
	}
}

func TestJobConfigurationLoadSkipLeadingRowsString(t *testing.T) {
	t.Parallel()
	var cfg jobs.JobConfiguration
	if err := json.Unmarshal([]byte(`{"load":{"skipLeadingRows":"1"}}`), &cfg); err != nil {
		t.Fatalf("unmarshal: %v", err)
	}
	if cfg.Load.SkipLeadingRows() != 1 {
		t.Fatalf("skipLeadingRows = %d, want 1", cfg.Load.SkipLeadingRows())
	}
}

func TestJobInsertLoadWithoutCatalogStillDeferred(t *testing.T) {
	t.Parallel()
	reg := jobs.NewRegistry()
	body := `{"configuration":{"load":{"sourceUris":["gs://b/f.csv"],"destinationTable":{"projectId":"dev","datasetId":"ds","tableId":"t"}}}}`
	rec := runJobInsert(t, Dependencies{Jobs: reg}, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d", rec.Code)
	}
	var got jobs.Job
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Status.ErrorResult == nil {
		t.Fatal("expected deferred errorResult when Catalog is nil")
	}
}
