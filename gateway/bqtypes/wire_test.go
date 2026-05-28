package bqtypes_test

import (
	"encoding/base64"
	"encoding/json"
	"reflect"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

// stringCell wraps a raw engine string-typed cell. It is the most
// common shape: the engine has already serialized the value per
// StandardSqlDataType.TypeKind.
func stringCell(s string) *enginepb.Cell {
	return &enginepb.Cell{Value: &enginepb.Cell_StringValue{StringValue: s}}
}

func nullCell() *enginepb.Cell {
	return &enginepb.Cell{Value: &enginepb.Cell_NullValue{NullValue: true}}
}

func arrayCell(elems ...*enginepb.Cell) *enginepb.Cell {
	return &enginepb.Cell{Value: &enginepb.Cell_Array{
		Array: &enginepb.Array{Elements: elems},
	}}
}

func structCell(fields ...*enginepb.Cell) *enginepb.Cell {
	return &enginepb.Cell{Value: &enginepb.Cell_StructValue{
		StructValue: &enginepb.Struct{Fields: fields},
	}}
}

// TestWireValueToCellScalars covers every scalar StandardSqlDataType
// TypeKind documented in docs/REST_API.md "Type wire encoding". The
// engine pre-stringifies values per BigQuery's conventions, so
// ValueToCell is a pass-through here; the test pins down the wire
// shape so any future engine-side format drift is caught at the
// gateway boundary.
func TestWireValueToCellScalars(t *testing.T) {
	t.Parallel()

	cases := []struct {
		name    string
		typeKnd string
		in      *enginepb.Cell
		wantV   any
	}{
		{"INT64", "INT64", stringCell("42"), "42"},
		{"INT64_negative", "INT64", stringCell("-9223372036854775808"), "-9223372036854775808"},
		{"FLOAT64_decimal", "FLOAT64", stringCell("3.14"), "3.14"},
		{"FLOAT64_NaN", "FLOAT64", stringCell("NaN"), "NaN"},
		{"FLOAT64_PosInf", "FLOAT64", stringCell("Infinity"), "Infinity"},
		{"FLOAT64_NegInf", "FLOAT64", stringCell("-Infinity"), "-Infinity"},
		{"BOOL_true", "BOOL", stringCell("true"), "true"},
		{"BOOL_false", "BOOL", stringCell("false"), "false"},
		{"STRING", "STRING", stringCell("hello world"), "hello world"},
		{"STRING_empty", "STRING", stringCell(""), ""},
		{
			"BYTES", "BYTES",
			stringCell(base64.StdEncoding.EncodeToString([]byte{0xDE, 0xAD, 0xBE, 0xEF})),
			"3q2+7w==",
		},
		{"DATE", "DATE", stringCell("1985-04-12"), "1985-04-12"},
		{
			"TIMESTAMP", "TIMESTAMP",
			stringCell("1985-04-12T23:20:50.520000Z"),
			"1985-04-12T23:20:50.520000Z",
		},
		{
			"DATETIME", "DATETIME",
			stringCell("1985-04-12 23:20:50.520000"),
			"1985-04-12 23:20:50.520000",
		},
		{"TIME", "TIME", stringCell("23:20:50.520000"), "23:20:50.520000"},
		{"NUMERIC", "NUMERIC", stringCell("12345.6789"), "12345.6789"},
		{
			"BIGNUMERIC", "BIGNUMERIC",
			stringCell("578960446186580977117854925043439539266.34992332820282019728792003956564819967"),
			"578960446186580977117854925043439539266.34992332820282019728792003956564819967",
		},
		{"GEOGRAPHY", "GEOGRAPHY", stringCell("POINT(1 2)"), "POINT(1 2)"},
		{"JSON", "JSON", stringCell(`{"k":"v"}`), `{"k":"v"}`},
	}

	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			t.Parallel()
			got := bqtypes.ValueToCell(tc.in)
			if !reflect.DeepEqual(got, bqtypes.Cell{V: tc.wantV}) {
				t.Fatalf("%s (TypeKind=%s): got %#v, want %#v",
					tc.name, tc.typeKnd, got, bqtypes.Cell{V: tc.wantV})
			}
		})
	}
}

// TestWireValueToCellNull pins down NULL handling for both an
// explicit `null_value=true` proto cell and a nil pointer (which
// callers occasionally pass when a column is absent in an upstream
// row).
func TestWireValueToCellNull(t *testing.T) {
	t.Parallel()

	got := bqtypes.ValueToCell(nullCell())
	if got.V != nil {
		t.Fatalf("explicit null: got %#v, want Cell{V: nil}", got)
	}

	got = bqtypes.ValueToCell(nil)
	if got.V != nil {
		t.Fatalf("nil cell pointer: got %#v, want Cell{V: nil}", got)
	}
}

// TestWireValueToCellArray covers the ARRAY TypeKind, including a NULL
// element. BigQuery encodes ARRAYs as `{"v": [{"v": ...}, ...]}` at
// the wire level.
func TestWireValueToCellArray(t *testing.T) {
	t.Parallel()

	in := arrayCell(stringCell("1"), nullCell(), stringCell("3"))
	got := bqtypes.ValueToCell(in)

	want := bqtypes.Cell{V: []bqtypes.Cell{
		{V: "1"},
		{V: nil},
		{V: "3"},
	}}
	if !reflect.DeepEqual(got, want) {
		t.Fatalf("got %#v, want %#v", got, want)
	}

	t.Run("empty", func(t *testing.T) {
		t.Parallel()
		empty := bqtypes.ValueToCell(arrayCell())
		want := bqtypes.Cell{V: []bqtypes.Cell{}}
		if !reflect.DeepEqual(empty, want) {
			t.Fatalf("got %#v, want %#v", empty, want)
		}
	})
}

