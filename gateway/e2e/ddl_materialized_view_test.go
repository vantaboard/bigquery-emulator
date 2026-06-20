//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestDDLCreateMaterializedViewRoundTrip pins CREATE MATERIALIZED VIEW
// visibility in tables.list / tables.get after jobs.query DDL.
func TestDDLCreateMaterializedViewRoundTrip(t *testing.T) {
	env := startEmulatorWithFlags(t, emulatorFlags{
		dataDir: t.TempDir(),
	})

	const (
		projectID = "proj-ddl-mv"
		datasetID = "ds_mv"
		baseID    = "base_t"
		mvID      = "mv_agg"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	createTable := `{"query":"CREATE TABLE ` + datasetID + `.` + baseID +
		` (x INT64)","useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost, base+"/queries", []byte(createTable))
	if status != http.StatusOK {
		t.Fatalf("CREATE TABLE -> %d: %s", status, string(body))
	}

	insert := `{"query":"INSERT INTO ` + datasetID + `.` + baseID +
		` VALUES (1), (2), (3)","useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost, base+"/queries", []byte(insert))
	if status != http.StatusOK {
		t.Fatalf("INSERT -> %d: %s", status, string(body))
	}

	mvSQL := "SELECT SUM(x) AS total FROM " + datasetID + "." + baseID
	createMV := `{"query":"CREATE MATERIALIZED VIEW ` + datasetID + `.` + mvID +
		` AS ` + mvSQL + `","useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost, base+"/queries", []byte(createMV))
	if status != http.StatusOK {
		t.Fatalf("CREATE MATERIALIZED VIEW -> %d: %s", status, string(body))
	}

	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables", nil)
	if status != http.StatusOK {
		t.Fatalf("tables.list -> %d: %s", status, string(body))
	}
	var list struct {
		Tables []struct {
			TableReference bqtypes.TableReference `json:"tableReference"`
			Type           string                 `json:"type"`
		} `json:"tables"`
	}
	if err := json.Unmarshal(body, &list); err != nil {
		t.Fatalf("decode table list: %v", err)
	}
	found := false
	for _, item := range list.Tables {
		if item.TableReference.TableID == mvID {
			found = true
			if item.Type != "MATERIALIZED_VIEW" {
				t.Errorf("list type = %q, want MATERIALIZED_VIEW", item.Type)
			}
		}
	}
	if !found {
		t.Fatalf("materialized view %q missing from tables.list: %s", mvID, string(body))
	}

	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables/"+mvID, nil)
	if status != http.StatusOK {
		t.Fatalf("tables.get -> %d: %s", status, string(body))
	}
	var got bqtypes.Table
	if err := json.Unmarshal(body, &got); err != nil {
		t.Fatalf("decode table: %v", err)
	}
	if got.Type != "MATERIALIZED_VIEW" {
		t.Errorf("type = %q, want MATERIALIZED_VIEW", got.Type)
	}
	if got.MaterializedView == nil || got.MaterializedView.Query != mvSQL {
		t.Errorf("materializedView.query = %+v, want %q", got.MaterializedView, mvSQL)
	}
}
