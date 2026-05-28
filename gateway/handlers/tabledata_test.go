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

// newTabledataReq builds the path + path values net/http would populate
// at runtime for the tabledata routes, so the handler can be exercised
// in isolation from the mux.
func newTabledataReq(method, projectID, datasetID, tableID, suffix, body string) *http.Request {
	url := "/bigquery/v2/projects/" + projectID + "/datasets/" + datasetID +
		"/tables/" + tableID + "/" + suffix
	req := httptest.NewRequest(method, url, strings.NewReader(body))
	req.SetPathValue("projectId", projectID)
	req.SetPathValue("datasetId", datasetID)
	req.SetPathValue("tableId", tableID)
	if body != "" {
		req.Header.Set("Content-Type", "application/json")
	}
	return req
}

// peopleSchema returns the proto schema used by the insertAll/list
// happy-path tests: (id INT64, name STRING, tags REPEATED STRING).
func peopleSchema() *enginepb.TableSchema {
	return &enginepb.TableSchema{
		Fields: []*enginepb.FieldSchema{
			{Name: "id", Type: sqlTypeINT64, Mode: sqlModeRequired},
			{Name: testColumnName, Type: sqlTypeSTRING, Mode: sqlModeNullable},
			{Name: "tags", Type: sqlTypeSTRING, Mode: "REPEATED"},
		},
	}
}

// TestTableDataInsertAllRoundTrip walks the JSON body all the way to
// the engine: the table schema fetched via DescribeTable drives the
// column order on the forwarded proto, missing/unknown fields are
// handled the way insertAll documents, and the response carries the
// documented `kind`.
func TestTableDataInsertAllRoundTrip(t *testing.T) {
	fake := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, _ *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			return &enginepb.DescribeTableResponse{Schema: peopleSchema()}, nil
		},
	}
	body := `{
        "rows":[
            {"insertId":"a","json":{"id":1,"name":"alice","tags":["x","y"]}},
            {"insertId":"b","json":{"id":2,"name":null,"tags":[]}}
        ]
    }`
	req := newTabledataReq(http.MethodPost, testProjectID, testDatasetID, testTableID, "insertAll", body)
	rec := httptest.NewRecorder()
	TableDataInsertAll(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	if fake.lastInsertRows == nil {
		t.Fatal("Catalog.InsertRows was not called")
	}
	if got := fake.lastInsertRows.GetTable().GetTableId(); got != testTableID {
		t.Errorf("table_id forwarded = %q, want %q", got, testTableID)
	}
	rows := fake.lastInsertRows.GetRows()
	if len(rows) != 2 {
		t.Fatalf("rows forwarded = %d, want 2", len(rows))
	}
	if cells := rows[0].GetCells(); len(cells) != 3 {
		t.Fatalf("row[0] cells = %d, want 3", len(cells))
	} else {
		if got := cells[0].GetStringValue(); got != "1" {
			t.Errorf("row[0].id = %q, want %q", got, "1")
		}
		if got := cells[1].GetStringValue(); got != testUserAlice {
			t.Errorf("row[0].name = %q, want %q", got, testUserAlice)
		}
		if cells[2].GetArray() == nil {
			t.Errorf("row[0].tags should be Array; got %+v", cells[2])
		} else if got := len(cells[2].GetArray().GetElements()); got != 2 {
			t.Errorf("row[0].tags len = %d, want 2", got)
		}
	}
	cells := rows[1].GetCells()
	if !cells[1].GetNullValue() {
		t.Errorf("row[1].name should be null, got %+v", cells[1])
	}

	var resp bqtypes.TableDataInsertAllResponse
	if err := json.NewDecoder(rec.Body).Decode(&resp); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if resp.Kind != tableDataInsertAllKind {
		t.Errorf("kind = %q, want %q", resp.Kind, tableDataInsertAllKind)
	}
}

// TestTableDataInsertAllMissingFieldsBecomeNull verifies the handler
// pads missing schema columns with NULL cells so the engine's shape
// check (cell count == column count) passes even on sparse rows.
func TestTableDataInsertAllMissingFieldsBecomeNull(t *testing.T) {
	fake := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, _ *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			return &enginepb.DescribeTableResponse{Schema: peopleSchema()}, nil
		},
	}
	body := `{"rows":[{"json":{"id":7}}]}`
	req := newTabledataReq(http.MethodPost, testProjectID, testDatasetID, testTableID, "insertAll", body)
	rec := httptest.NewRecorder()
	TableDataInsertAll(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	if fake.lastInsertRows == nil {
		t.Fatal("Catalog.InsertRows was not called")
	}
	cells := fake.lastInsertRows.GetRows()[0].GetCells()
	if len(cells) != 3 {
		t.Fatalf("cells = %d, want 3", len(cells))
	}
	if cells[0].GetStringValue() != "7" {
		t.Errorf("id cell = %v, want \"7\"", cells[0])
	}
	if !cells[1].GetNullValue() {
		t.Errorf("name cell should be null, got %+v", cells[1])
	}
	if !cells[2].GetNullValue() {
		t.Errorf("tags cell should be null, got %+v", cells[2])
	}
}

// TestTableDataInsertAllEmptyRowsSkipsRPC short-circuits an empty
// insertAll body to a success envelope without ever calling InsertRows.
func TestTableDataInsertAllEmptyRowsSkipsRPC(t *testing.T) {
	fake := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, _ *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			return &enginepb.DescribeTableResponse{Schema: peopleSchema()}, nil
		},
	}
	req := newTabledataReq(http.MethodPost, testProjectID, testDatasetID, testTableID, "insertAll", `{"rows":[]}`)
	rec := httptest.NewRecorder()
	TableDataInsertAll(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	if fake.lastInsertRows != nil {
		t.Error("Catalog.InsertRows should not be called for an empty rows[] list")
	}
}

