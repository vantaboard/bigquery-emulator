package handlers

import (
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

func TestStripExpandedPositionalArrayParams(t *testing.T) {
	t.Parallel()
	params, err := bqtypes.ParseQueryParameters([]byte(`[
		{"parameterType":{"type":"` + sqlTypeARRAY + `","arrayType":{"type":"` + sqlTypeSTRING + `"}},
		 "parameterValue":{"arrayValues":[{"value":"a"}]}},
		{"parameterType":{"type":"` + sqlTypeSTRING + `"},"parameterValue":{"value":"romeo"}},
		{"parameterType":{"type":"` + sqlTypeINT64 + `"},"parameterValue":{"value":"1"}}
	]`))
	if err != nil {
		t.Fatalf("ParseQueryParameters: %v", err)
	}
	sql := `SELECT 1 FROM t WHERE word IN UNNEST(?) AND corpus = ? AND n >= ?`
	got := stripExpandedPositionalArrayParams(sql, params)
	if len(got) != 2 {
		t.Fatalf("len = %d, want 2 (array stripped)", len(got))
	}
}

func TestExpandPositionalArrayParamsInSQL(t *testing.T) {
	t.Parallel()
	params, err := bqtypes.ParseQueryParameters([]byte(`[{
		"parameterType":{"type":"` + sqlTypeARRAY + `","arrayType":{"type":"` + sqlTypeSTRING + `"}},
		"parameterValue":{"arrayValues":[{"value":"and"},{"value":"is"}]}
	}]`))
	if err != nil {
		t.Fatalf("ParseQueryParameters: %v", err)
	}
	sql := `SELECT 1 FROM t WHERE word IN UNNEST(?) AND corpus = ?`
	got := expandQueryParamsInSQL(sql, params)
	want := `SELECT 1 FROM t WHERE word IN ('and', 'is') AND corpus = ?`
	if got != want {
		t.Fatalf("expandQueryParamsInSQL:\n got:  %s\n want: %s", got, want)
	}
}

func TestExpandNamedArrayParamsNotInUnnest(t *testing.T) {
	t.Parallel()
	params, err := bqtypes.ParseQueryParameters([]byte(`[{
		"name":"p0",
		"parameterType":{"type":"` + sqlTypeARRAY + `","arrayType":{"type":"` + sqlTypeSTRING + `"}},
		"parameterValue":{"arrayValues":[{"value":"voided"},{"value":"refunded"}]}
	},{
		"name":"p1",
		"parameterType":{"type":"` + sqlTypeINT64 + `"},
		"parameterValue":{"value":"1"}
	}]`))
	if err != nil {
		t.Fatalf("ParseQueryParameters: %v", err)
	}
	sql := `SELECT 1 FROM t WHERE status NOT IN UNNEST(@p0) AND n >= @p1`
	expanded := expandQueryParamsInSQL(sql, params)
	want := `SELECT 1 FROM t WHERE status NOT IN ('voided', 'refunded') AND n >= @p1`
	if expanded != want {
		t.Fatalf("expandQueryParamsInSQL:\n got:  %s\n want: %s", expanded, want)
	}
	got := stripExpandedArrayParams(sql, expanded, params)
	if len(got) != 1 {
		t.Fatalf("stripExpandedArrayParams len = %d, want 1 (array stripped)", len(got))
	}
	if got[0].Name != "p1" {
		t.Fatalf("remaining param = %q, want p1", got[0].Name)
	}
}

func TestParametersToEngineMapNamedAndPositional(t *testing.T) {
	t.Parallel()

	params, err := bqtypes.ParseQueryParameters([]byte(`[
		{
			"name":"corpus",
			"parameterType":{"type":"` + sqlTypeSTRING + `"},
			"parameterValue":{"value":"romeoandjuliet"}
		},
		{
			"parameterType":{"type":"` + sqlTypeINT64 + `"},
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
	if p := got["__pos_0"]; p == nil || p.GetTypeKind() != sqlTypeINT64 ||
		p.GetValueJson() != "250" {
		t.Errorf("__pos_0 = %+v, want INT64/250", p)
	}
}

func TestParametersToEngineMapTimestamp(t *testing.T) {
	t.Parallel()
	params, err := bqtypes.ParseQueryParameters([]byte(`[{
		"name":"ts_value",
		"parameterType":{"type":"` + sqlTypeTIMESTAMP + `"},
		"parameterValue":{"value":"2016-12-07T08:00:00+00:00"}
	}]`))
	if err != nil {
		t.Fatalf("ParseQueryParameters: %v", err)
	}
	got := parametersToEngineMap(params)
	p := got["ts_value"]
	if p == nil || p.GetTypeKind() != sqlTypeTIMESTAMP ||
		p.GetValueJson() != "2016-12-07T08:00:00+00:00" {
		t.Fatalf("ts_value = %+v", p)
	}
}

func TestParametersToEngineMapArray(t *testing.T) {
	t.Parallel()
	params, err := bqtypes.ParseQueryParameters([]byte(`[{
		"name":"states",
		"parameterType":{"type":"` + sqlTypeARRAY + `","arrayType":{"type":"` + sqlTypeSTRING + `"}},
		"parameterValue":{"arrayValues":[
			{"value":"WA"},
			{"value":"WI"},
			{"value":"WV"},
			{"value":"WY"}
		]}
	}]`))
	if err != nil {
		t.Fatalf("ParseQueryParameters: %v", err)
	}
	got := parametersToEngineMap(params)
	p := got["states"]
	if p == nil || p.GetTypeKind() != sqlTypeARRAY {
		t.Fatalf("states = %+v", p)
	}
	if p.GetTypeJson() != sqlTypeSTRING {
		t.Errorf("TypeJson = %q, want %s", p.GetTypeJson(), sqlTypeSTRING)
	}
	if p.GetValueJson() != `["WA","WI","WV","WY"]` {
		t.Errorf("ValueJson = %q, want [\"WA\",\"WI\",\"WV\",\"WY\"]", p.GetValueJson())
	}
}

func TestParametersToEngineMapStruct(t *testing.T) {
	t.Parallel()
	params, err := bqtypes.ParseQueryParameters([]byte(`[{
		"name":"struct_value",
		"parameterType":{"type":"STRUCT","structTypes":[
			{"name":"x","type":{"type":"` + sqlTypeINT64 + `"}},
			{"name":"y","type":{"type":"` + sqlTypeSTRING + `"}}
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
	if p.GetTypeJson() != "x:"+sqlTypeINT64+",y:"+sqlTypeSTRING {
		t.Errorf("TypeJson = %q, want x:%s,y:%s", p.GetTypeJson(), sqlTypeINT64, sqlTypeSTRING)
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
