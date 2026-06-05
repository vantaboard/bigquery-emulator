package handlers

import (
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

func TestParametersToEngineMapNamedAndPositional(t *testing.T) {
	t.Parallel()

	params, err := bqtypes.ParseQueryParameters([]byte(`[
		{
			"name":"corpus",
			"parameterType":{"type":"STRING"},
			"parameterValue":{"value":"romeoandjuliet"}
		},
		{
			"parameterType":{"type":"INT64"},
			"parameterValue":{"value":250}
		}
	]`))
	if err != nil {
		t.Fatalf("ParseQueryParameters: %v", err)
	}

	got := parametersToEngineMap(params)
	if len(got) != 2 {
		t.Fatalf("map len = %d, want 2", len(got))
	}
	if p := got["corpus"]; p == nil || p.GetTypeKind() != sqlTypeSTRING ||
		p.GetValueJson() != "romeoandjuliet" {
		t.Errorf("corpus = %+v", p)
	}
	if p := got["p0"]; p == nil || p.GetTypeKind() != sqlTypeINT64 ||
		p.GetValueJson() != "250" {
		t.Errorf("p0 = %+v, want INT64/250", p)
	}
}

func TestParametersToEngineMapSkipsUntyped(t *testing.T) {
	t.Parallel()
	got := parametersToEngineMap([]bqtypes.QueryParameter{{
		Name:           "orphan",
		ParameterValue: &bqtypes.QueryParameterValue{Value: "1"},
	}})
	if len(got) != 0 {
		t.Errorf("got = %+v, want empty (missing parameterType)", got)
	}
}

func TestParametersToEngineMapEmpty(t *testing.T) {
	t.Parallel()
	if got := parametersToEngineMap(nil); got != nil {
		t.Errorf("got = %+v, want nil", got)
	}
}

// Compile-time guard that the proto type is still wired as expected.
var _ = enginepb.QueryParameter{}
