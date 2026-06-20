//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"testing"
)

// TestRoutineCreateViaQueryListAndGet creates a scalar UDF through
// jobs.query and verifies routines.list and routines.get surface it.
func TestRoutineCreateViaQueryListAndGet(t *testing.T) {
	dataDir := t.TempDir()
	env, err := launchEmulator(dataDir)
	if err != nil {
		t.Skip(err.Error())
	}
	t.Cleanup(env.tearDown)

	const (
		projectID = "proj-routine-rest"
		datasetID = "ds_routines"
		routineID = "mul_two"
	)
	base := env.httpServer.URL + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	createSQL := "CREATE FUNCTION " + datasetID + "." + routineID + "(x INT64) RETURNS INT64 AS (x * 2)"
	status, body = doJSON(t, http.MethodPost, base+"/queries",
		[]byte(`{"query":`+mustJSON(createSQL)+`,"useLegacySql":false}`))
	if status != http.StatusOK {
		t.Fatalf("CREATE FUNCTION -> %d: %s", status, string(body))
	}

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
			RoutineType      string `json:"routineType"`
			Language         string `json:"language"`
			CreationTime     string `json:"creationTime"`
			LastModifiedTime string `json:"lastModifiedTime"`
		} `json:"routines"`
	}
	if err := json.Unmarshal(body, &listed); err != nil {
		t.Fatalf("decode list: %v", err)
	}
	if len(listed.Routines) != 1 {
		t.Fatalf("listed routines = %d, want 1; body=%s", len(listed.Routines), string(body))
	}
	entry := listed.Routines[0]
	if entry.RoutineReference.RoutineID != routineID {
		t.Errorf("routineId = %q, want %q", entry.RoutineReference.RoutineID, routineID)
	}
	if entry.RoutineType != "SCALAR_FUNCTION" {
		t.Errorf("routineType = %q, want SCALAR_FUNCTION", entry.RoutineType)
	}
	if entry.Language != "SQL" {
		t.Errorf("language = %q, want SQL", entry.Language)
	}
	if entry.CreationTime == "" || entry.CreationTime == "0" {
		t.Errorf("creationTime = %q, want non-zero", entry.CreationTime)
	}
	if entry.LastModifiedTime == "" || entry.LastModifiedTime == "0" {
		t.Errorf("lastModifiedTime = %q, want non-zero", entry.LastModifiedTime)
	}

	getURL := listURL + "/" + routineID
	status, body = doJSON(t, http.MethodGet, getURL, nil)
	if status != http.StatusOK {
		t.Fatalf("routines.get -> %d: %s", status, string(body))
	}
	var got struct {
		DefinitionBody   string `json:"definitionBody"`
		CreationTime     string `json:"creationTime"`
		LastModifiedTime string `json:"lastModifiedTime"`
		ReturnType       struct {
			TypeKind string `json:"typeKind"`
		} `json:"returnType"`
	}
	if err := json.Unmarshal(body, &got); err != nil {
		t.Fatalf("decode get: %v", err)
	}
	if got.DefinitionBody != "x * 2" {
		t.Errorf("definitionBody = %q, want %q", got.DefinitionBody, "x * 2")
	}
	if got.ReturnType.TypeKind != "INT64" {
		t.Errorf("returnType.typeKind = %q, want INT64", got.ReturnType.TypeKind)
	}
	if got.CreationTime == "" || got.CreationTime == "0" {
		t.Errorf("creationTime = %q, want non-zero", got.CreationTime)
	}
	if got.LastModifiedTime == "" || got.LastModifiedTime == "0" {
		t.Errorf("lastModifiedTime = %q, want non-zero", got.LastModifiedTime)
	}
}
