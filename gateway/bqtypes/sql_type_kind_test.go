package bqtypes_test

import (
	"encoding/json"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func TestSqlTypeKindUnmarshalJSONString(t *testing.T) {
	var dt bqtypes.StandardSqlDataType
	if err := json.Unmarshal([]byte(`{"typeKind":"INT64"}`), &dt); err != nil {
		t.Fatalf("unmarshal: %v", err)
	}
	if dt.TypeKind != "INT64" {
		t.Errorf("typeKind = %q, want INT64", dt.TypeKind)
	}
}

func TestSqlTypeKindUnmarshalJSONNumeric(t *testing.T) {
	var rt bqtypes.Routine
	if err := json.Unmarshal([]byte(`{"arguments":[{"name":"x","dataType":{"typeKind":2}}]}`), &rt); err != nil {
		t.Fatalf("unmarshal: %v", err)
	}
	if rt.Arguments[0].DataType == nil || rt.Arguments[0].DataType.TypeKind != "INT64" {
		t.Fatalf("argument dataType = %#v, want INT64", rt.Arguments[0].DataType)
	}
}
