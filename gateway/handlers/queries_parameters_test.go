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

func TestParametersToEngineMapTimestamp(t *testing.T) {
	t.Parallel()
	params, err := bqtypes.ParseQueryParameters([]byte(`[{
		"name":"ts_value",
		"parameterType":{"type":"TIMESTAMP"},
		"parameterValue":{"value":"2016-12-07T08:00:00+00:00"}
	}]`))
	if err != nil {
		t.Fatalf("ParseQueryParameters: %v", err)
	}
	got := parametersToEngineMap(params)
	p := got["ts_value"]
	if p == nil || p.GetTypeKind() != "TIMESTAMP" ||
		p.GetValueJson() != "2016-12-07T08:00:00+00:00" {
		t.Fatalf("ts_value = %+v", p)
	}
}

func TestParametersToEngineMapStruct(t *testing.T) {
	t.Parallel()
	params, err := bqtypes.ParseQueryParameters([]byte(`[{
		"name":"struct_value",
		"parameterType":{"type":"STRUCT","structTypes":[
			{"name":"x","type":{"type":"INT64"}},
			{"name":"y","type":{"type":"STRING"}}
		]},
		"parameterValue":{"structValues":{
			"x":{"value":"1"},
			"y":{"value":"foo"}
		}}
	}]`))
	if err != nil {
		t.Fatalf("ParseQueryParameters: %v", err)
	}
	got := parametersToEngineMap(params)
	p := got["struct_value"]
	if p == nil || p.GetTypeKind() != "STRUCT" {
		t.Fatalf("struct_value = %+v", p)
	}
	if p.GetTypeJson() != "x:INT64,y:STRING" {
		t.Errorf("TypeJson = %q, want x:INT64,y:STRING", p.GetTypeJson())
	}
	if p.GetValueJson() != `[1,"foo"]` {
		t.Errorf("ValueJson = %q, want [1,\"foo\"]", p.GetValueJson())
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
