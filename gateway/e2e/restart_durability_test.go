//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestViewInsertPersistenceAcrossEngineRestart creates a view via
// tables.insert (the client path), restarts emulator_main against the
// same data_dir, and verifies tables.list, tables.get, and SELECT through
// the view return identical rows.
func TestViewInsertPersistenceAcrossEngineRestart(t *testing.T) {
	dataDir := t.TempDir()
	env, err := launchEmulator(dataDir)
	if err != nil {
		t.Skip(err.Error())
	}

	const (
		projectID = "proj-view-insert-restart"
		datasetID = "ds_main"
		viewID    = "profiles_dedup"
	)
	base := env.httpServer.URL + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	status, body = doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":"CREATE TABLE `+datasetID+`.profiles_staging (`+
			`id INT64, inserted_at TIMESTAMP, name STRING)","useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("CREATE TABLE -> %d: %s", status, string(body))
	}

	insertAll := `{"kind":"bigquery#tableDataInsertAllRequest","rows":[` +
		`{"json":{"id":1,"inserted_at":"2026-01-01T00:00:00Z","name":"old-ada"}},` +
		`{"json":{"id":1,"inserted_at":"2026-02-01T00:00:00Z","name":"new-ada"}},` +
		`{"json":{"id":2,"inserted_at":"2026-01-15T00:00:00Z","name":"linus"}}]}`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables/profiles_staging/insertAll",
		[]byte(insertAll))
	if status != http.StatusOK {
		t.Fatalf("insertAll -> %d: %s", status, string(body))
	}

	viewQuery := "SELECT * EXCEPT(rn) FROM (" +
		"SELECT *, ROW_NUMBER() OVER (PARTITION BY id ORDER BY inserted_at DESC) AS rn " +
		"FROM " + datasetID + ".profiles_staging) WHERE rn = 1"
	viewBody := `{"tableReference":{"tableId":"` + viewID + `"},` +
		`"view":{"query":` + mustJSON(viewQuery) + `}}`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables", []byte(viewBody))
	if status != http.StatusOK {
		t.Fatalf("tables.insert(view) -> %d: %s", status, string(body))
	}

	assertViewRows := func(t *testing.T, urlBase string) {
		t.Helper()
		selectBody := `{"query":"SELECT id, name FROM ` + datasetID + `.` + viewID +
			` ORDER BY id","useLegacySql":false}`
		status, body := doJSON(t, http.MethodPost, urlBase+"/queries", []byte(selectBody))
		if status != http.StatusOK {
			t.Fatalf("SELECT -> %d: %s", status, string(body))
		}
		var run bqtypes.QueryResponse
		if err := json.Unmarshal(body, &run); err != nil {
			t.Fatalf("decode select: %v", err)
		}
		if len(run.Rows) != 2 {
			t.Fatalf("rows = %d, want 2", len(run.Rows))
		}
	}

	assertViewRows(t, base)

	env.tearDown()

	restarted, err := launchEmulator(dataDir)
	if err != nil {
		t.Fatalf("relaunch engine: %v", err)
	}
	t.Cleanup(restarted.tearDown)

	restartBase := restarted.httpServer.URL + "/bigquery/v2/projects/" + projectID

	status, body = doJSON(t, http.MethodGet,
		restartBase+"/datasets/"+datasetID+"/tables", nil)
	if status != http.StatusOK {
		t.Fatalf("post-restart tables.list -> %d: %s", status, string(body))
	}
	for _, id := range []string{"profiles_staging", viewID} {
		if !strings.Contains(string(body), id) {
			t.Fatalf("tables.list missing %q: %s", id, string(body))
		}
	}

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
	if got.View == nil || got.View.Query == "" {
		t.Errorf("view.query missing: %+v", got.View)
	}

	assertViewRows(t, restartBase)
}
