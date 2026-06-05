package bqtypes_test

import (
	"encoding/json"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func TestQueryParameterValueUnmarshalNumericAndBool(t *testing.T) {
	t.Parallel()

	const payload = `[
		{
			"name":"min_word_count",
			"parameterType":{"type":"INT64"},
			"parameterValue":{"value":250}
		},
		{
			"name":"active",
			"parameterType":{"type":"BOOL"},
			"parameterValue":{"value":true}
		},
		{
			"name":"ratio",
			"parameterType":{"type":"FLOAT64"},
			"parameterValue":{"value":3.14}
		}
	]`

	params, err := bqtypes.ParseQueryParameters([]byte(payload))
	if err != nil {
		t.Fatalf("ParseQueryParameters: %v", err)
	}
	if len(params) != 3 {
		t.Fatalf("params = %d, want 3", len(params))
	}
	if got := params[0].ParameterValue.Value; got != "250" {
		t.Errorf("INT64 value = %q, want %q", got, "250")
	}
	if got := params[1].ParameterValue.Value; got != "true" {
		t.Errorf("BOOL value = %q, want %q", got, "true")
	}
	if got := params[2].ParameterValue.Value; got != "3.14" {
		t.Errorf("FLOAT64 value = %q, want %q", got, "3.14")
	}
}

func TestQueryParameterValueValueJSON(t *testing.T) {
	t.Parallel()

	var v bqtypes.QueryParameterValue
	if err := json.Unmarshal([]byte(`{"value":42}`), &v); err != nil {
		t.Fatalf("Unmarshal: %v", err)
	}
	if got := v.ValueJSON(); got != "42" {
		t.Errorf("ValueJSON() = %q, want %q", got, "42")
	}
}
