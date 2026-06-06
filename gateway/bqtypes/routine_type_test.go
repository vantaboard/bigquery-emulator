package bqtypes_test

import (
	"encoding/json"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func TestRoutineTypeUnmarshalJSONString(t *testing.T) {
	var rt bqtypes.Routine
	if err := json.Unmarshal([]byte(`{"routineType":"SCALAR_FUNCTION"}`), &rt); err != nil {
		t.Fatalf("unmarshal: %v", err)
	}
	if rt.RoutineType != "SCALAR_FUNCTION" {
		t.Errorf("routineType = %q, want SCALAR_FUNCTION", rt.RoutineType)
	}
}

func TestRoutineTypeUnmarshalJSONNumeric(t *testing.T) {
	var rt bqtypes.Routine
	if err := json.Unmarshal([]byte(`{"routineType":1}`), &rt); err != nil {
		t.Fatalf("unmarshal: %v", err)
	}
	if rt.RoutineType != "SCALAR_FUNCTION" {
		t.Errorf("routineType = %q, want SCALAR_FUNCTION", rt.RoutineType)
	}
}

func TestRoutineTypeUnmarshalJSONNumericProcedure(t *testing.T) {
	var rt bqtypes.Routine
	if err := json.Unmarshal([]byte(`{"routineType":2}`), &rt); err != nil {
		t.Fatalf("unmarshal: %v", err)
	}
	if rt.RoutineType != "PROCEDURE" {
		t.Errorf("routineType = %q, want PROCEDURE", rt.RoutineType)
	}
}
