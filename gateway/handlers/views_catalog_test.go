package handlers

import (
	"context"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

const (
	testViewName = "my_view"
	testViewSQL  = "SELECT 1 AS x"
)

func ddlViewQueryStream(statementType string) *fakeQueryResultStream {
	return &fakeQueryResultStream{
		msgs: []*enginepb.QueryResultRow{
			{StatementType: statementType},
		},
	}
}

func viewNotFoundCatalog(viewID string) *fakeCatalogClient {
	return &fakeCatalogClient{
		describeTableFn: func(_ context.Context, _ *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			return nil, status.Error(codes.NotFound, "table not found")
		},
		listTablesFn: func(_ context.Context, _ *enginepb.ListTablesRequest) (*enginepb.ListTablesResponse, error) {
			return &enginepb.ListTablesResponse{
				Tables: []*enginepb.TableRef{
					{ProjectId: testProjectID, DatasetId: testDatasetID, TableId: viewID},
				},
			}, nil
		},
	}
}

func viewDDLQueryDeps(
	viewID string,
	executeFn func(context.Context, *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error),
) Dependencies {
	return Dependencies{
		Catalog: viewNotFoundCatalog(viewID),
		Query: &fakeQueryClient{
			executeQueryFn: executeFn,
		},
		Metadata: NewMetadataStore(),
	}
}

func runViewDDLQuery(t *testing.T, deps Dependencies, sql string) {
	t.Helper()
	body := `{"query":` + jsonQuote(sql) + `,"useLegacySql":false,"defaultDataset":{"datasetId":"` +
		testDatasetID + `"}}`
	rec := runQueryWithDeps(t, testProjectID, deps, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("query: status=%d body=%s", rec.Code, rec.Body.String())
	}
}

func getViewTable(t *testing.T, deps Dependencies, viewID string) bqtypes.Table {
	t.Helper()
	get := newTableReq(http.MethodGet, viewID, "")
	rec := httptest.NewRecorder()
	TableGet(deps)(rec, get)
	if rec.Code != http.StatusOK {
		t.Fatalf("get: status=%d body=%s", rec.Code, rec.Body.String())
	}
	var got bqtypes.Table
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	return got
}

// TestQueryRunCreateViewPersistsMetadata asserts CREATE VIEW via
// jobs.query stashes view metadata so tables.get / tables.list work.
func TestQueryRunCreateViewPersistsMetadata(t *testing.T) {
	createSQL := "CREATE OR REPLACE VIEW `" + testProjectID + "." + testDatasetID + "." + testViewName +
		"` AS " + testViewSQL
	deps := viewDDLQueryDeps(
		testViewName,
		func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return ddlViewQueryStream("CREATE_VIEW"), nil
		},
	)
	runViewDDLQuery(t, deps, createSQL)

	got := getViewTable(t, deps, testViewName)
	if got.Type != viewTableType {
		t.Errorf("type = %q, want %q", got.Type, viewTableType)
	}
	if got.View == nil || got.View.Query != testViewSQL {
		t.Errorf("view = %+v, want query %q", got.View, testViewSQL)
	}

	listReq := httptest.NewRequest(http.MethodGet,
		"/bigquery/v2/projects/"+testProjectID+"/datasets/"+testDatasetID+"/tables", nil)
	listReq.SetPathValue("projectId", testProjectID)
	listReq.SetPathValue("datasetId", testDatasetID)
	rec := httptest.NewRecorder()
	TableList(deps)(rec, listReq)
	if rec.Code != http.StatusOK {
		t.Fatalf("list: status=%d body=%s", rec.Code, rec.Body.String())
	}
	var list map[string]any
	if err := json.NewDecoder(rec.Body).Decode(&list); err != nil {
		t.Fatalf("decode list: %v", err)
	}
	tables, _ := list["tables"].([]any)
	if len(tables) != 1 {
		t.Fatalf("tables = %v, want 1 entry", list["tables"])
	}
	entry, _ := tables[0].(map[string]any)
	if entry["type"] != viewTableType {
		t.Errorf("list type = %v, want %q", entry["type"], viewTableType)
	}
}

// TestQueryRunCreateViewReplaceUpdatesQuery asserts CREATE OR REPLACE
// VIEW updates the stashed view.query.
func TestQueryRunCreateViewReplaceUpdatesQuery(t *testing.T) {
	viewName := "replace_view"
	firstSQL := "CREATE VIEW `" + testProjectID + "." + testDatasetID + "." + viewName +
		"` AS SELECT 1 AS a"
	secondSQL := "CREATE OR REPLACE VIEW `" + testProjectID + "." + testDatasetID + "." + viewName +
		"` AS SELECT 2 AS b"
	deps := viewDDLQueryDeps(
		viewName,
		func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return ddlViewQueryStream("CREATE_VIEW"), nil
		},
	)
	runViewDDLQuery(t, deps, firstSQL)
	runViewDDLQuery(t, deps, secondSQL)

	got := getViewTable(t, deps, viewName)
	if got.View == nil || got.View.Query != "SELECT 2 AS b" {
		t.Errorf("view.query = %+v, want SELECT 2 AS b", got.View)
	}
}

// TestQueryRunDropViewEvictsMetadata asserts DROP VIEW removes the
// stashed overlay (engine surfaces statementType DROP_TABLE).
func TestQueryRunDropViewEvictsMetadata(t *testing.T) {
	createSQL := "CREATE VIEW `" + testProjectID + "." + testDatasetID + "." + testViewName +
		"` AS " + testViewSQL
	dropSQL := "DROP VIEW `" + testProjectID + "." + testDatasetID + "." + testViewName + "`"
	var call int
	deps := viewDDLQueryDeps(
		testViewName,
		func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			call++
			stmt := "CREATE_VIEW"
			if call > 1 {
				stmt = "DROP_TABLE"
			}
			return ddlViewQueryStream(stmt), nil
		},
	)
	runViewDDLQuery(t, deps, createSQL)
	runViewDDLQuery(t, deps, dropSQL)

	get := newTableReq(http.MethodGet, testViewName, "")
	rec := httptest.NewRecorder()
	TableGet(deps)(rec, get)
	if rec.Code != http.StatusNotFound {
		t.Errorf("get after drop: status=%d, want 404; body=%s", rec.Code, rec.Body.String())
	}
}

func jsonQuote(s string) string {
	b, err := json.Marshal(s)
	if err != nil {
		panic(err)
	}
	return string(b)
}
