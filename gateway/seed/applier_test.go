package seed

import (
	"context"
	"errors"
	"reflect"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

// TestCatalogApplier_EnsureDataset_ForwardsCall pins the wire shape
// the orchestrator depends on: project / dataset id end up where
// the engine reads them, plus the location string is preserved.
func TestCatalogApplier_EnsureDataset_ForwardsCall(t *testing.T) {
	fake := &fakeCatalogClient{}
	a := NewCatalogApplier(fake)

	created, err := a.EnsureDataset(context.Background(), "proj", "ds", "US")
	if err != nil {
		t.Fatalf("EnsureDataset: %v", err)
	}
	if !created {
		t.Error("created=false on first call; want true")
	}
	if fake.lastRegisterDataset == nil {
		t.Fatal("RegisterDataset not called")
	}
	if got := fake.lastRegisterDataset.GetDataset().GetProjectId(); got != "proj" {
		t.Errorf("projectId=%q, want proj", got)
	}
	if got := fake.lastRegisterDataset.GetDataset().GetDatasetId(); got != "ds" {
		t.Errorf("datasetId=%q, want ds", got)
	}
	if got := fake.lastRegisterDataset.GetLocation(); got != "US" {
		t.Errorf("location=%q, want US", got)
	}
}

// TestCatalogApplier_EnsureDataset_IdempotentOnAlreadyExists pins
// the core "rerun is safe" contract: ALREADY_EXISTS surfaces as
// `created=false, err=nil` so the orchestrator increments
// DatasetsSkipped and proceeds.
func TestCatalogApplier_EnsureDataset_IdempotentOnAlreadyExists(t *testing.T) {
	fake := &fakeCatalogClient{
		registerDatasetFn: func(_ context.Context, _ *enginepb.RegisterDatasetRequest) (*enginepb.RegisterDatasetResponse, error) {
			return nil, status.Error(codes.AlreadyExists, "dataset already exists")
		},
	}
	a := NewCatalogApplier(fake)
	created, err := a.EnsureDataset(context.Background(), "p", "d", "")
	if err != nil {
		t.Fatalf("EnsureDataset: %v", err)
	}
	if created {
		t.Error("created=true on ALREADY_EXISTS; want false (must surface as skipped)")
	}
}

// TestCatalogApplier_EnsureDataset_SurfacesOtherErrors guards
// against a future regression that classifies every gRPC error as
// "skipped" and silently fails to seed.
func TestCatalogApplier_EnsureDataset_SurfacesOtherErrors(t *testing.T) {
	fake := &fakeCatalogClient{
		registerDatasetFn: func(_ context.Context, _ *enginepb.RegisterDatasetRequest) (*enginepb.RegisterDatasetResponse, error) {
			return nil, status.Error(codes.Internal, "boom")
		},
	}
	a := NewCatalogApplier(fake)
	_, err := a.EnsureDataset(context.Background(), "p", "d", "")
	if err == nil {
		t.Fatal("EnsureDataset accepted INTERNAL; should surface as error")
	}
}

// TestCatalogApplier_EnsureTable_Schema verifies the schema travels
// to the engine intact, including nested struct fields.
func TestCatalogApplier_EnsureTable_Schema(t *testing.T) {
	fake := &fakeCatalogClient{}
	a := NewCatalogApplier(fake)
	schema := &enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: "id", Type: bqTypeInt64, Mode: "REQUIRED"},
		{Name: "address", Type: "STRUCT", Fields: []*enginepb.FieldSchema{
			{Name: "city", Type: bqTypeString},
		}},
	}}
	ref := TableRef{ProjectID: "p", DatasetID: "d", TableID: "t"}
	if _, err := a.EnsureTable(context.Background(), ref, schema); err != nil {
		t.Fatalf("EnsureTable: %v", err)
	}
	if fake.lastRegisterTable == nil {
		t.Fatal("RegisterTable not called")
	}
	got := fake.lastRegisterTable
	if got.GetTable().GetTableId() != "t" {
		t.Errorf("tableId=%q", got.GetTable().GetTableId())
	}
	gotFields := got.GetSchema().GetFields()
	if len(gotFields) != 2 {
		t.Fatalf("len(fields)=%d, want 2", len(gotFields))
	}
	if gotFields[1].GetType() != "STRUCT" || len(gotFields[1].GetFields()) != 1 {
		t.Errorf("nested struct not forwarded: %+v", gotFields[1])
	}
}

