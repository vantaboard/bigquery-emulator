package bqstorage

import (
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
