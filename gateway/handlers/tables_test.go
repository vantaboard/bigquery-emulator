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
// unit tests don't have to wire the full server.
func newTableReq(method, projectID, datasetID, tableID, body string) *http.Request {
	url := "/bigquery/v2/projects/" + projectID + "/datasets/" + datasetID + "/tables"
	if tableID != "" {
		url += "/" + tableID
	}
	req := httptest.NewRequest(method, url, strings.NewReader(body))
	req.SetPathValue("projectId", projectID)
	req.SetPathValue("datasetId", datasetID)
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
	req := newTableReq(http.MethodPost, "proj", "ds1", "", body)
	rec := httptest.NewRecorder()
	TableInsert(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	if fake.lastRegisterTable == nil {
		t.Fatal("Catalog.RegisterTable was not called")
	}
	if got := fake.lastRegisterTable.GetTable().GetTableId(); got != "t1" {
		t.Errorf("table_id forwarded = %q, want %q", got, "t1")
	}
	schema := fake.lastRegisterTable.GetSchema()
	if schema == nil || len(schema.Fields) != 2 {
		t.Fatalf("schema not forwarded correctly: %+v", schema)
	}
	if schema.Fields[0].Name != "id" || schema.Fields[0].Type != "INT64" || schema.Fields[0].Mode != "REQUIRED" {
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
	req := newTableReq(http.MethodPost, "proj", "ds1", "", `{}`)
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
						Name: "id", Type: "INT64", Mode: "REQUIRED",
					}},
				},
			}, nil
		},
	}
	req := newTableReq(http.MethodGet, "proj", "ds1", "t1", "")
	rec := httptest.NewRecorder()
	TableGet(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200", rec.Code)
	}
	if fake.lastDescribeTable.GetTable().GetTableId() != "t1" {
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
	if tbl.Schema.Fields[0].Name != "id" || tbl.Schema.Fields[0].Type != "INT64" {
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
	req := newTableReq(http.MethodGet, "proj", "ds1", "missing", "")
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
	req := newTableReq(http.MethodDelete, "proj", "ds1", "t1", "")
	rec := httptest.NewRecorder()
	TableDelete(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	if fake.lastDropTable == nil {
		t.Fatal("Catalog.DropTable was not called")
	}
	tr := fake.lastDropTable.GetTable()
	if tr.GetProjectId() != "proj" || tr.GetDatasetId() != "ds1" || tr.GetTableId() != "t1" {
		t.Errorf("TableRef forwarded = %+v, want {proj, ds1, t1}", tr)
	}
}

// TestTableListReturnsEmptyPage pins the empty-page shape.
func TestTableListReturnsEmptyPage(t *testing.T) {
	req := newTableReq(http.MethodGet, "proj", "ds1", "", "")
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
