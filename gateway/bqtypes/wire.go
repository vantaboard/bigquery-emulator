package bqtypes

import (
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

// ValueToCell converts a `bigquery_emulator.v1.Cell` from the engine
// gRPC contract into the BigQuery REST `f`/`v` wire shape that
// `jobs.query`, `jobs.getQueryResults`, and `tabledata.list` emit.
//
// The C++ engine is responsible for serializing each `googlesql::Value`
// into the per-TypeKind string form documented at
// docs/bigquery/docs/reference/rest/v2/StandardSqlDataType.md (mirrored
// in docs/REST_API.md "Type wire encoding"). At the wire level
// everything is strings/objects/arrays, so this function is mostly a
// structural rewrap:
//
//   - `INT64`            -> decimal string, e.g. "42"
//   - `BOOL`             -> "true" or "false"
//   - `FLOAT64`          -> decimal string, or "NaN" / "Infinity" / "-Infinity"
//   - `STRING`           -> raw string
//   - `BYTES`            -> base64 string (RFC 4648 section 4)
//   - `DATE`             -> "YYYY-MM-DD"
//   - `TIMESTAMP`        -> RFC 3339 with mandatory `Z`, microsecond precision
//   - `DATETIME`         -> "YYYY-MM-DD HH:MM:SS.ffffff"
//   - `TIME`             -> "HH:MM:SS.ffffff"
//   - `NUMERIC`          -> decimal string
//   - `BIGNUMERIC`       -> decimal string
//   - `GEOGRAPHY`        -> WKT string
//   - `JSON`             -> string-encoded JSON
//   - `ARRAY`            -> Cell whose `v` is a list of {"v": ...} entries
//   - `STRUCT`           -> Cell whose `v` is a Row-shaped {"f": [{"v": ...}, ...]}
//   - NULL               -> Cell whose `v` is nil (JSON null)
//
// STRUCT is rendered as a nested `Row` (positional `f`) rather than a
// JSON object so it round-trips through `tabledata.list`, which
// disallows duplicate field names. ARRAY elements are themselves
// `Cell`s so nested ARRAY-of-STRUCT, ARRAY-of-ARRAY, and NULL elements
// all marshal consistently.
//
// A nil input cell is treated as NULL.
func ValueToCell(c *enginepb.Cell) Cell {
	if c == nil {
		return Cell{V: nil}
	}
	switch v := c.GetValue().(type) {
	case *enginepb.Cell_StringValue:
		return Cell{V: v.StringValue}
	case *enginepb.Cell_NullValue:
		return Cell{V: nil}
	case *enginepb.Cell_Array:
		elements := v.Array.GetElements()
		out := make([]Cell, 0, len(elements))
		for _, el := range elements {
			out = append(out, ValueToCell(el))
		}
		return Cell{V: out}
	case *enginepb.Cell_StructValue:
		fields := v.StructValue.GetFields()
		out := make([]Cell, 0, len(fields))
		for _, f := range fields {
			out = append(out, ValueToCell(f))
		}
		return Cell{V: Row{F: out}}
	default:
		return Cell{V: nil}
	}
}

// CellsToRow lowers a flat slice of engine cells into the top-level
// `f`/`v` Row shape BigQuery REST clients expect. Top-level rows are
// always Row-shaped; a STRUCT column nested inside the row becomes a
// Cell whose `v` is itself a Row (handled by ValueToCell).
func CellsToRow(cells []*enginepb.Cell) Row {
	out := Row{F: make([]Cell, 0, len(cells))}
	for _, c := range cells {
		out.F = append(out.F, ValueToCell(c))
	}
	return out
}
