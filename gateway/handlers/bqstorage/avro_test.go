package bqstorage

import (
	"reflect"
	"testing"

	"cloud.google.com/go/bigquery/storage/apiv1/storagepb"
	goavro "github.com/linkedin/goavro/v2"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

func TestRowsToAvroBatchRoundTrip(t *testing.T) {
	schema := &enginepb.TableSchema{
		Fields: []*enginepb.FieldSchema{
			{Name: "id", Type: bqTypeINT64, Mode: bqModeRequired},
			{Name: "name", Type: bqTypeSTRING, Mode: bqModeNullable},
			{Name: "active", Type: bqTypeBOOL, Mode: bqModeNullable},
		},
	}
	rows := []*enginepb.DataRow{
		{Cells: []*enginepb.Cell{
			{Value: &enginepb.Cell_StringValue{StringValue: "1"}},
			{Value: &enginepb.Cell_StringValue{StringValue: "ada"}},
			{Value: &enginepb.Cell_StringValue{StringValue: "true"}},
		}},
		{Cells: []*enginepb.Cell{
			{Value: &enginepb.Cell_StringValue{StringValue: "2"}},
			{Value: &enginepb.Cell_NullValue{NullValue: true}},
			{Value: &enginepb.Cell_StringValue{StringValue: "false"}},
		}},
	}

	batch, err := rowsToAvroBatch(schema, rows)
	if err != nil {
		t.Fatalf("rowsToAvroBatch: %v", err)
	}
	if len(rows) != 2 {
		t.Fatalf("fixture rows = %d, want 2", len(rows))
	}
	if len(batch.GetSerializedBinaryRows()) == 0 {
		t.Fatal("expected non-empty serialized_binary_rows")
	}

	schemaJSON, err := avroSchemaJSONFromEngine(schema)
	if err != nil {
		t.Fatalf("avroSchemaJSONFromEngine: %v", err)
	}
	codec, err := goavro.NewCodec(schemaJSON)
	if err != nil {
		t.Fatalf("NewCodec: %v", err)
	}

	remaining := batch.GetSerializedBinaryRows()
	var decoded int
	for len(remaining) > 0 {
		native, buf, decErr := codec.NativeFromBinary(remaining)
		if decErr != nil {
			t.Fatalf("NativeFromBinary at row %d: %v", decoded, decErr)
		}
		record, ok := native.(map[string]any)
		if !ok {
			t.Fatalf("row %d: want map[string]any, got %T", decoded, native)
		}
		if record["id"] != int64(1) && record["id"] != int64(2) {
			t.Fatalf("row %d id = %v", decoded, record["id"])
		}
		remaining = buf
		decoded++
	}
	if decoded != 2 {
		t.Fatalf("decoded %d rows, want 2", decoded)
	}
}

const (
	nestedColTags   = "tags"
	nestedColUser   = "user"
	nestedColEvents = "events"
	nestedColIDs    = "ids"
)

func TestRowsToAvroBatchNestedRoundTrip(t *testing.T) {
	schema := nestedTestSchema()
	batch, err := rowsToAvroBatch(schema, nestedTestRows())
	if err != nil {
		t.Fatalf("rowsToAvroBatch: %v", err)
	}
	schemaJSON, err := avroSchemaJSONFromEngine(schema)
	if err != nil {
		t.Fatalf("avroSchemaJSONFromEngine: %v", err)
	}
	codec, err := goavro.NewCodec(schemaJSON)
	if err != nil {
		t.Fatalf("NewCodec: %v", err)
	}

	want := nestedAvroWant()
	remaining := batch.GetSerializedBinaryRows()
	for i, w := range want {
		native, buf, decErr := codec.NativeFromBinary(remaining)
		if decErr != nil {
			t.Fatalf("NativeFromBinary at row %d: %v", i, decErr)
		}
		if !reflect.DeepEqual(native, w) {
			t.Errorf("row %d = %#v, want %#v", i, native, w)
		}
		remaining = buf
	}
	if len(remaining) != 0 {
		t.Fatalf("%d bytes left after %d rows", len(remaining), len(want))
	}
}

func nestedTestSchema() *enginepb.TableSchema {
	return &enginepb.TableSchema{
		Fields: []*enginepb.FieldSchema{
			{Name: "id", Type: bqTypeINT64, Mode: bqModeRequired},
			{Name: nestedColTags, Type: bqTypeSTRING, Mode: bqModeRepeated},
			{Name: nestedColUser, Type: bqTypeSTRUCT, Mode: bqModeNullable, Fields: []*enginepb.FieldSchema{
				{Name: "login", Type: bqTypeSTRING, Mode: bqModeNullable},
				{Name: "address", Type: bqTypeRECORD, Mode: bqModeNullable, Fields: []*enginepb.FieldSchema{
					{Name: "city", Type: bqTypeSTRING, Mode: bqModeNullable},
				}},
			}},
			{Name: nestedColEvents, Type: bqTypeRECORD, Mode: bqModeRepeated, Fields: []*enginepb.FieldSchema{
				{Name: "kind", Type: bqTypeSTRING, Mode: bqModeNullable},
			}},
			{Name: nestedColIDs, Type: bqTypeARRAY, Mode: bqModeNullable, Fields: []*enginepb.FieldSchema{
				{Type: bqTypeINT64},
			}},
		},
	}
}

func nestedTestRows() []*enginepb.DataRow {
	null := &enginepb.Cell{Value: &enginepb.Cell_NullValue{NullValue: true}}
	return []*enginepb.DataRow{
		{Cells: []*enginepb.Cell{
			stringCell("1"),
			arrayCell(stringCell("a"), stringCell("b")),
			structCell(stringCell("ada"), structCell(stringCell("Paris"))),
			arrayCell(structCell(stringCell("view"))),
			arrayCell(stringCell("3"), stringCell("5")),
		}},
		{Cells: []*enginepb.Cell{stringCell("2"), null, null, null, null}},
	}
}

func nestedAvroWant() []map[string]any {
	return []map[string]any{
		{
			"id":          int64(1),
			nestedColTags: []any{"a", "b"},
			nestedColUser: map[string]any{"root__user": map[string]any{
				"login": map[string]any{avroTypeString: "ada"},
				"address": map[string]any{"root__user__address": map[string]any{
					"city": map[string]any{avroTypeString: "Paris"},
				}},
			}},
			nestedColEvents: []any{map[string]any{"kind": map[string]any{avroTypeString: "view"}}},
			nestedColIDs:    []any{int64(3), int64(5)},
		},
		{"id": int64(2), nestedColTags: []any{}, nestedColUser: nil, nestedColEvents: []any{}, nestedColIDs: []any{}},
	}
}

func stringCell(s string) *enginepb.Cell {
	return &enginepb.Cell{Value: &enginepb.Cell_StringValue{StringValue: s}}
}

func arrayCell(cells ...*enginepb.Cell) *enginepb.Cell {
	return &enginepb.Cell{Value: &enginepb.Cell_Array{Array: &enginepb.Array{Elements: cells}}}
}

func structCell(cells ...*enginepb.Cell) *enginepb.Cell {
	return &enginepb.Cell{Value: &enginepb.Cell_StructValue{StructValue: &enginepb.Struct{Fields: cells}}}
}

func TestPublicReadSessionFromEngineAvro(t *testing.T) {
	session, err := publicReadSessionFromEngine(&enginepb.ReadSession{
		Name:  "projects/p/locations/-/sessions/s1",
		Table: testTableResource,
		Schema: &enginepb.TableSchema{
			Fields: []*enginepb.FieldSchema{
				{Name: "id", Type: bqTypeINT64, Mode: bqModeRequired},
			},
		},
		Streams: []*enginepb.ReadStream{{Name: "projects/p/locations/-/sessions/s1/streams/0"}},
	}, storagepb.DataFormat_AVRO)
	if err != nil {
		t.Fatalf("publicReadSessionFromEngine: %v", err)
	}
	if session.GetAvroSchema() == nil || session.GetAvroSchema().GetSchema() == "" {
		t.Fatal("expected non-empty avro_schema")
	}
	if session.GetDataFormat() != storagepb.DataFormat_AVRO {
		t.Fatalf("data_format = %v, want AVRO", session.GetDataFormat())
	}
}
