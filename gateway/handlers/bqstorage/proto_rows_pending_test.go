package bqstorage

import (
	"strconv"
	"testing"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/protobuf/proto"
	"google.golang.org/protobuf/reflect/protoreflect"
	"google.golang.org/protobuf/types/descriptorpb"
	"google.golang.org/protobuf/types/dynamicpb"
)

// pendingManagedWriterSchema mirrors
// third_party/golang-bigquery-tests/snippets/managedwriter/integration_test.go.
func pendingManagedWriterSchema() *enginepb.TableSchema {
	return &enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: "bool_col", Type: bqTypeBOOL},
		{Name: "bytes_col", Type: bqTypeBYTES},
		{Name: "float64_col", Type: bqTypeFLOAT64},
		{Name: "int64_col", Type: bqTypeINTEGER},
		{Name: "string_col", Type: bqTypeSTRING},
		{Name: "date_col", Type: bqTypeDATE},
		{Name: "datetime_col", Type: bqTypeDATETIME},
		{Name: "geography_col", Type: bqTypeGEOGRAPHY},
		{Name: "numeric_col", Type: bqTypeNUMERIC},
		{Name: "bignumeric_col", Type: bqTypeBIGNUMERIC},
		{Name: "time_col", Type: bqTypeTIME},
		{Name: "timestamp_col", Type: bqTypeTIMESTAMP},
		{Name: "int64_list", Type: bqTypeINTEGER, Mode: "REPEATED"},
		{Name: "struct_col", Type: bqTypeRECORD, Mode: "NULLABLE", Fields: []*enginepb.FieldSchema{
			{Name: "sub_int_col", Type: bqTypeINTEGER},
		}},
		{Name: "struct_list", Type: bqTypeRECORD, Mode: "REPEATED", Fields: []*enginepb.FieldSchema{
			{Name: "sub_int_col", Type: bqTypeINTEGER},
		}},
		{Name: "row_num", Type: bqTypeINTEGER, Mode: "REQUIRED"},
	}}
}

func TestDescriptorFromEngineTableSchemaTimestampFieldType(t *testing.T) {
	t.Parallel()
	desc := descriptorFromEngineTableSchema(pendingManagedWriterSchema())
	if desc == nil {
		t.Fatal("nil descriptor")
	}
	for _, f := range desc.Field {
		if f.GetName() != "timestamp_col" {
			continue
		}
		if f.GetType() != descriptorpb.FieldDescriptorProto_TYPE_INT64 {
			t.Fatalf("timestamp_col proto type = %v, want INT64", f.GetType())
		}
		if f.GetNumber() != 12 {
			t.Fatalf("timestamp_col number = %d, want 12", f.GetNumber())
		}
		return
	}
	t.Fatal("timestamp_col missing from descriptor")
}

func TestFallbackDescriptorDecodesTimestampMicros(t *testing.T) {
	t.Parallel()
	micros := time.Now().UnixNano() / 1000
	desc := descriptorFromEngineTableSchema(pendingManagedWriterSchema())
	md, err := messageDescriptor(desc)
	if err != nil {
		t.Fatal(err)
	}
	msg := dynamicpb.NewMessage(md)
	tsFD := md.Fields().ByName("timestamp_col")
	if tsFD == nil {
		t.Fatal("timestamp_col field missing")
	}
	msg.Set(tsFD, protoreflect.ValueOfInt64(micros))
	rowNumFD := md.Fields().ByName("row_num")
	msg.Set(rowNumFD, protoreflect.ValueOfInt64(23))

	raw, err := proto.Marshal(msg)
	if err != nil {
		t.Fatal(err)
	}
	dyn := dynamicpb.NewMessage(md)
	if unmarshalErr := proto.Unmarshal(raw, dyn); unmarshalErr != nil {
		t.Fatal(unmarshalErr)
	}
	row, err := dynamicMessageToDataRow(dyn)
	if err != nil {
		t.Fatal(err)
	}
	if len(row.Cells) < 12 {
		t.Fatalf("cells = %d, want >= 12", len(row.Cells))
	}
	tsCell := row.Cells[11].GetStringValue()
	if tsCell != strconv.FormatInt(micros, 10) {
		t.Fatalf("timestamp cell = %q, want %d", tsCell, micros)
	}
	if _, err := strconv.ParseFloat(tsCell, 64); err == nil && len(tsCell) < 13 {
		t.Fatalf("timestamp cell looks like float seconds, not micros: %q", tsCell)
	}
}
