package handlers

import (
	"context"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

// schemaPatchFakeCatalog tracks a single table schema across
// DescribeTable / DropTable / RegisterTable for PATCH schema tests.
type schemaPatchFakeCatalog struct {
	fakeCatalogClient
	schema *enginepb.TableSchema
}

func newSchemaPatchFakeCatalog(schema *enginepb.TableSchema) *schemaPatchFakeCatalog {
	c := &schemaPatchFakeCatalog{schema: schema}
	c.describeTableFn = func(_ context.Context, _ *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
		return &enginepb.DescribeTableResponse{Schema: c.schema}, nil
	}
	c.listRowsFn = func(_ context.Context, _ *enginepb.ListRowsRequest) (*enginepb.ListRowsResponse, error) {
		return &enginepb.ListRowsResponse{TotalRows: 0}, nil
	}
	c.dropTableFn = func(_ context.Context, _ *enginepb.DropTableRequest) (*enginepb.DropTableResponse, error) {
		return &enginepb.DropTableResponse{}, nil
	}
	c.registerTableFn = func(_ context.Context, in *enginepb.RegisterTableRequest) (*enginepb.RegisterTableResponse, error) {
		c.schema = in.GetSchema()
		c.lastRegisterTable = in
		return &enginepb.RegisterTableResponse{}, nil
	}
	return c
}

func runTableSchemaPatch(t *testing.T, cat *schemaPatchFakeCatalog, patch string) (bqtypes.Table, Dependencies) {
	t.Helper()
	store := NewMetadataStore()
	deps := Dependencies{Catalog: cat, Metadata: store}

	insert := newTableReq(http.MethodPost, "", `{
		"tableReference":{"tableId":"`+testTableID+`"},
		"schema":{"fields":[{"name":"id","type":"`+sqlTypeINT64+`","mode":"`+sqlModeRequired+`"}]}
	}`)
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
	var patched bqtypes.Table
	if err := json.NewDecoder(rec.Body).Decode(&patched); err != nil {
		t.Fatalf("decode patch response: %v", err)
	}
	return patched, deps
}

func tableGetSchema(t *testing.T, deps Dependencies) bqtypes.Table {
	t.Helper()
	get := newTableReq(http.MethodGet, testTableID, "")
	rec := httptest.NewRecorder()
	TableGet(deps)(rec, get)
	if rec.Code != http.StatusOK {
		t.Fatalf("get: status = %d, body=%s", rec.Code, rec.Body.String())
	}
	var got bqtypes.Table
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode get response: %v", err)
	}
	return got
}

func TestTablePatchSchemaRelaxRequiredToNullable(t *testing.T) {
	t.Parallel()
	cat := newSchemaPatchFakeCatalog(&enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: "id", Type: sqlTypeINT64, Mode: sqlModeRequired},
	}})
	patch := `{"schema":{"fields":[{"name":"id","type":"` + sqlTypeINT64 + `","mode":"NULLABLE"}]}}`
	got, deps := runTableSchemaPatch(t, cat, patch)
	if got.Schema == nil || got.Schema.Fields[0].Mode != "" {
		t.Fatalf("patch response schema = %#v, want NULLABLE id", got.Schema)
	}
	afterGet := tableGetSchema(t, deps)
	if afterGet.Schema == nil || afterGet.Schema.Fields[0].Mode != "" {
		t.Fatalf("get schema = %#v, want NULLABLE id", afterGet.Schema)
	}
}

func TestTablePatchSchemaDescriptionRoundTrip(t *testing.T) {
	t.Parallel()
	cat := newSchemaPatchFakeCatalog(&enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: "id", Type: sqlTypeINT64, Mode: sqlModeRequired},
	}})
	patch := `{"schema":{"fields":[{"name":"id","type":"` + sqlTypeINT64 +
		`","mode":"REQUIRED","description":"primary key"}]}}`
	got, _ := runTableSchemaPatch(t, cat, patch)
	if got.Schema == nil || got.Schema.Fields[0].Description != "primary key" {
		t.Fatalf("patch response schema = %#v", got.Schema)
	}
	if cat.schema.Fields[0].GetDescription() != "primary key" {
		t.Fatalf("engine schema description = %q", cat.schema.Fields[0].GetDescription())
	}
}

func TestTablePatchSchemaDefaultValueExpressionOverlay(t *testing.T) {
	t.Parallel()
	cat := newSchemaPatchFakeCatalog(&enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: "id", Type: sqlTypeINT64},
	}})
	patch := `{"schema":{"fields":[{"name":"id","type":"` + sqlTypeINT64 +
		`","defaultValueExpression":"GENERATE_UUID()"}]}}`
	got, deps := runTableSchemaPatch(t, cat, patch)
	if got.Schema == nil || got.Schema.Fields[0].DefaultValueExpression != "GENERATE_UUID()" {
		t.Fatalf("patch response schema = %#v", got.Schema)
	}
	afterGet := tableGetSchema(t, deps)
	if afterGet.Schema == nil ||
		afterGet.Schema.Fields[0].DefaultValueExpression != "GENERATE_UUID()" {
		t.Fatalf("get schema = %#v", afterGet.Schema)
	}
}

func TestTablePatchSchemaAddFieldPersists(t *testing.T) {
	t.Parallel()
	cat := newSchemaPatchFakeCatalog(&enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: "id", Type: sqlTypeINT64, Mode: sqlModeRequired},
	}})
	patch := `{"schema":{"fields":[
		{"name":"id","type":"` + sqlTypeINT64 + `","mode":"REQUIRED"},
		{"name":"name","type":"` + sqlTypeSTRING + `"}
	]}}`
	_, _ = runTableSchemaPatch(t, cat, patch)
	if len(cat.schema.GetFields()) != 2 {
		t.Fatalf("engine schema fields = %d, want 2", len(cat.schema.GetFields()))
	}
}

func TestTablePatchSchemaRejectNarrowing(t *testing.T) {
	t.Parallel()
	cat := newSchemaPatchFakeCatalog(&enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: "id", Type: sqlTypeINT64},
	}})
	store := NewMetadataStore()
	deps := Dependencies{Catalog: cat, Metadata: store}
	patchReq := newTableReq(http.MethodPatch, testTableID, `{"schema":{"fields":[
		{"name":"id","type":"`+sqlTypeINT64+`","mode":"REQUIRED"}
	]}}`)
	rec := httptest.NewRecorder()
	TablePatch(deps)(rec, patchReq)
	if rec.Code != http.StatusBadRequest {
		t.Fatalf("patch: status = %d, want 400; body=%s", rec.Code, rec.Body.String())
	}
}

func TestTablePatchResponseMatchesGet(t *testing.T) {
	t.Parallel()
	cat := newSchemaPatchFakeCatalog(&enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: "id", Type: sqlTypeINT64, Mode: sqlModeRequired},
	}})
	patch := `{"schema":{"fields":[
		{"name":"id","type":"` + sqlTypeINT64 + `","mode":"NULLABLE","description":"pk"}
	]}}`
	patched, deps := runTableSchemaPatch(t, cat, patch)
	got := tableGetSchema(t, deps)
	if got.Schema == nil || patched.Schema == nil {
		t.Fatalf("schema missing: patch=%#v get=%#v", patched.Schema, got.Schema)
	}
	if got.Schema.Fields[0].Mode != patched.Schema.Fields[0].Mode ||
		got.Schema.Fields[0].Description != patched.Schema.Fields[0].Description {
		t.Fatalf("patch/get mismatch: patch=%#v get=%#v", patched.Schema.Fields[0], got.Schema.Fields[0])
	}
}
