//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestTablePatchSchemaRelaxRequiredToNullable is the Edit Schema modal
// repro: PATCH mode REQUIRED→NULLABLE must persist on a subsequent GET.
func TestTablePatchSchemaRelaxRequiredToNullable(t *testing.T) {
	env := startEmulatorWithFlags(t, emulatorFlags{dataDir: t.TempDir()})

	const (
		projectID = "proj-schema-patch"
		datasetID = "ds_schema_patch"
		tableID   = "items"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	createBody := `{
		"tableReference":{"projectId":"` + projectID + `","datasetId":"` + datasetID + `","tableId":"` + tableID + `"},
		"schema":{"fields":[{"name":"id","type":"INT64","mode":"REQUIRED"}]}
	}`
	status, body = doJSON(t, http.MethodPost, base+"/datasets/"+datasetID+"/tables", []byte(createBody))
	if status != http.StatusOK {
		t.Fatalf("tables.insert -> %d: %s", status, string(body))
	}

	patchBody := `{"schema":{"fields":[{"name":"id","type":"INT64","mode":"NULLABLE"}]}}`
	tablesURL := base + "/datasets/" + datasetID + "/tables/" + tableID
	status, body = doJSON(t, http.MethodPatch, tablesURL, []byte(patchBody))
	if status != http.StatusOK {
		t.Fatalf("tables.patch -> %d: %s", status, string(body))
	}
	var patched bqtypes.Table
	if err := json.Unmarshal(body, &patched); err != nil {
		t.Fatalf("decode patch: %v", err)
	}
	if patched.Schema == nil || patched.Schema.Fields[0].Mode != "" {
		t.Fatalf("patch response schema = %#v, want NULLABLE id", patched.Schema)
	}

	status, body = doJSON(t, http.MethodGet, tablesURL, nil)
	if status != http.StatusOK {
		t.Fatalf("tables.get -> %d: %s", status, string(body))
	}
	var got bqtypes.Table
	if err := json.Unmarshal(body, &got); err != nil {
		t.Fatalf("decode get: %v", err)
	}
	if got.Schema == nil || got.Schema.Fields[0].Mode != "" {
		t.Fatalf("get schema = %#v, want NULLABLE id", got.Schema)
	}
}