// TestWireValueToCellStruct covers the STRUCT TypeKind. STRUCTs are
// encoded as `{"v": {"f": [{"v": ...}, ...]}}` — i.e. the cell's `V`
// is a Row, not a plain slice, so the JSON round-trip emits the `f`
// key. Verified here by both reflect.DeepEqual and a JSON marshal
// sanity check.
func TestWireValueToCellStruct(t *testing.T) {
	t.Parallel()

	in := structCell(stringCell("alice"), stringCell("42"), nullCell())
	got := bqtypes.ValueToCell(in)

	want := bqtypes.Cell{V: bqtypes.Row{F: []bqtypes.Cell{
		{V: "alice"},
		{V: "42"},
		{V: nil},
	}}}
	if !reflect.DeepEqual(got, want) {
		t.Fatalf("got %#v, want %#v", got, want)
	}

	// JSON shape: {"v":{"f":[{"v":"alice"},{"v":"42"},{"v":null}]}}.
	raw, err := json.Marshal(got)
	if err != nil {
		t.Fatalf("json.Marshal: %v", err)
	}
	const wantJSON = `{"v":{"f":[{"v":"alice"},{"v":"42"},{"v":null}]}}`
	if string(raw) != wantJSON {
		t.Fatalf("json marshal mismatch:\n got: %s\nwant: %s", raw, wantJSON)
	}
}

// TestWireValueToCellNestedStruct exercises a STRUCT-inside-STRUCT
// and an ARRAY-inside-STRUCT, mirroring what queries on schemas like
// `STRUCT<name STRING, scores ARRAY<INT64>, inner STRUCT<x INT64>>`
// emit. The recursion must keep arrays as plain slices and structs
// as Row-shaped objects.
func TestWireValueToCellNestedStruct(t *testing.T) {
	t.Parallel()

	in := structCell(
		stringCell("bob"),
		arrayCell(stringCell("10"), stringCell("20")),
		structCell(stringCell("99")),
	)
	got := bqtypes.ValueToCell(in)

	want := bqtypes.Cell{V: bqtypes.Row{F: []bqtypes.Cell{
		{V: "bob"},
		{V: []bqtypes.Cell{{V: "10"}, {V: "20"}}},
		{V: bqtypes.Row{F: []bqtypes.Cell{{V: "99"}}}},
	}}}
	if !reflect.DeepEqual(got, want) {
		t.Fatalf("got %#v, want %#v", got, want)
	}

	raw, err := json.Marshal(got)
	if err != nil {
		t.Fatalf("json.Marshal: %v", err)
	}
	const wantJSON = `{"v":{"f":[{"v":"bob"},{"v":[{"v":"10"},{"v":"20"}]},{"v":{"f":[{"v":"99"}]}}]}}`
	if string(raw) != wantJSON {
		t.Fatalf("json marshal mismatch:\n got: %s\nwant: %s", raw, wantJSON)
	}
}

// TestWireValueToCellArrayOfStruct covers ARRAY<STRUCT<...>>, a shape
// the engine emits for repeated record fields and nested subqueries.
// Each element is itself a Cell whose `V` is a Row.
func TestWireValueToCellArrayOfStruct(t *testing.T) {
	t.Parallel()

	in := arrayCell(
		structCell(stringCell("1"), stringCell("a")),
		structCell(stringCell("2"), nullCell()),
	)
	got := bqtypes.ValueToCell(in)

	want := bqtypes.Cell{V: []bqtypes.Cell{
		{V: bqtypes.Row{F: []bqtypes.Cell{{V: "1"}, {V: "a"}}}},
		{V: bqtypes.Row{F: []bqtypes.Cell{{V: "2"}, {V: nil}}}},
	}}
	if !reflect.DeepEqual(got, want) {
		t.Fatalf("got %#v, want %#v", got, want)
	}
}

// TestWireCellsToRow checks the top-level Row helper that the query
// handler will call once per streamed result row.
func TestWireCellsToRow(t *testing.T) {
	t.Parallel()

	in := []*enginepb.Cell{
		stringCell("42"),
		stringCell("hello"),
		nullCell(),
		arrayCell(stringCell("x"), stringCell("y")),
	}
	got := bqtypes.CellsToRow(in)

	want := bqtypes.Row{F: []bqtypes.Cell{
		{V: "42"},
		{V: "hello"},
		{V: nil},
		{V: []bqtypes.Cell{{V: "x"}, {V: "y"}}},
	}}
	if !reflect.DeepEqual(got, want) {
		t.Fatalf("got %#v, want %#v", got, want)
	}

	t.Run("empty", func(t *testing.T) {
		t.Parallel()
		empty := bqtypes.CellsToRow(nil)
		want := bqtypes.Row{F: []bqtypes.Cell{}}
		if !reflect.DeepEqual(empty, want) {
			t.Fatalf("got %#v, want %#v", empty, want)
		}
	})
}