// TestTableDataInsertAllPropagatesNotFound asserts the gateway
// translates a NOT_FOUND describe failure into 404 + the BigQuery
// error envelope without ever calling InsertRows.
func TestTableDataInsertAllPropagatesNotFound(t *testing.T) {
	fake := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, _ *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			return nil, status.Error(codes.NotFound, "no such table")
		},
	}
	body := `{"rows":[{"json":{"id":1}}]}`
	req := newTabledataReq(http.MethodPost, testProjectID, testDatasetID, "ghost", "insertAll", body)
	rec := httptest.NewRecorder()
	TableDataInsertAll(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404; body=%s", rec.Code, rec.Body.String())
	}
	if fake.lastInsertRows != nil {
		t.Error("InsertRows must not be issued after a DescribeTable error")
	}
}

// TestTableDataListPaginationParams asserts the handler forwards the
// documented startIndex / maxResults / pageToken triple into the
// ListRowsRequest and exposes totalRows + pageToken in the response.
func TestTableDataListPaginationParams(t *testing.T) {
	fake := &fakeCatalogClient{
		listRowsFn: func(_ context.Context, in *enginepb.ListRowsRequest) (*enginepb.ListRowsResponse, error) {
			if in.GetStartIndex() != 5 || in.GetMaxResults() != 2 {
				t.Errorf("ListRows forwarded start=%d max=%d, want start=5 max=2",
					in.GetStartIndex(), in.GetMaxResults())
			}
			return &enginepb.ListRowsResponse{
				Rows: []*enginepb.DataRow{
					{Cells: []*enginepb.Cell{
						{Value: &enginepb.Cell_StringValue{StringValue: "5"}},
						{Value: &enginepb.Cell_StringValue{StringValue: "five"}},
					}},
					{Cells: []*enginepb.Cell{
						{Value: &enginepb.Cell_StringValue{StringValue: "6"}},
						{Value: &enginepb.Cell_NullValue{NullValue: true}},
					}},
				},
				TotalRows:      10,
				NextStartIndex: 7,
			}, nil
		},
	}
	req := newTabledataReq(
		http.MethodGet,
		testProjectID,
		testDatasetID,
		testTableID,
		"data?startIndex=5&maxResults=2",
		"",
	)
	rec := httptest.NewRecorder()
	TableDataList(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	var out bqtypes.TableDataList
	if err := json.NewDecoder(rec.Body).Decode(&out); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if out.Kind != tableDataListKind {
		t.Errorf("kind = %q, want %q", out.Kind, tableDataListKind)
	}
	if out.TotalRows != "10" {
		t.Errorf("totalRows = %q, want %q", out.TotalRows, "10")
	}
	if out.PageToken != "7" {
		t.Errorf("pageToken = %q, want %q", out.PageToken, "7")
	}
	if len(out.Rows) != 2 {
		t.Fatalf("rows = %d, want 2", len(out.Rows))
	}
	if got := out.Rows[0].F[1].V; got != "five" {
		t.Errorf("row[0].name = %v, want %q", got, "five")
	}
	if out.Rows[1].F[1].V != nil {
		t.Errorf("row[1].name = %v, want nil", out.Rows[1].F[1].V)
	}
}

// TestTableDataListPageTokenOverridesStartIndex verifies that the
// pageToken query parameter wins over startIndex (matching how the
// official BigQuery client libraries page through results).
func TestTableDataListPageTokenOverridesStartIndex(t *testing.T) {
	fake := &fakeCatalogClient{
		listRowsFn: func(_ context.Context, in *enginepb.ListRowsRequest) (*enginepb.ListRowsResponse, error) {
			if in.GetStartIndex() != 9 {
				t.Errorf("ListRows.start_index = %d, want 9 (pageToken)",
					in.GetStartIndex())
			}
			return &enginepb.ListRowsResponse{TotalRows: 9, NextStartIndex: 9}, nil
		},
	}
	req := newTabledataReq(
		http.MethodGet,
		testProjectID,
		testDatasetID,
		testTableID,
		"data?startIndex=2&pageToken=9",
		"",
	)
	rec := httptest.NewRecorder()
	TableDataList(Dependencies{Catalog: fake})(rec, req)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}

	var out bqtypes.TableDataList
	if err := json.NewDecoder(rec.Body).Decode(&out); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if out.PageToken != "" {
		t.Errorf("pageToken should be empty when next_start_index >= total_rows, got %q",
			out.PageToken)
	}
}

// TestTableDataListMalformedParam returns a 400 envelope without an
// RPC when startIndex / maxResults fail to parse.
func TestTableDataListMalformedParam(t *testing.T) {
	fake := &fakeCatalogClient{}
	req := newTabledataReq(http.MethodGet, testProjectID, testDatasetID, testTableID, "data?startIndex=oops", "")
	rec := httptest.NewRecorder()
	TableDataList(Dependencies{Catalog: fake})(rec, req)
	if rec.Code != http.StatusBadRequest {
		t.Fatalf("status = %d, want 400", rec.Code)
	}
	if fake.lastListRows != nil {
		t.Error("ListRows must not be called for a malformed query parameter")
	}
}
