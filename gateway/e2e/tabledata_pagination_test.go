//go:build integration

package e2e

import (
	"encoding/json"
	"fmt"
	"net/http"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestTableDataListMultiPagePagination seeds more rows than one page holds,
// then walks pageToken until exhausted and asserts every row appears once.
func TestTableDataListMultiPagePagination(t *testing.T) {
	env := startEmulatorWithFlags(t, emulatorFlags{dataDir: t.TempDir()})

	const (
		projectID = "proj-td-page"
		datasetID = "ds_td_page"
		tableID   = "nums"
		pageSize  = 7
		rowCount  = 23
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
		"schema":{"fields":[{"name":"n","type":"INT64","mode":"REQUIRED"}]}
	}`
	status, body = doJSON(t, http.MethodPost, base+"/datasets/"+datasetID+"/tables", []byte(createBody))
	if status != http.StatusOK {
		t.Fatalf("tables.insert -> %d: %s", status, string(body))
	}

	rowsJSON := `{"kind":"bigquery#tableDataInsertAllRequest","rows":[`
	for i := 1; i <= rowCount; i++ {
		if i > 1 {
			rowsJSON += ","
		}
		rowsJSON += fmt.Sprintf(`{"json":{"n":%d}}`, i)
	}
	rowsJSON += `]}`
	dataURL := base + "/datasets/" + datasetID + "/tables/" + tableID
	status, body = doJSON(t, http.MethodPost, dataURL+"/insertAll", []byte(rowsJSON))
	if status != http.StatusOK {
		t.Fatalf("insertAll -> %d: %s", status, string(body))
	}

	seen := map[string]bool{}
	var totalRows string
	pageToken := ""
	for page := 0; page < 10; page++ {
		url := dataURL + "/data?maxResults=" + fmt.Sprint(pageSize)
		if pageToken != "" {
			url += "&pageToken=" + pageToken
		}
		status, body = doJSON(t, http.MethodGet, url, nil)
		if status != http.StatusOK {
			t.Fatalf("tabledata.list page %d -> %d: %s", page, status, string(body))
		}
		var pageResp bqtypes.TableDataList
		if err := json.Unmarshal(body, &pageResp); err != nil {
			t.Fatalf("decode page %d: %v", page, err)
		}
		if pageResp.TotalRows == "" {
			t.Fatalf("page %d missing totalRows", page)
		}
		if totalRows == "" {
			totalRows = pageResp.TotalRows
		} else if pageResp.TotalRows != totalRows {
			t.Fatalf("totalRows changed: %q vs %q", pageResp.TotalRows, totalRows)
		}
		if pageResp.Etag == "" {
			t.Errorf("page %d missing etag", page)
		}
		for _, r := range pageResp.Rows {
			if len(r.F) != 1 {
				t.Fatalf("row cells = %d, want 1", len(r.F))
			}
			idStr, _ := r.F[0].V.(string)
			if seen[idStr] {
				t.Fatalf("duplicate row n=%s on page %d", idStr, page)
			}
			seen[idStr] = true
		}
		pageToken = pageResp.PageToken
		if pageToken == "" {
			break
		}
	}
	if len(seen) != rowCount {
		t.Fatalf("collected %d rows, want %d (totalRows=%s)", len(seen), rowCount, totalRows)
	}
}
