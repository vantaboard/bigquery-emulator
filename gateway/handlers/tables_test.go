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
	"google.golang.org/grpc"
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
            {"name":"id","type":"` + sqlTypeINT64 + `","mode":"` + sqlModeRequired + `"},
            {"name":"meta","type":"RECORD","mode":"` + sqlModeNullable + `","fields":[
                {"name":"k","type":"` + sqlTypeSTRING + `"}
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

// TestTableInsertMaterializedViewInfersSchema asserts a REST insert with
// materializedView.query but no schema dry-runs the query and registers
// the inferred output columns (QueryMaterializedViewIT posture).
func TestTableInsertMaterializedViewInfersSchema(t *testing.T) {
	fakeCat := &fakeCatalogClient{}
	fakeQuery := &fakeQueryClient{
		dryRunFn: func(_ context.Context, _ *enginepb.QueryRequest) (*enginepb.DryRunResponse, error) {
			return &enginepb.DryRunResponse{
				Schema: &enginepb.TableSchema{
					Fields: []*enginepb.FieldSchema{
						{Name: "TimestampField", Type: sqlTypeTIMESTAMP},
						{Name: "StringField", Type: sqlTypeSTRING},
						{Name: "BooleanField", Type: sqlTypeBOOL},
					},
				},
			}, nil
		},
	}
	body := `{
        "tableReference":{"tableId":"mv1"},
        "materializedView":{"query":"SELECT 1 AS x"}
    }`
	req := newTableReq(http.MethodPost, "", body)
	rec := httptest.NewRecorder()
	TableInsert(Dependencies{Catalog: fakeCat, Query: fakeQuery})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	if fakeQuery.lastDryRun == nil {
		t.Fatal("Query.DryRun was not called for materialized view insert")
	}
	if fakeCat.lastRegisterTable == nil {
		t.Fatal("Catalog.RegisterTable was not called")
	}
	schema := fakeCat.lastRegisterTable.GetSchema()
	if schema == nil || len(schema.Fields) != 3 {
		t.Fatalf("inferred schema not registered: %+v", schema)
	}
	var got bqtypes.Table
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if got.Type != "MATERIALIZED_VIEW" {
		t.Errorf("type = %q, want MATERIALIZED_VIEW", got.Type)
	}
	if got.MaterializedView == nil || got.MaterializedView.Query == "" {
		t.Errorf("materializedView not round-tripped: %+v", got.MaterializedView)
	}
}

// TestTableInsertLogicalViewRegistersInEngine asserts a REST insert
// with a view.query body registers the view through the engine's
// CREATE OR REPLACE VIEW path (so reads inline its definition) instead
// of registering an empty backing table that would shadow the view and
// make SELECT return zero rows. This is the regression Chris reported:
// "querying the view itself ... returned nothing".
func TestTableInsertLogicalViewRegistersInEngine(t *testing.T) {
	fakeCat := &fakeCatalogClient{}
	fakeQuery := &fakeQueryClient{
		dryRunFn: func(_ context.Context, _ *enginepb.QueryRequest) (*enginepb.DryRunResponse, error) {
			return &enginepb.DryRunResponse{
				Schema: &enginepb.TableSchema{
					Fields: []*enginepb.FieldSchema{
						{Name: "id", Type: sqlTypeINT64},
						{Name: "name", Type: sqlTypeSTRING},
					},
				},
			}, nil
		},
		executeQueryFn: func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return &fakeQueryResultStream{}, nil
		},
	}
	body := `{
        "tableReference":{"tableId":"v1"},
        "view":{"query":"SELECT id, name FROM ` + testDatasetID + `.people"}
    }`
	req := newTableReq(http.MethodPost, "", body)
	rec := httptest.NewRecorder()
	TableInsert(Dependencies{Catalog: fakeCat, Query: fakeQuery})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	// The defining query must reach the engine as a CREATE OR REPLACE
	// VIEW statement (the view-registry path), not a RegisterTable call.
	if fakeQuery.lastExecuteQuery == nil {
		t.Fatal("Query.ExecuteQuery was not called for logical view insert")
	}
	gotSQL := fakeQuery.lastExecuteQuery.GetSql()
	if !strings.Contains(strings.ToUpper(gotSQL), "CREATE OR REPLACE VIEW") {
		t.Errorf("ExecuteQuery SQL = %q, want a CREATE OR REPLACE VIEW", gotSQL)
	}
	if !strings.Contains(gotSQL, "SELECT id, name FROM "+testDatasetID+".people") {
		t.Errorf("ExecuteQuery SQL missing the view definition: %q", gotSQL)
	}
	if fakeCat.lastRegisterTable != nil {
		t.Fatal("Catalog.RegisterTable must not be called for a logical view " +
			"(an empty backing table would shadow the view on read)")
	}

	var got bqtypes.Table
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if got.Type != "VIEW" {
		t.Errorf("type = %q, want VIEW", got.Type)
	}
	if got.View == nil || got.View.Query == "" {
		t.Errorf("view not round-tripped: %+v", got.View)
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
	if tbl.Schema.Fields[0].Name != "id" || tbl.Schema.Fields[0].Type != sqlTypeINTEGER {
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

// TestTableListReturnsEmptyPage pins the empty-page shape (no Catalog
// client wired) the handler returns when the engine is missing.
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

// TestTableListSurfacesEngineEntries asserts the handler forwards
// the (project_id, dataset_id) pair, calls the engine, and folds the
// response into the REST envelope. Each entry must carry the
// required shape (kind, id, tableReference, labels) so the
// node-bigquery-tests sample's iteration works.
func TestTableListSurfacesEngineEntries(t *testing.T) {
	fake := &fakeCatalogClient{
		listTablesFn: func(_ context.Context, _ *enginepb.ListTablesRequest) (*enginepb.ListTablesResponse, error) {
			return &enginepb.ListTablesResponse{
				Tables: []*enginepb.TableRef{
					{ProjectId: testProjectID, DatasetId: testDatasetID, TableId: "alpha"},
					{ProjectId: testProjectID, DatasetId: testDatasetID, TableId: "bravo"},
				},
			}, nil
		},
	}
	req := newTableReq(http.MethodGet, "", "")
	rec := httptest.NewRecorder()
	TableList(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	if fake.lastListTables == nil {
		t.Fatal("Catalog.ListTables was not called")
	}
	ds := fake.lastListTables.GetDataset()
	if ds.GetProjectId() != testProjectID || ds.GetDatasetId() != testDatasetID {
		t.Errorf("dataset ref forwarded = %+v, want {proj, ds1}", ds)
	}
	var doc map[string]any
	if err := json.NewDecoder(rec.Body).Decode(&doc); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if doc["kind"] != "bigquery#tableList" {
		t.Errorf("kind = %v", doc["kind"])
	}
	items, _ := doc["tables"].([]any)
	if len(items) != 2 {
		t.Fatalf("tables entries = %d, want 2; body=%s", len(items), rec.Body.String())
	}
	first, _ := items[0].(map[string]any)
	if first["id"] != testProjectID+":"+testDatasetID+".alpha" {
		t.Errorf("first entry id = %v", first["id"])
	}
	ref, _ := first["tableReference"].(map[string]any)
	if ref["tableId"] != "alpha" {
		t.Errorf("first entry tableReference.tableId = %v", ref["tableId"])
	}
	if first["type"] != defaultTableType {
		t.Errorf("first entry type = %v, want %q", first["type"], defaultTableType)
	}
	if labels, ok := first["labels"].(map[string]any); !ok || len(labels) != 0 {
		t.Errorf("first entry labels = %v, want {}", first["labels"])
	}
}

// runTableMetadataRoundTrip drives the Insert -> Patch -> Get cycle a
// node `setMetadata` + `getMetadata` sample exercises, asserting the
// cached REST-only field comes back on the wire after the engine
// DescribeTable response (which carries only the schema) is merged
// with the in-memory MetadataStore overlay.
//
// `patch` is the JSON delta sent to the PATCH endpoint; `assertion`
// inspects the decoded Table on the GET response and reports any
// roundtrip failures via t.Errorf.
func runTableMetadataRoundTrip(t *testing.T, patch string, assertion func(*testing.T, bqtypes.Table)) {
	t.Helper()
	store := NewMetadataStore()
	fake := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, _ *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			return &enginepb.DescribeTableResponse{}, nil
		},
	}
	deps := Dependencies{Catalog: fake, Metadata: store}

	insert := newTableReq(http.MethodPost, "", `{"tableReference":{"tableId":"`+testTableID+`"}}`)
	rec := httptest.NewRecorder()
	TableInsert(deps)(rec, insert)
	if rec.Code != http.StatusOK {
		t.Fatalf("insert: status = %d, body=%s", rec.Code, rec.Body.String())
	}

	patchReq := newTableReq(http.MethodPatch, testTableID, patch)
	rec = httptest.NewRecorder()
	TablePatch(deps)(rec, patchReq)
	if rec.Code != http.StatusOK {
		t.Fatalf("patch: status = %d, body=%s", rec.Code, rec.Body.String())
	}

	get := newTableReq(http.MethodGet, testTableID, "")
	rec = httptest.NewRecorder()
	TableGet(deps)(rec, get)
	if rec.Code != http.StatusOK {
		t.Fatalf("get: status = %d, body=%s", rec.Code, rec.Body.String())
	}
	var got bqtypes.Table
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode get response: %v", err)
	}
	assertion(t, got)
}

// TestTableMetadataLabelsRoundTrip mirrors the upstream
// `labelTable` + `getTableLabels` sample sequence: setMetadata writes
// `{color:"green"}` then getMetadata reads it back. Without the
// MetadataStore overlay the get would lose the labels.
func TestTableMetadataLabelsRoundTrip(t *testing.T) {
	runTableMetadataRoundTrip(t, `{"labels":{"color":"green"}}`, func(t *testing.T, got bqtypes.Table) {
		if got.Labels["color"] != "green" {
			t.Errorf("labels = %v, want {color:\"green\"}", got.Labels)
		}
	})
}

// TestTableMetadataExpirationTimeRoundTrip mirrors the
// `updateTableExpiration` sample's setMetadata + getMetadata flow.
func TestTableMetadataExpirationTimeRoundTrip(t *testing.T) {
	runTableMetadataRoundTrip(t, `{"expirationTime":"1234567890"}`, func(t *testing.T, got bqtypes.Table) {
		if got.ExpirationTime.String() != "1234567890" {
			t.Errorf("expirationTime = %q, want %q", got.ExpirationTime, "1234567890")
		}
	})
}

// TestTableMetadataDefaultCollationRoundTrip pins the `und:ci` flow
// the `should create a table with collation` node test exercises.
func TestTableMetadataDefaultCollationRoundTrip(t *testing.T) {
	runTableMetadataRoundTrip(t, `{"defaultCollation":"und:ci"}`, func(t *testing.T, got bqtypes.Table) {
		if got.DefaultCollation != "und:ci" {
			t.Errorf("defaultCollation = %q, want %q", got.DefaultCollation, "und:ci")
		}
	})
}

// TestTableMetadataRangePartitioningRoundTrip pins the integer-range
// partitioning spec the `createTableRangePartitioned` sample writes.
func TestTableMetadataRangePartitioningRoundTrip(t *testing.T) {
	patch := `{"rangePartitioning":{"field":"x","range":{"start":"0","end":"100","interval":"10"}}}`
	runTableMetadataRoundTrip(t, patch, func(t *testing.T, got bqtypes.Table) {
		if got.RangePartitioning == nil {
			t.Fatal("rangePartitioning missing")
		}
		if got.RangePartitioning.Field != "x" {
			t.Errorf("rangePartitioning.field = %q, want %q", got.RangePartitioning.Field, "x")
		}
		if got.RangePartitioning.Range == nil ||
			got.RangePartitioning.Range.Start != "0" ||
			got.RangePartitioning.Range.End != "100" ||
			got.RangePartitioning.Range.Interval != "10" {
			t.Errorf("rangePartitioning.range = %+v", got.RangePartitioning.Range)
		}
	})
}

// TestTableMetadataClusteringRoundTrip pins the clustering spec the
// `createTableClustered` sample writes.
func TestTableMetadataClusteringRoundTrip(t *testing.T) {
	runTableMetadataRoundTrip(t, `{"clustering":{"fields":["city","zipcode"]}}`, func(t *testing.T, got bqtypes.Table) {
		if got.Clustering == nil {
			t.Fatal("clustering missing")
		}
		if len(got.Clustering.Fields) != 2 ||
			got.Clustering.Fields[0] != "city" ||
			got.Clustering.Fields[1] != "zipcode" {
			t.Errorf("clustering.fields = %v, want [city zipcode]", got.Clustering.Fields)
		}
	})
}

// TestTableDeleteEvictsMetadata asserts that DELETE clears the
// MetadataStore so a subsequent Insert against the same ID does not
// surface stale labels.
func TestTableDeleteEvictsMetadata(t *testing.T) {
	store := NewMetadataStore()
	fake := &fakeCatalogClient{}
	deps := Dependencies{Catalog: fake, Metadata: store}

	insert := newTableReq(http.MethodPost, "",
		`{"tableReference":{"tableId":"`+testTableID+`"},"labels":{"k":"v"}}`)
	rec := httptest.NewRecorder()
	TableInsert(deps)(rec, insert)
	if rec.Code != http.StatusOK {
		t.Fatalf("insert: status=%d body=%s", rec.Code, rec.Body.String())
	}
	if entry, ok := store.GetTable(testProjectID, testDatasetID, testTableID); !ok || entry.Labels["k"] != "v" {
		t.Fatalf("metadata not stored after insert: %+v ok=%v", entry, ok)
	}

	del := newTableReq(http.MethodDelete, testTableID, "")
	rec = httptest.NewRecorder()
	TableDelete(deps)(rec, del)
	if rec.Code != http.StatusOK {
		t.Fatalf("delete: status=%d body=%s", rec.Code, rec.Body.String())
	}
	if _, ok := store.GetTable(testProjectID, testDatasetID, testTableID); ok {
		t.Errorf("MetadataStore entry survived TableDelete")
	}
}

func TestTableGetIamPolicyReturnsEmptyStubPolicy(t *testing.T) {
	rec := httptest.NewRecorder()
	req := newTableReq(http.MethodPost, testTableID+":getIamPolicy", "{}")
	TableCustomMethodPOST(Dependencies{})(rec, req)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	var got map[string]any
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if etag, _ := got["etag"].(string); etag == "" {
		t.Errorf("etag = %q, want non-empty", etag)
	}
	bindings, _ := got["bindings"].([]any)
	if len(bindings) != 0 {
		t.Errorf("bindings = %v, want []", bindings)
	}
}

func TestTableSetIamPolicyReturnsPolicyWithEtag(t *testing.T) {
	rec := httptest.NewRecorder()
	body := `{"policy":{"version":1,"bindings":[{"role":"roles/viewer","members":["user:alice@example.com"]}]}}`
	req := newTableReq(http.MethodPost, testTableID+":setIamPolicy", body)
	TableCustomMethodPOST(Dependencies{})(rec, req)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	var got map[string]any
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if etag, _ := got["etag"].(string); etag == "" {
		t.Errorf("etag = %q, want non-empty", etag)
	}
	bindings, _ := got["bindings"].([]any)
	if len(bindings) != 1 {
		t.Fatalf("bindings = %v, want one entry", bindings)
	}
}