// TestCatalogApplier_InsertRows_PositionalLayout pins the
// schema-driven layout: rows are laid out positionally so the
// engine's Storage::AppendRows sees the columns in declaration
// order, and missing keys become NULL cells (matching the
// tabledata.insertAll handler).
func TestCatalogApplier_InsertRows_PositionalLayout(t *testing.T) {
	fake := &fakeCatalogClient{}
	a := NewCatalogApplier(fake)
	schema := &enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: "id", Type: bqTypeInt64},
		{Name: colName, Type: bqTypeString},
		{Name: "age", Type: bqTypeInt64},
	}}
	rows := []map[string]any{
		{"id": 1, colName: rowValueAda}, // age missing -> null
		{"id": 2, colName: "bob", "age": 42},
	}
	ref := TableRef{ProjectID: "p", DatasetID: "d", TableID: "t"}
	n, err := a.InsertRows(context.Background(), ref, schema, rows)
	if err != nil {
		t.Fatalf("InsertRows: %v", err)
	}
	if n != 2 {
		t.Errorf("returned %d, want 2", n)
	}
	if fake.lastInsertRows == nil {
		t.Fatal("InsertRows not called")
	}
	got := fake.lastInsertRows.GetRows()
	if len(got) != 2 {
		t.Fatalf("len(rows)=%d, want 2", len(got))
	}
	if got[0].GetCells()[2].GetNullValue() != true {
		t.Errorf("age cell on row 0 should be null; got %+v", got[0].GetCells()[2])
	}
	if got[1].GetCells()[2].GetStringValue() != "42" {
		t.Errorf("age cell on row 1=%q, want 42", got[1].GetCells()[2].GetStringValue())
	}
}

// TestCatalogApplier_InsertRows_EmptyShortCircuits avoids burning a
// gRPC roundtrip on a no-op insert. The fake should never see the
// call.
func TestCatalogApplier_InsertRows_EmptyShortCircuits(t *testing.T) {
	fake := &fakeCatalogClient{}
	a := NewCatalogApplier(fake)
	n, err := a.InsertRows(context.Background(), TableRef{ProjectID: "p", DatasetID: "d", TableID: "t"},
		&enginepb.TableSchema{}, nil)
	if err != nil {
		t.Fatalf("InsertRows: %v", err)
	}
	if n != 0 {
		t.Errorf("returned %d, want 0", n)
	}
	if fake.lastInsertRows != nil {
		t.Error("InsertRows RPC issued for empty row set; should short-circuit")
	}
}

// TestCatalogApplier_NilClientGivesUsefulError prevents the
// historically-common pattern of accidentally passing a nil
// CatalogClient (which would otherwise produce a panic deep in the
// gRPC stack).
func TestCatalogApplier_NilClientGivesUsefulError(t *testing.T) {
	a := NewCatalogApplier(nil)
	for _, fn := range []func() error{
		func() error {
			_, err := a.EnsureDataset(context.Background(), "p", "d", "")
			return err
		},
		func() error {
			_, err := a.EnsureTable(context.Background(), TableRef{ProjectID: "p", DatasetID: "d", TableID: "t"}, nil)
			return err
		},
		func() error {
			_, err := a.InsertRows(context.Background(), TableRef{ProjectID: "p"}, nil, []map[string]any{{"x": 1}})
			return err
		},
	} {
		err := fn()
		if err == nil {
			t.Error("nil CatalogClient did not produce an error")
		}
	}
}

// TestValueToCell_Conversions pins the JSON->Cell mapping the
// YAML loader and orchestrator both depend on. We assert on the
// wire payload directly so a refactor that swaps the variant tag
// flags itself.
func TestValueToCell_Conversions(t *testing.T) {
	cases := []struct {
		name string
		in   any
		want any // wire string for scalars, marker for null/struct/array
	}{
		{literalNil, nil, nil},
		{"string", rowValueAda, rowValueAda},
		{"int", int(42), "42"},
		{"int64", int64(-7), "-7"},
		{"float-integer", 1.0, "1"},
		{"float-fraction", 1.5, "1.5"},
		{"bool-true", true, "true"},
		{"bool-false", false, "false"},
	}
	for _, c := range cases {
		t.Run(c.name, func(t *testing.T) {
			cell := ValueToCell(c.in)
			if c.want == nil {
				if cell.GetNullValue() != true {
					t.Errorf("got %+v, want null cell", cell)
				}
				return
			}
			if got := cell.GetStringValue(); !reflect.DeepEqual(got, c.want) {
				t.Errorf("got %q, want %q", got, c.want)
			}
		})
	}
}

// TestValueToCell_Array verifies array values land in the
// Cell.Array oneof variant with cells preserved in order.
func TestValueToCell_Array(t *testing.T) {
	cell := ValueToCell([]any{int(1), "two", nil})
	arr := cell.GetArray()
	if arr == nil || len(arr.GetElements()) != 3 {
		t.Fatalf("array cell not built: %+v", cell)
	}
	if got := arr.GetElements()[0].GetStringValue(); got != "1" {
		t.Errorf("array[0]=%q, want 1", got)
	}
	if got := arr.GetElements()[1].GetStringValue(); got != "two" {
		t.Errorf("array[1]=%q, want two", got)
	}
	if arr.GetElements()[2].GetNullValue() != true {
		t.Errorf("array[2] not null: %+v", arr.GetElements()[2])
	}
}

// TestIsAlreadyExistsHandlesPlainErrors checks the helper does not
// panic / mis-classify when handed a non-gRPC error.
func TestIsAlreadyExistsHandlesPlainErrors(t *testing.T) {
	if isAlreadyExists(errors.New("boom")) {
		t.Error("non-gRPC error misclassified as ALREADY_EXISTS")
	}
	if isAlreadyExists(nil) {
		t.Error("nil error misclassified as ALREADY_EXISTS")
	}
}
