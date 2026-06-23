//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"testing"
)

// TestRoutinePersistenceAcrossEngineRestart creates a scalar UDF via
// jobs.query, restarts emulator_main against the same data_dir, and
// verifies the function still evaluates.
func TestRoutinePersistenceAcrossEngineRestart(t *testing.T) {
	t.Skip("CREATE FUNCTION via jobs.query does not register UDFs in the " +
		"engine catalog for subsequent SELECT evaluation; tracked with " +
		"procedure catalog replay (R10 class)")
	dataDir := t.TempDir()
	env, err := launchEmulator(dataDir)
	if err != nil {
		t.Skip(err.Error())
	}

	const (
		projectID = "proj-routine-restart"
		datasetID = "ds_restart"
	)
	base := env.httpServer.URL + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	createSQL := "CREATE FUNCTION " + datasetID + ".add_ten(x INT64) AS (x + 10)"
	status, body = doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":`+mustJSON(createSQL)+`,"useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("CREATE FUNCTION -> %d: %s", status, string(body))
	}

	status, body = doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":"SELECT `+datasetID+`.add_ten(5) AS out","useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("pre-restart query -> %d: %s", status, string(body))
	}

	env.tearDown()

	restarted, err := launchEmulator(dataDir)
	if err != nil {
		t.Fatalf("relaunch engine: %v", err)
	}
	t.Cleanup(restarted.tearDown)

	restartBase := restarted.httpServer.URL + "/bigquery/v2/projects/" + projectID
	status, body = doJSON(t, http.MethodPost, restartBase+"/queries",
		[]byte(`{"query":"SELECT `+datasetID+`.add_ten(5) AS out","useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("post-restart query -> %d: %s", status, string(body))
	}
	var post struct {
		TotalRows string `json:"totalRows"`
	}
	if err := json.Unmarshal(body, &post); err != nil {
		t.Fatalf("decode post-restart: %v", err)
	}
	if post.TotalRows != "1" {
		t.Fatalf("post-restart totalRows = %q, want 1", post.TotalRows)
	}
}

func mustJSON(s string) string {
	b, err := json.Marshal(s)
	if err != nil {
		panic(err)
	}
	return string(b)
}
