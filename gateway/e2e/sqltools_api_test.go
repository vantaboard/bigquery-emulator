//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"strings"
	"testing"
)

// TestSQLToolsCapabilitiesE2E locks gap #14: capabilities returns sqlTools
// when the gateway is started with --enable-sql-tools-api.
func TestSQLToolsCapabilitiesE2E(t *testing.T) {
	env := startEmulatorWithFlags(t, emulatorFlags{
		dataDir:           t.TempDir(),
		enableSQLToolsAPI: true,
	})

	status, body := doJSON(t, http.MethodGet, env.URL()+"/api/emulator/sql/capabilities", nil)
	if status != http.StatusOK {
		t.Fatalf("capabilities -> %d: %s", status, string(body))
	}
	if !strings.Contains(string(body), `"sqlTools":true`) {
		t.Fatalf("body = %s", string(body))
	}
}

// TestSQLToolsAnalyzeE2E locks gap #16: analyze returns referencedTables for
// a qualified table reference.
func TestSQLToolsAnalyzeE2E(t *testing.T) {
	env := startEmulatorWithFlags(t, emulatorFlags{
		dataDir:           t.TempDir(),
		enableSQLToolsAPI: true,
	})

	const (
		projectID = "proj-sqltools"
		datasetID = "ds_st"
		tableID   = "events"
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
	status, body = doJSON(t, http.MethodPost, base+"/datasets/"+datasetID+"/tables", []byte(tableBody))
	if status != http.StatusOK {
		t.Fatalf("tables.insert -> %d: %s", status, string(body))
	}

	analyzeBody := `{
		"projectId":"` + projectID + `",
		"defaultDatasetId":"` + datasetID + `",
		"sql":"SELECT id FROM ` + projectID + `.` + datasetID + `.` + tableID + `"
	}`
	status, body = doJSON(t, http.MethodPost, env.URL()+"/api/emulator/sql/analyze", []byte(analyzeBody))
	if status != http.StatusOK {
		t.Fatalf("analyze -> %d: %s", status, string(body))
	}
	if !strings.Contains(string(body), `"referencedTables"`) {
		t.Fatalf("body = %s", string(body))
	}
	if !strings.Contains(string(body), `"tableId":"`+tableID+`"`) {
		t.Fatalf("body = %s", string(body))
	}
}

// TestSQLToolsCompleteRoutineCandidateE2E locks gap #15 polish: user UDFs
// appear as kind routine with fqn in /complete.
func TestSQLToolsCompleteRoutineCandidateE2E(t *testing.T) {
	env := startEmulatorWithFlags(t, emulatorFlags{
		dataDir:           t.TempDir(),
		enableSQLToolsAPI: true,
	})

	const (
		projectID = "proj-sqltools-complete"
		datasetID = "udf_ds"
		routineID = "add_one"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	createSQL := "CREATE FUNCTION " + datasetID + "." + routineID + "(x INT64) RETURNS INT64 AS (x + 1)"
	status, body = doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":`+mustJSON(createSQL)+`,"useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("CREATE FUNCTION -> %d: %s", status, string(body))
	}

	completeBody := `{
		"projectId":"` + projectID + `",
		"defaultDatasetId":"` + datasetID + `",
		"sql":"SELECT add_",
		"cursorByteOffset":10
	}`
	status, body = doJSON(t, http.MethodPost, env.URL()+"/api/emulator/sql/complete", []byte(completeBody))
	if status != http.StatusOK {
		t.Fatalf("complete -> %d: %s", status, string(body))
	}
	bodyStr := string(body)
	if !strings.Contains(bodyStr, `"kind":"routine"`) {
		t.Fatalf("body = %s", bodyStr)
	}
	if !strings.Contains(bodyStr, `"fqn":`) {
		t.Fatalf("body = %s", bodyStr)
	}
	var parsed struct {
		Candidates []struct {
			Label string `json:"label"`
			Kind  string `json:"kind"`
			Fqn   string `json:"fqn"`
		} `json:"candidates"`
	}
	if err := json.Unmarshal(body, &parsed); err != nil {
		t.Fatalf("decode: %v", err)
	}
	found := false
	for _, c := range parsed.Candidates {
		if c.Label == routineID && c.Kind == "routine" && c.Fqn != "" {
			found = true
			break
		}
	}
	if !found {
		t.Fatalf("candidates = %+v", parsed.Candidates)
	}
}
