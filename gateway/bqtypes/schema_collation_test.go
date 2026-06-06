package bqtypes_test

import (
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func TestApplyDefaultCollationToStringFields(t *testing.T) {
	schema := &bqtypes.TableSchema{
		Fields: []bqtypes.TableFieldSchema{
			{Name: "name", Type: "STRING"},
			{Name: "nums", Type: "INTEGER"},
		},
	}
	got := bqtypes.ApplyDefaultCollationToStringFields(schema, "und:ci")
	if got.Fields[0].Collation != "und:ci" {
		t.Fatalf("STRING field collation = %q, want und:ci", got.Fields[0].Collation)
	}
	if got.Fields[1].Collation != "" {
		t.Fatalf("INTEGER field collation = %q, want empty", got.Fields[1].Collation)
	}
}
