package bqtypes_test

import (
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func TestApplyDefaultCollationToStringFields(t *testing.T) {
	schema := &bqtypes.TableSchema{
		Fields: []bqtypes.TableFieldSchema{
			{Name: "name", Type: schemaTypeSTRING},
			{Name: "nums", Type: schemaTypeINTEGER},
		},
	}
	got := bqtypes.ApplyDefaultCollationToStringFields(schema, collationUndCI)
	if got.Fields[0].Collation != collationUndCI {
		t.Fatalf("STRING field collation = %q, want %s", got.Fields[0].Collation, collationUndCI)
	}
	if got.Fields[1].Collation != "" {
		t.Fatalf("INTEGER field collation = %q, want empty", got.Fields[1].Collation)
	}
}
