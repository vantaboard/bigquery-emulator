package server_test

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/server"
	"github.com/vantaboard/bigquery-emulator/types"
)

func TestEmulatorProjectsCRUD(t *testing.T) {
	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		t.Fatal(err)
	}
	defer bqServer.Close()

	if err := bqServer.Load(server.StructSource(types.NewProject("seed"))); err != nil {
		t.Fatal(err)
	}

	ts := bqServer.TestServer()
	defer ts.Close()

	base := strings.TrimSuffix(ts.URL, "/")
	client := &http.Client{}

	// GET list includes seed
	resp, err := client.Get(base + "/emulator/v1/projects")
	if err != nil {
		t.Fatal(err)
	}
	body, _ := io.ReadAll(resp.Body)
	resp.Body.Close()
	if resp.StatusCode != http.StatusOK {
		t.Fatalf("GET list: want 200, got %d: %s", resp.StatusCode, body)
	}
	var list []string
	if err := json.Unmarshal(body, &list); err != nil {
		t.Fatal(err)
	}
	if len(list) != 1 || list[0] != "seed" {
		t.Fatalf("GET list: want [seed], got %#v", list)
	}

	// POST create
	createBody := `{"id":"p2"}`
	resp, err = client.Post(base+"/emulator/v1/projects", "application/json", strings.NewReader(createBody))
	if err != nil {
		t.Fatal(err)
	}
	body, _ = io.ReadAll(resp.Body)
	resp.Body.Close()
	if resp.StatusCode != http.StatusCreated {
		t.Fatalf("POST create: want 201, got %d: %s", resp.StatusCode, body)
	}

	// POST duplicate -> 409
	resp, err = client.Post(base+"/emulator/v1/projects", "application/json", strings.NewReader(createBody))
	if err != nil {
		t.Fatal(err)
	}
	body, _ = io.ReadAll(resp.Body)
	resp.Body.Close()
	if resp.StatusCode != http.StatusConflict {
		t.Fatalf("POST duplicate: want 409, got %d: %s", resp.StatusCode, body)
	}

	// GET one
	resp, err = client.Get(base + "/emulator/v1/projects/p2")
	if err != nil {
		t.Fatal(err)
	}
	body, _ = io.ReadAll(resp.Body)
	resp.Body.Close()
	if resp.StatusCode != http.StatusOK {
		t.Fatalf("GET one: want 200, got %d: %s", resp.StatusCode, body)
	}

	// DELETE non-empty project (seed has no datasets — add dataset via loading another struct)
	if err := bqServer.Load(server.StructSource(
		types.NewProject("busy", types.NewDataset("ds1")),
	)); err != nil {
		t.Fatal(err)
	}
	req, _ := http.NewRequest(http.MethodDelete, base+"/emulator/v1/projects/busy", nil)
	resp, err = client.Do(req)
	if err != nil {
		t.Fatal(err)
	}
	body, _ = io.ReadAll(resp.Body)
	resp.Body.Close()
	if resp.StatusCode != http.StatusConflict {
		t.Fatalf("DELETE non-empty: want 409, got %d: %s", resp.StatusCode, body)
	}

	// DELETE empty project p2
	req, _ = http.NewRequest(http.MethodDelete, base+"/emulator/v1/projects/p2", nil)
	resp, err = client.Do(req)
	if err != nil {
		t.Fatal(err)
	}
	body, _ = io.ReadAll(resp.Body)
	resp.Body.Close()
	if resp.StatusCode != http.StatusNoContent {
		t.Fatalf("DELETE p2: want 204, got %d: %s", resp.StatusCode, body)
	}

	// GET missing -> 404
	resp, err = client.Get(base + "/emulator/v1/projects/p2")
	if err != nil {
		t.Fatal(err)
	}
	body, _ = io.ReadAll(resp.Body)
	resp.Body.Close()
	if resp.StatusCode != http.StatusNotFound {
		t.Fatalf("GET missing: want 404, got %d: %s", resp.StatusCode, body)
	}
}

func TestLoadJobMissingDestinationProjectReturns404(t *testing.T) {
	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		t.Fatal(err)
	}
	defer bqServer.Close()

	if err := bqServer.Load(server.StructSource(types.NewProject("url-project"))); err != nil {
		t.Fatal(err)
	}

	ts := bqServer.TestServer()
	defer ts.Close()
	base := strings.TrimSuffix(ts.URL, "/")
	client := &http.Client{}

	jobID := "load-job-missing-proj"
	uploadJob := map[string]interface{}{
		"jobReference": map[string]interface{}{
			"projectId": "url-project",
			"jobId":     jobID,
		},
		"configuration": map[string]interface{}{
			"load": map[string]interface{}{
				"destinationTable": map[string]interface{}{
					"projectId": "other-project",
					"datasetId": "ds1",
					"tableId":   "t1",
				},
				"sourceFormat": "NEWLINE_DELIMITED_JSON",
			},
		},
	}
	raw, err := json.Marshal(uploadJob)
	if err != nil {
		t.Fatal(err)
	}

	postURL := fmt.Sprintf("%s/upload/bigquery/v2/projects/url-project/jobs?uploadType=resumable", base)
	resp, err := client.Post(postURL, "application/json", bytes.NewReader(raw))
	if err != nil {
		t.Fatal(err)
	}
	if resp.StatusCode != http.StatusOK {
		b, _ := io.ReadAll(resp.Body)
		resp.Body.Close()
		t.Fatalf("POST resumable: want 200, got %d: %s", resp.StatusCode, b)
	}
	resp.Body.Close()

	putURL := fmt.Sprintf("%s/upload/bigquery/v2/projects/url-project/jobs?uploadType=resumable&upload_id=%s", base, jobID)
	req, err := http.NewRequest(http.MethodPut, putURL, strings.NewReader(`{"x":1}`))
	if err != nil {
		t.Fatal(err)
	}
	req.Header.Set("Content-Type", "application/octet-stream")
	resp, err = client.Do(req)
	if err != nil {
		t.Fatal(err)
	}
	body, _ := io.ReadAll(resp.Body)
	resp.Body.Close()
	if resp.StatusCode != http.StatusNotFound {
		t.Fatalf("PUT upload to missing dest project: want 404, got %d: %s", resp.StatusCode, body)
	}
}
