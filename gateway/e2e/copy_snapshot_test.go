//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestCopyTableJobE2E exercises the UI copy-table modal path: a synchronous
// configuration.copy job reaches DONE and the destination table has the
// source rows.
func TestCopyTableJobE2E(t *testing.T) {
	env := startEmulatorWithFlags(t, emulatorFlags{dataDir: t.TempDir()})

	const (
		projectID = "proj-copy-table"
		datasetID = "ds_copy"
		srcTable  = "src"
		dstTable  = "dst"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	createBody := `{
		"tableReference":{"projectId":"` + projectID + `","datasetId":"` + datasetID + `","tableId":"` + srcTable + `"},
		"schema":{"fields":[{"name":"id","type":"INT64"}]}
	}`
	status, body = doJSON(t, http.MethodPost, base+"/datasets/"+datasetID+"/tables", []byte(createBody))
	if status != http.StatusOK {
		t.Fatalf("tables.insert src -> %d: %s", status, string(body))
	}

	insertBody := `{"rows":[{"json":{"id":"7"}}]}`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables/"+srcTable+"/insertAll", []byte(insertBody))
	if status != http.StatusOK {
		t.Fatalf("tabledata.insertAll -> %d: %s", status, string(body))
	}

	copyJob := `{
		"configuration":{
			"copy":{
				"sourceTable":{"projectId":"` + projectID + `","datasetId":"` + datasetID + `","tableId":"` + srcTable + `"},
				"destinationTable":{"projectId":"` + projectID + `","datasetId":"` + datasetID + `","tableId":"` + dstTable + `"},
				"writeDisposition":"WRITE_EMPTY"
			}
		}
	}`
	status, body = doJSON(t, http.MethodPost, base+"/jobs", []byte(copyJob))
	if status != http.StatusOK {
		t.Fatalf("jobs.insert copy -> %d: %s", status, string(body))
	}
	var job struct {
		Status struct {
			State       string `json:"state"`
			ErrorResult *struct {
				Message string `json:"message"`
			} `json:"errorResult"`
		} `json:"status"`
	}
	if err := json.Unmarshal(body, &job); err != nil {
		t.Fatalf("decode job: %v", err)
	}
	if job.Status.State != "DONE" {
		t.Fatalf("job state = %q, want DONE", job.Status.State)
	}
	if job.Status.ErrorResult != nil {
		t.Fatalf("job error: %s", job.Status.ErrorResult.Message)
	}

	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables/"+dstTable, nil)
	if status != http.StatusOK {
		t.Fatalf("tables.get dst -> %d: %s", status, string(body))
	}
	var got bqtypes.Table
	if err := json.Unmarshal(body, &got); err != nil {
		t.Fatalf("decode table: %v", err)
	}
	if got.NumRows != "1" {
		t.Fatalf("destination numRows = %q, want 1", got.NumRows)
	}
}

// TestSnapshotCopyJobE2E mirrors the UI snapshot modal: operationType=SNAPSHOT
// with destinationExpirationTime stamps the destination as SNAPSHOT and
// persists expiration on tables.get.
func TestSnapshotCopyJobE2E(t *testing.T) {
	env := startEmulatorWithFlags(t, emulatorFlags{dataDir: t.TempDir()})

	const (
		projectID = "proj-snapshot"
		datasetID = "ds_snap"
		srcTable  = "live"
		snapTable = "snap"
		expMs     = "1990000000000"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	createBody := `{
		"tableReference":{"projectId":"` + projectID + `","datasetId":"` + datasetID + `","tableId":"` + srcTable + `"},
		"schema":{"fields":[{"name":"name","type":"STRING"}]}
	}`
	status, body = doJSON(t, http.MethodPost, base+"/datasets/"+datasetID+"/tables", []byte(createBody))
	if status != http.StatusOK {
		t.Fatalf("tables.insert -> %d: %s", status, string(body))
	}

	insertBody := `{"rows":[{"json":{"name":"snap_src"}}]}`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables/"+srcTable+"/insertAll", []byte(insertBody))
	if status != http.StatusOK {
		t.Fatalf("tabledata.insertAll -> %d: %s", status, string(body))
	}

	snapJob := `{
		"configuration":{
			"copy":{
				"sourceTable":{"projectId":"` + projectID + `","datasetId":"` + datasetID + `","tableId":"` + srcTable + `"},
				"destinationTable":{"projectId":"` + projectID + `","datasetId":"` + datasetID + `","tableId":"` + snapTable + `"},
				"operationType":"SNAPSHOT",
				"destinationExpirationTime":"` + expMs + `",
				"writeDisposition":"WRITE_EMPTY"
			}
		}
	}`
	status, body = doJSON(t, http.MethodPost, base+"/jobs", []byte(snapJob))
	if status != http.StatusOK {
		t.Fatalf("jobs.insert snapshot -> %d: %s", status, string(body))
	}

	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables/"+snapTable, nil)
	if status != http.StatusOK {
		t.Fatalf("tables.get snap -> %d: %s", status, string(body))
	}
	var got bqtypes.Table
	if err := json.Unmarshal(body, &got); err != nil {
		t.Fatalf("decode table: %v", err)
	}
	if got.Type != "SNAPSHOT" {
		t.Fatalf("type = %q, want SNAPSHOT", got.Type)
	}
	if got.ExpirationTime.String() != expMs {
		t.Fatalf("expirationTime = %q, want %q", got.ExpirationTime, expMs)
	}
	if got.NumRows != "1" {
		t.Fatalf("numRows = %q, want 1", got.NumRows)
	}
}

// TestMultiSourceCopyJobE2E verifies per-table dataset copy orchestration:
// one copy job with multiple sourceTables appends into a single destination.
func TestMultiSourceCopyJobE2E(t *testing.T) {
	env := startEmulatorWithFlags(t, emulatorFlags{dataDir: t.TempDir()})

	const (
		projectID = "proj-multi-copy"
		datasetID = "ds_multi"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	for _, spec := range []struct{ id, val string }{
		{"t1", "1"},
		{"t2", "2"},
	} {
		createBody := `{
			"tableReference":{"projectId":"` + projectID + `","datasetId":"` + datasetID + `","tableId":"` + spec.id + `"},
			"schema":{"fields":[{"name":"id","type":"INT64"}]}
		}`
		status, body = doJSON(t, http.MethodPost, base+"/datasets/"+datasetID+"/tables", []byte(createBody))
		if status != http.StatusOK {
			t.Fatalf("tables.insert %s -> %d: %s", spec.id, status, string(body))
		}
		insertBody := `{"rows":[{"json":{"id":"` + spec.val + `"}}]}`
		status, body = doJSON(t, http.MethodPost,
			base+"/datasets/"+datasetID+"/tables/"+spec.id+"/insertAll", []byte(insertBody))
		if status != http.StatusOK {
			t.Fatalf("insertAll %s -> %d: %s", spec.id, status, string(body))
		}
	}

	copyJob := `{
		"configuration":{
			"copy":{
				"sourceTables":[
					{"projectId":"` + projectID + `","datasetId":"` + datasetID + `","tableId":"t1"},
					{"projectId":"` + projectID + `","datasetId":"` + datasetID + `","tableId":"t2"}
				],
				"destinationTable":{"projectId":"` + projectID + `","datasetId":"` + datasetID + `","tableId":"merged"},
				"writeDisposition":"WRITE_EMPTY"
			}
		}
	}`
	status, body = doJSON(t, http.MethodPost, base+"/jobs", []byte(copyJob))
	if status != http.StatusOK {
		t.Fatalf("jobs.insert multi copy -> %d: %s", status, string(body))
	}

	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables/merged", nil)
	if status != http.StatusOK {
		t.Fatalf("tables.get merged -> %d: %s", status, string(body))
	}
	var got bqtypes.Table
	if err := json.Unmarshal(body, &got); err != nil {
		t.Fatalf("decode table: %v", err)
	}
	if got.NumRows != "2" {
		t.Fatalf("merged numRows = %q, want 2", got.NumRows)
	}
}
