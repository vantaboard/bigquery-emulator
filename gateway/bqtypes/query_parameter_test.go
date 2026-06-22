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

func TestParameterTypeWireArray(t *testing.T) {
	t.Parallel()

	kind, typeJSON := bqtypes.ParameterTypeWire(&bqtypes.QueryParameterType{
		Type:      typeKindARRAY,
		ArrayType: &bqtypes.QueryParameterType{Type: schemaTypeSTRING},
	})
	if kind != typeKindARRAY || typeJSON != schemaTypeSTRING {
		t.Errorf("ARRAY<STRING> = (%q, %q), want (%s, %s)", kind, typeJSON, typeKindARRAY, schemaTypeSTRING)
	}

	kind, typeJSON = bqtypes.ParameterTypeWire(&bqtypes.QueryParameterType{
		Type: typeKindARRAY,
		ArrayType: &bqtypes.QueryParameterType{
			Type: schemaTypeSTRUCT,
			StructTypes: []bqtypes.QueryParameterStructType{
				{Name: "x", Type: bqtypes.QueryParameterType{Type: typeKindINT64}},
				{Name: "y", Type: bqtypes.QueryParameterType{Type: schemaTypeSTRING}},
			},
		},
	})
	wantStructTypeJSON := schemaTypeSTRUCT + ":x:" + typeKindINT64 + ",y:" + schemaTypeSTRING
	if kind != typeKindARRAY || typeJSON != wantStructTypeJSON {
		t.Errorf("ARRAY<STRUCT> = (%q, %q), want (%s, %s)",
			kind, typeJSON, typeKindARRAY, wantStructTypeJSON)
	}
}

func TestQueryParameterWireMatrixPreservesRawStrings(t *testing.T) {
	t.Parallel()

	cases := []struct {
		name      string
		payload   string
		wantValue string
	}{
		{
			name: "timestamp_naive_iso",
			payload: `{"name":"p","parameterType":{"type":"TIMESTAMP"},
				"parameterValue":{"value":"2026-06-22T10:00:00"}}`,
			wantValue: "2026-06-22T10:00:00",
		},
		{
			name: "timestamp_space",
			payload: `{"name":"p","parameterType":{"type":"TIMESTAMP"},
				"parameterValue":{"value":"2026-06-22 10:00:00"}}`,
			wantValue: "2026-06-22 10:00:00",
		},
		{
			name: "date_iso",
			payload: `{"name":"p","parameterType":{"type":"DATE"},
				"parameterValue":{"value":"2020-06-15"}}`,
			wantValue: "2020-06-15",
		},
		{
			name: "datetime_iso_t",
			payload: `{"name":"p","parameterType":{"type":"DATETIME"},
				"parameterValue":{"value":"2020-06-15T12:30:45"}}`,
			wantValue: "2020-06-15T12:30:45",
		},
		{
			name: "time_fractional",
			payload: `{"name":"p","parameterType":{"type":"TIME"},
				"parameterValue":{"value":"12:30:45.123456"}}`,
			wantValue: "12:30:45.123456",
		},
		{
			name: "bytes_base64",
			payload: `{"name":"p","parameterType":{"type":"BYTES"},
				"parameterValue":{"value":"SGVsbG8="}}`,
			wantValue: "SGVsbG8=",
		},
	}

	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			t.Parallel()
			params, err := bqtypes.ParseQueryParameters([]byte("[" + tc.payload + "]"))
			if err != nil {
				t.Fatalf("ParseQueryParameters: %v", err)
			}
			if got := params[0].ParameterValue.Value; got != tc.wantValue {
				t.Errorf("Value = %q, want %q", got, tc.wantValue)
			}
		})
	}
}
