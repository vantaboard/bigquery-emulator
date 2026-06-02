package handlers

import (
	"context"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

// newTableReq builds an *http.Request mirroring how Go's mux populates
// the projectId / datasetId / tableId wildcards at runtime, so handler
// unit tests don't have to wire the full server. The project ID and
// dataset ID are the package-level testProjectID / testDatasetID
// constants; every call site here uses those.
func newTableReq(method, tableID, body string) *http.Request {
	url := "/bigquery/v2/projects/" + testProjectID + "/datasets/" + testDatasetID + "/tables"
	if tableID != "" {
		url += "/" + tableID
	}
	req := httptest.NewRequest(method, url, strings.NewReader(body))
	req.SetPathValue("projectId", testProjectID)
	req.SetPathValue("datasetId", testDatasetID)
	if tableID != "" {
		req.SetPathValue("tableId", tableID)
	}
	if body != "" {
		req.Header.Set("Content-Type", "application/json")
	}
	return req
}

// TestTableInsertForwardsSchema asserts the REST body's nested schema
// is converted to the gRPC FieldSchema graph and forwarded verbatim,
// and that the response carries the stamped Table resource fields.
func TestTableInsertForwardsSchema(t *testing.T) {
	fake := &fakeCatalogClient{}
	body := `{
        "tableReference":{"tableId":"t1"},
        "schema":{"fields":[
            {"name":"id","type":"INT64","mode":"REQUIRED"},
            {"name":"meta","type":"RECORD","mode":"NULLABLE","fields":[
                {"name":"k","type":"STRING"}
            ]}
        ]}
    }`
	req := newTableReq(http.MethodPost, "", body)
	rec := httptest.NewRecorder()
	TableInsert(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	if fake.lastRegisterTable == nil {
		t.Fatal("Catalog.RegisterTable was not called")
	}
	if got := fake.lastRegisterTable.GetTable().GetTableId(); got != testTableID {
		t.Errorf("table_id forwarded = %q, want %q", got, testTableID)
	}
	schema := fake.lastRegisterTable.GetSchema()
	if schema == nil || len(schema.Fields) != 2 {
		t.Fatalf("schema not forwarded correctly: %+v", schema)
	}
	if schema.Fields[0].Name != "id" || schema.Fields[0].Type != sqlTypeINT64 ||
		schema.Fields[0].Mode != sqlModeRequired {
		t.Errorf("top-level field mismatch: %+v", schema.Fields[0])
	}
	if len(schema.Fields[1].Fields) != 1 || schema.Fields[1].Fields[0].Name != "k" {
		t.Errorf("nested STRUCT field not forwarded: %+v", schema.Fields[1])
	}

	var got bqtypes.Table
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if got.Kind != "bigquery#table" {
		t.Errorf("kind = %q, want %q", got.Kind, "bigquery#table")
	}
	if got.ID != "proj:ds1.t1" {
		t.Errorf("id = %q, want %q", got.ID, "proj:ds1.t1")
	}
	if got.Type != "TABLE" {
		t.Errorf("type = %q, want %q", got.Type, "TABLE")
	}
	if got.Schema == nil || len(got.Schema.Fields) != 2 {
		t.Errorf("response schema mismatch: %+v", got.Schema)
	}
}

// TestTableInsertRequiresTableID rejects a missing tableId before any
// RPC is issued, mirroring the upstream documented requirement.
func TestTableInsertRequiresTableID(t *testing.T) {
	fake := &fakeCatalogClient{}
	req := newTableReq(http.MethodPost, "", `{}`)
	rec := httptest.NewRecorder()
	TableInsert(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusBadRequest {
		t.Fatalf("status = %d, want 400", rec.Code)
	}
	if fake.lastRegisterTable != nil {
		t.Fatal("Catalog.RegisterTable must not be called when tableId is missing")
	}
}

// TestTableGetUsesDescribeTable verifies the GET handler resolves a
// table through Catalog.DescribeTable and round-trips the schema.
func TestTableGetUsesDescribeTable(t *testing.T) {
	fake := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, _ *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			return &enginepb.DescribeTableResponse{
				Schema: &enginepb.TableSchema{
					Fields: []*enginepb.FieldSchema{{
						Name: "id", Type: sqlTypeINT64, Mode: sqlModeRequired,
					}},
				},
			}, nil
		},
	}
	req := newTableReq(http.MethodGet, testTableID, "")
	rec := httptest.NewRecorder()
	TableGet(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200", rec.Code)
	}
	if fake.lastDescribeTable.GetTable().GetTableId() != testTableID {
		t.Errorf("DescribeTable not called for the requested table_id; got %q",
			fake.lastDescribeTable.GetTable().GetTableId())
	}

	var tbl bqtypes.Table
	if err := json.NewDecoder(rec.Body).Decode(&tbl); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if tbl.Schema == nil || len(tbl.Schema.Fields) != 1 {
		t.Fatalf("schema not round-tripped: %+v", tbl.Schema)
	}
	if tbl.Schema.Fields[0].Name != "id" || tbl.Schema.Fields[0].Type != sqlTypeINT64 {
		t.Errorf("schema field round-trip mismatch: %+v", tbl.Schema.Fields[0])
	}
}

// TestTableGetNotFound asserts a missing table surfaces as 404 with
// the BigQuery envelope, not 500.
func TestTableGetNotFound(t *testing.T) {
	fake := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, _ *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			return nil, status.Error(codes.NotFound, "table not found")
		},
	}
	req := newTableReq(http.MethodGet, "missing", "")
	rec := httptest.NewRecorder()
	TableGet(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404", rec.Code)
	}
}

