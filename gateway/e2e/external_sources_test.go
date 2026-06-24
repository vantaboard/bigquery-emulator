//go:build integration

package e2e

import (
	"net/http"
	"strings"
	"testing"
)

// TestExternalBigtableTableInsertE2E locks Bigtable external table insert +
// metadata round-trip for the Create Table UI flow.
func TestExternalBigtableTableInsertE2E(t *testing.T) {
	env := startEmulator(t)

	const (
		projectID = "proj-ext-bt"
		datasetID = "ds_ext"
		tableID   = "bt_table"
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
		"schema":{"fields":[{"name":"cf","type":"STRING"}]},
		"externalDataConfiguration":{
			"sourceFormat":"BIGTABLE",
			"sourceUris":["https://googleapis.com/bigtable/projects/p/instances/i/tables/t"]
		}
	}`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables", []byte(tableBody))
	if status != http.StatusOK {
		t.Fatalf("tables.insert -> %d: %s", status, string(body))
	}
	if !strings.Contains(string(body), `"type":"EXTERNAL"`) &&
		!strings.Contains(string(body), `"type": "EXTERNAL"`) {
		t.Fatalf("insert response missing EXTERNAL type: %s", string(body))
	}

	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables/"+tableID, nil)
	if status != http.StatusOK {
		t.Fatalf("tables.get -> %d: %s", status, string(body))
	}
	if !strings.Contains(string(body), "BIGTABLE") {
		t.Fatalf("GET missing BIGTABLE config: %s", string(body))
	}
}

// TestLoadJobS3ErrorShapeE2E asserts s3:// load jobs return a stable error
// message when S3_ENDPOINT is unset.
func TestLoadJobS3ErrorShapeE2E(t *testing.T) {
	env := startEmulator(t)

	const (
		projectID = "proj-s3-load"
		datasetID = "ds_load"
		tableID   = "t_load"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	jobBody := `{
		"configuration":{"load":{
			"sourceUris":["s3://bucket/x.csv"],
			"destinationTable":{"projectId":"` + projectID +
		`","datasetId":"` + datasetID + `","tableId":"` + tableID + `"},
			"sourceFormat":"CSV"
		}}
	}`
	status, body = doJSON(t, http.MethodPost, base+"/jobs", []byte(jobBody))
	if status != http.StatusOK {
		t.Fatalf("jobs.insert -> %d: %s", status, string(body))
	}
	if !strings.Contains(string(body), "S3_ENDPOINT") &&
		!strings.Contains(string(body), "errorResult") {
		t.Fatalf("expected load error mentioning S3_ENDPOINT or errorResult: %s", string(body))
	}
}
