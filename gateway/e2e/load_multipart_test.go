//go:build integration

package e2e

import (
	"encoding/json"
	"io"
	"net/http"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestLoadJobMultipartCSVRoundTrip mirrors the UI Create Table upload flow:
// multipart jobs.insert LOAD → tabledata.list reads rows back.
func TestLoadJobMultipartCSVRoundTrip(t *testing.T) {
	env := startEmulatorWithFlags(t, emulatorFlags{dataDir: t.TempDir()})

	const (
		projectID = "proj-load-mp"
		datasetID = "ds_load_mp"
		tableID   = "people"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	meta := `{"configuration":{"load":{"destinationTable":{"projectId":"` + projectID +
		`","datasetId":"` + datasetID + `","tableId":"` + tableID +
		`"},"sourceFormat":"CSV","skipLeadingRows":"1","autodetect":true}}}`
	mpBody := strings.Join([]string{
		"--BOUNDARY",
		"Content-Type: application/json; charset=UTF-8",
		"",
		meta,
		"--BOUNDARY",
		"Content-Type: text/csv",
		"",
		"full_name,age\nAda Lovelace,36\n",
		"--BOUNDARY--",
		"",
	}, "\r\n")

	req, err := http.NewRequest(http.MethodPost,
		env.URL()+"/upload/bigquery/v2/projects/"+projectID+"/jobs?uploadType=multipart",
		strings.NewReader(mpBody))
	if err != nil {
		t.Fatalf("NewRequest: %v", err)
	}
	req.Header.Set("Content-Type", `multipart/related; boundary="BOUNDARY"`)
	resp, err := http.DefaultClient.Do(req)
	if err != nil {
		t.Fatalf("multipart load: %v", err)
	}
	defer resp.Body.Close()
	respBody, _ := io.ReadAll(resp.Body)
	if resp.StatusCode != http.StatusOK {
		t.Fatalf("multipart load -> %d: %s", resp.StatusCode, string(respBody))
	}

	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables/"+tableID+"/data", nil)
	if status != http.StatusOK {
		t.Fatalf("tabledata.list -> %d: %s", status, string(body))
	}
	var page bqtypes.TableDataList
	if err := json.Unmarshal(body, &page); err != nil {
		t.Fatalf("decode list: %v", err)
	}
	if page.TotalRows != "1" {
		t.Fatalf("totalRows = %q, want 1", page.TotalRows)
	}
	if len(page.Rows) != 1 || len(page.Rows[0].F) < 2 {
		t.Fatalf("rows = %#v", page.Rows)
	}
	name, _ := page.Rows[0].F[0].V.(string)
	if name != "Ada Lovelace" {
		t.Errorf("name = %q, want Ada Lovelace", name)
	}
}
