//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestDDLCreateViewRoundTrip pins CREATE OR REPLACE VIEW visibility in
// tables.list / tables.get after jobs.query DDL, matching the
// bigquery-emulator-ui Save view flow.
func TestDDLCreateViewRoundTrip(t *testing.T) {
	env := startEmulatorWithFlags(t, emulatorFlags{
		dataDir: t.TempDir(),
	})

	const (
		projectID = "proj-ddl-view"
		datasetID = "ds_view"
		viewID    = "my_view"
		viewSQL   = "SELECT 1 AS x"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

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
		t.Fatalf("jobs.query (CREATE VIEW) -> %d: %s", status, string(body))
	}
	var run bqtypes.QueryResponse
	if err := json.Unmarshal(body, &run); err != nil {
		t.Fatalf("decode QueryResponse: %v", err)
	}
	if !run.JobComplete {
		t.Errorf("jobComplete = false, want true")
	}
	if run.Statistics == nil || run.Statistics.Query == nil ||
		run.Statistics.Query.StatementType != "CREATE_VIEW" {
		t.Errorf("statementType = %+v, want CREATE_VIEW", run.Statistics)
	}

	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables", nil)
	if status != http.StatusOK {
		t.Fatalf("tables.list -> %d: %s", status, string(body))
	}
	var list struct {
		Tables []struct {
			TableReference bqtypes.TableReference  `json:"tableReference"`
			Type           string                  `json:"type"`
			View           *bqtypes.ViewDefinition `json:"view"`
		} `json:"tables"`
	}
	if err := json.Unmarshal(body, &list); err != nil {
		t.Fatalf("decode table list: %v", err)
	}
	found := false
	for _, item := range list.Tables {
		if item.TableReference.TableID == viewID {
			found = true
			if item.Type != "VIEW" {
				t.Errorf("list type = %q, want VIEW", item.Type)
			}
			if item.View == nil || item.View.Query != viewSQL {
				t.Errorf("list view.query = %+v, want %q", item.View, viewSQL)
			}
		}
	}
	if !found {
		t.Fatalf("view %q missing from tables.list: %s", viewID, string(body))
	}

	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables/"+viewID, nil)
	if status != http.StatusOK {
		t.Fatalf("tables.get -> %d: %s", status, string(body))
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

	selectBody := `{"query":"SELECT x FROM ` + datasetID + `.` + viewID +
		`","useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost, base+"/queries", []byte(selectBody))
	if status != http.StatusOK {
		t.Fatalf("SELECT from view -> %d: %s", status, string(body))
	}
	if err := json.Unmarshal(body, &run); err != nil {
		t.Fatalf("decode select: %v", err)
	}
	if len(run.Rows) != 1 {
		t.Fatalf("rows = %d, want 1", len(run.Rows))
	}

	dropBody := `{"query":"DROP VIEW ` + datasetID + `.` + viewID + `","useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost, base+"/queries", []byte(dropBody))
	if status != http.StatusOK {
		t.Fatalf("DROP VIEW -> %d: %s", status, string(body))
	}
	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables/"+viewID, nil)
	if status != http.StatusNotFound {
		t.Errorf("tables.get after drop -> %d, want 404; body=%s", status, string(body))
	}

	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables", nil)
	if status != http.StatusOK {
		t.Fatalf("tables.list after drop -> %d: %s", status, string(body))
	}
	if err := json.Unmarshal(body, &list); err != nil {
		t.Fatalf("decode table list after drop: %v", err)
	}
	for _, item := range list.Tables {
		if item.TableReference.TableID == viewID {
			t.Fatalf("view %q still in tables.list after drop: %s", viewID, string(body))
		}
	}
}

// TestDDLCreateViewThreeSegmentName exercises CREATE VIEW with separate
// backtick segments for project, dataset, and view.
func TestDDLCreateViewThreeSegmentName(t *testing.T) {
	runDDLViewQualifiedNameTest(t,
		"CREATE OR REPLACE VIEW `"+projectDDLView+"`.`"+datasetDDLView+"`.`"+viewDDLName+
			"` AS SELECT 1 AS x")
}

// TestDDLCreateViewDottedName exercises CREATE VIEW with a single
// backtick segment containing dots (`project.dataset.view`).
func TestDDLCreateViewDottedName(t *testing.T) {
	runDDLViewQualifiedNameTest(t,
		"CREATE OR REPLACE VIEW `"+projectDDLView+"."+datasetDDLView+"."+viewDDLName+
			"` AS SELECT 1 AS x")
}

const (
	projectDDLView = "proj-ddl-view-qual"
	datasetDDLView = "ds_view_qual"
	viewDDLName    = "vdot"
)

func runDDLViewQualifiedNameTest(t *testing.T, createSQL string) {
	t.Helper()
	env := startEmulatorWithFlags(t, emulatorFlags{
		dataDir: t.TempDir(),
	})
	const viewSQL = "SELECT 1 AS x"
	base := env.URL() + "/bigquery/v2/projects/" + projectDDLView

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectDDLView+
			`","datasetId":"`+datasetDDLView+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	createBody := `{"query":` + mustJSONQuote(createSQL) + `,"useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost, base+"/queries", []byte(createBody))
	if status != http.StatusOK {
		t.Fatalf("jobs.query (CREATE VIEW) -> %d: %s", status, string(body))
	}

	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetDDLView+"/tables", nil)
	if status != http.StatusOK {
		t.Fatalf("tables.list -> %d: %s", status, string(body))
	}
	var list struct {
		Tables []struct {
			TableReference bqtypes.TableReference  `json:"tableReference"`
			Type           string                  `json:"type"`
			View           *bqtypes.ViewDefinition `json:"view"`
		} `json:"tables"`
	}
	if err := json.Unmarshal(body, &list); err != nil {
		t.Fatalf("decode table list: %v", err)
	}
	found := false
	for _, item := range list.Tables {
		if item.TableReference.TableID == viewDDLName {
			found = true
			if item.Type != "VIEW" {
				t.Errorf("list type = %q, want VIEW", item.Type)
			}
			if item.View == nil || item.View.Query != viewSQL {
				t.Errorf("list view.query = %+v, want %q", item.View, viewSQL)
			}
		}
		if item.TableReference.TableID != viewDDLName &&
			item.TableReference.TableID == projectDDLView+"."+datasetDDLView+"."+viewDDLName {
			t.Errorf("list tableId = dotted qualified name %q, want %q",
				item.TableReference.TableID, viewDDLName)
		}
	}
	if !found {
		t.Fatalf("view %q missing from tables.list: %s", viewDDLName, string(body))
	}

	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetDDLView+"/tables/"+viewDDLName, nil)
	if status != http.StatusOK {
		t.Fatalf("tables.get -> %d: %s", status, string(body))
	}
	var got bqtypes.Table
	if err := json.Unmarshal(body, &got); err != nil {
		t.Fatalf("decode table: %v", err)
	}
	if got.Type != "VIEW" || got.View == nil || got.View.Query != viewSQL {
		t.Errorf("get = type %q view %+v, want VIEW query %q", got.Type, got.View, viewSQL)
	}

	selectBody := `{"query":"SELECT x FROM ` + datasetDDLView + `.` + viewDDLName +
		`","useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost, base+"/queries", []byte(selectBody))
	if status != http.StatusOK {
		t.Fatalf("SELECT from view -> %d: %s", status, string(body))
	}
}

func mustJSONQuote(s string) string {
	b, err := json.Marshal(s)
	if err != nil {
		panic(err)
	}
	return string(b)
}
