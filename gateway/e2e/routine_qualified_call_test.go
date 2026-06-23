//go:build integration

package e2e

import (
	"encoding/json"
	"fmt"
	"net/http"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestRoutineQualifiedCall verifies CREATE FUNCTION followed by
// dataset-qualified and fully-qualified SELECT calls return the
// expected scalar (plan 10 UI repro).
func TestRoutineQualifiedCall(t *testing.T) {
	dataDir := t.TempDir()
	env, err := launchEmulator(dataDir)
	if err != nil {
		t.Skip(err.Error())
	}
	t.Cleanup(env.tearDown)

	const (
		projectID = "local-project"
		datasetID = "udf_ds"
		routineID = "add_one"
	)
	base := env.httpServer.URL + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	createSQL := "CREATE FUNCTION " + datasetID + "." + routineID +
		"(x INT64) RETURNS INT64 AS (x + 1)"
	status, body = doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":`+mustJSON(createSQL)+`,"useLegacySql":false,"defaultDataset":{"datasetId":"`+datasetID+`"}}`))
	if status != http.StatusOK {
		t.Fatalf("CREATE FUNCTION -> %d: %s", status, string(body))
	}

	cases := []struct {
		name string
		sql  string
	}{
		{
			name: "unqualified",
			sql:  "SELECT " + routineID + "(5) AS out",
		},
		{
			name: "dataset_qualified",
			sql:  "SELECT " + datasetID + "." + routineID + "(5) AS out",
		},
		{
			name: "dotted_backtick",
			sql:  "SELECT `" + projectID + "." + datasetID + "." + routineID + "`(5) AS out",
		},
		{
			name: "three_segment_backticks",
			sql: fmt.Sprintf("SELECT `%s`.`%s`.`%s`(5) AS out",
				projectID, datasetID, routineID),
		},
	}

	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			status, body := doJSON(t, http.MethodPost, base+"/queries",
				[]byte(`{"query":`+mustJSON(tc.sql)+`,"useLegacySql":false,"defaultDataset":{"datasetId":"`+datasetID+`"}}`))
			if status != http.StatusOK {
				t.Fatalf("jobs.query -> %d: %s", status, string(body))
			}
			var resp bqtypes.QueryResponse
			if err := json.Unmarshal(body, &resp); err != nil {
				t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
			}
			if resp.TotalRows != "1" {
				t.Fatalf("totalRows = %q, want 1", resp.TotalRows)
			}
			if len(resp.Rows) != 1 || len(resp.Rows[0].F) != 1 {
				t.Fatalf("rows shape = %+v, want one row with one cell", resp.Rows)
			}
			if v := resp.Rows[0].F[0].V; v != "6" {
				t.Errorf("rows[0].f[0].v = %v, want %q", v, "6")
			}
		})
	}

	// Regression: routines.list / routines.get unchanged after qualified calls.
	listURL := base + "/datasets/" + datasetID + "/routines"
	status, body = doJSON(t, http.MethodGet, listURL, nil)
	if status != http.StatusOK {
		t.Fatalf("routines.list -> %d: %s", status, string(body))
	}
	var listed struct {
		Routines []struct {
			RoutineReference struct {
				RoutineID string `json:"routineId"`
			} `json:"routineReference"`
		} `json:"routines"`
	}
	if err := json.Unmarshal(body, &listed); err != nil {
		t.Fatalf("decode list: %v", err)
	}
	if len(listed.Routines) != 1 {
		t.Fatalf("listed routines = %d, want 1", len(listed.Routines))
	}
	if listed.Routines[0].RoutineReference.RoutineID != routineID {
		t.Errorf("routineId = %q, want %q", listed.Routines[0].RoutineReference.RoutineID, routineID)
	}

	getURL := listURL + "/" + routineID
	status, body = doJSON(t, http.MethodGet, getURL, nil)
	if status != http.StatusOK {
		t.Fatalf("routines.get -> %d: %s", status, string(body))
	}
}
