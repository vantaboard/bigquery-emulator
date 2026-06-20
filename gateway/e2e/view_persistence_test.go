//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestViewPersistenceAcrossEngineRestart creates a view via jobs.query,
// restarts emulator_main against the same data_dir, and verifies
// tables.get still returns type=VIEW and view.query without the gateway
// metadata overlay.
func TestViewPersistenceAcrossEngineRestart(t *testing.T) {
	dataDir := t.TempDir()
	env, err := launchEmulator(dataDir)
	if err != nil {
		t.Skip(err.Error())
	}

	const (
		projectID = "proj-view-restart"
		datasetID = "ds_view_restart"
		viewID    = "persisted_view"
		viewSQL   = "SELECT 42 AS answer"
	)
	base := env.httpServer.URL + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	createBody := `{"query":"CREATE OR REPLACE VIEW ` + datasetID + `.` + viewID +
		` AS ` + viewSQL + `","useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost, base+"/queries", []byte(createBody))
	if status != http.StatusOK {
		t.Fatalf("CREATE VIEW -> %d: %s", status, string(body))
	}

	env.tearDown()

	restarted, err := launchEmulator(dataDir)
	if err != nil {
		t.Fatalf("relaunch engine: %v", err)
	}
	t.Cleanup(restarted.tearDown)

	restartBase := restarted.httpServer.URL + "/bigquery/v2/projects/" + projectID
	status, body = doJSON(t, http.MethodGet,
		restartBase+"/datasets/"+datasetID+"/tables/"+viewID, nil)
	if status != http.StatusOK {
		t.Fatalf("post-restart tables.get -> %d: %s", status, string(body))
	}
	var got bqtypes.Table
	if err := json.Unmarshal(body, &got); err != nil {
		t.Fatalf("decode table: %v", err)
	}
	if got.Type != "VIEW" {
		t.Errorf("type = %q, want VIEW", got.Type)
	}
	if got.View == nil || got.View.Query != viewSQL {
		t.Errorf("view.query = %+v, want %q", got.View, viewSQL)
	}

	selectBody := `{"query":"SELECT answer FROM ` + datasetID + `.` + viewID +
		`","useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost, restartBase+"/queries", []byte(selectBody))
	if status != http.StatusOK {
		t.Fatalf("post-restart SELECT -> %d: %s", status, string(body))
	}
	var run bqtypes.QueryResponse
	if err := json.Unmarshal(body, &run); err != nil {
		t.Fatalf("decode select: %v", err)
	}
	if len(run.Rows) != 1 {
		t.Fatalf("rows = %d, want 1", len(run.Rows))
	}
}