// TestTableDeleteForwardsTableRef pins the (project,dataset,table)
// triple forwarded to the engine on a delete.
func TestTableDeleteForwardsTableRef(t *testing.T) {
	fake := &fakeCatalogClient{}
	req := newTableReq(http.MethodDelete, testTableID, "")
	rec := httptest.NewRecorder()
	TableDelete(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	if fake.lastDropTable == nil {
		t.Fatal("Catalog.DropTable was not called")
	}
	tr := fake.lastDropTable.GetTable()
	if tr.GetProjectId() != testProjectID || tr.GetDatasetId() != testDatasetID || tr.GetTableId() != testTableID {
		t.Errorf("TableRef forwarded = %+v, want {proj, ds1, t1}", tr)
	}
}

// TestTableGetLabelsIsEmptyObjectNotNull pins the live-IT contract:
// node-bigquery-tests `getTableLabels` calls
// `Object.entries(table.metadata.labels)` on the deserialized response,
// which raises `TypeError: Cannot convert undefined or null to object`
// when the field is absent or null. Live BigQuery returns
// `labels: {}` for a newly-created table; the emulator must do the
// same.
func TestTableGetLabelsIsEmptyObjectNotNull(t *testing.T) {
	fake := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, _ *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			return &enginepb.DescribeTableResponse{}, nil
		},
	}
	req := newTableReq(http.MethodGet, testTableID, "")
	rec := httptest.NewRecorder()
	TableGet(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200", rec.Code)
	}
	var doc map[string]any
	if err := json.NewDecoder(rec.Body).Decode(&doc); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	got, present := doc["labels"]
	if !present {
		t.Fatalf("response missing `labels` field; body=%s", rec.Body.String())
	}
	if got == nil {
		t.Fatalf("`labels` is null; live BigQuery returns {}")
	}
	obj, ok := got.(map[string]any)
	if !ok {
		t.Fatalf("`labels` is %T, want map[string]any", got)
	}
	if len(obj) != 0 {
		t.Errorf("`labels` = %v, want {}", obj)
	}
}

// TestTableListReturnsEmptyPage pins the empty-page shape.
func TestTableListReturnsEmptyPage(t *testing.T) {
	req := newTableReq(http.MethodGet, "", "")
	rec := httptest.NewRecorder()
	TableList(Dependencies{})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200", rec.Code)
	}
	var doc map[string]any
	if err := json.NewDecoder(rec.Body).Decode(&doc); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if doc["kind"] != "bigquery#tableList" {
		t.Errorf("kind = %v, want %q", doc["kind"], "bigquery#tableList")
	}
}
