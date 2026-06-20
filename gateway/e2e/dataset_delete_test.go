//go:build integration

package e2e

import (
	"net/http"
	"testing"
)

// TestDatasetDeleteWithContentsE2E locks DELETE ?deleteContents=true: a
// dataset with a table is removed and follow-up tables.list /
// datasets.get return 404.
func TestDatasetDeleteWithContentsE2E(t *testing.T) {
	env := startEmulator(t)

	const (
		projectID = "proj-del-ds"
		datasetID = "ds_delete_e2e"
		tableID   = "t1"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	tableBody := `{
		"tableReference":{"projectId":"` + projectID +
		`","datasetId":"` + datasetID +
		`","tableId":"` + tableID + `"},
		"schema":{"fields":[{"name":"id","type":"INT64"}]}
	}`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables", []byte(tableBody))
	if status != http.StatusOK {
		t.Fatalf("tables.insert -> %d: %s", status, string(body))
	}

	status, body = doJSON(t, http.MethodDelete,
		base+"/datasets/"+datasetID+"?deleteContents=true", nil)
	if status != http.StatusOK {
		t.Fatalf("datasets.delete -> %d: %s", status, string(body))
	}

	status, body = doJSON(t, http.MethodGet, base+"/datasets/"+datasetID, nil)
	if status != http.StatusNotFound {
		t.Fatalf("datasets.get after delete -> %d, want 404: %s", status, string(body))
	}

	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables", nil)
	if status != http.StatusNotFound {
		t.Fatalf("tables.list after delete -> %d, want 404: %s", status, string(body))
	}
}
