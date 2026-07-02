//go:build integration

package e2e

import (
	"net/http"
	"strings"
	"testing"
)

// TestDatasetUndeleteRoundTripE2E deletes a dataset with contents via
// REST, restores it with datasets.undelete, and verifies table rows.
func TestDatasetUndeleteRoundTripE2E(t *testing.T) {
	env := startEmulator(t)

	const (
		projectID = "proj-undelete-ds"
		datasetID = "ds_undelete_e2e"
		tableID   = "items"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	status, body = doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":"CREATE TABLE `+datasetID+`.`+tableID+
			` (id INT64, name STRING)","useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("CREATE TABLE -> %d: %s", status, string(body))
	}

	insertAll := `{"kind":"bigquery#tableDataInsertAllRequest","rows":[` +
		`{"json":{"id":1,"name":"ada"}}]}`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables/"+tableID+"/insertAll",
		[]byte(insertAll))
	if status != http.StatusOK {
		t.Fatalf("insertAll -> %d: %s", status, string(body))
	}

	status, body = doJSON(t, http.MethodDelete,
		base+"/datasets/"+datasetID+"?deleteContents=true", nil)
	if status != http.StatusOK {
		t.Fatalf("datasets.delete -> %d: %s", status, string(body))
	}

	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+":undelete", nil)
	if status != http.StatusOK {
		t.Fatalf("datasets.undelete -> %d: %s", status, string(body))
	}

	status, body = doJSON(t, http.MethodGet, base+"/datasets/"+datasetID, nil)
	if status != http.StatusOK {
		t.Fatalf("datasets.get after undelete -> %d: %s", status, string(body))
	}

	selectBody := `{"query":"SELECT id, name FROM ` + datasetID + `.` + tableID +
		`","useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost, base+"/queries", []byte(selectBody))
	if status != http.StatusOK {
		t.Fatalf("SELECT after undelete -> %d: %s", status, string(body))
	}
	if !strings.Contains(string(body), `"ada"`) || !strings.Contains(string(body), `"1"`) {
		t.Fatalf("SELECT body missing restored row: %s", string(body))
	}
}
