package bqtypes

import (
	"strconv"
	"strings"
	"time"

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

// CellsToRowForSchema is like CellsToRow but re-encodes TIMESTAMP
// values as decimal microsecond strings. google-cloud-bigquery's
// query-result parser (`CELL_DATA_PARSER.timestamp_to_py`) expects
// microseconds since Unix epoch, not the human-readable strings the
// engine emits.
func CellsToRowForSchema(cells []*enginepb.Cell, schema *enginepb.TableSchema) Row {
	fields := []*enginepb.FieldSchema(nil)
	if schema != nil {
		fields = schema.GetFields()
	}
	out := Row{F: make([]Cell, 0, len(cells))}
	for i, c := range cells {
		var field *enginepb.FieldSchema
		if i < len(fields) {
			field = fields[i]
		}
		out.F = append(out.F, encodeCellForField(ValueToCell(c), field))
	}
	return out
}

func encodeCellForField(cell Cell, field *enginepb.FieldSchema) Cell {
	if cell.V == nil || field == nil {
		return cell
	}
	fieldType := field.GetType()
	if strings.HasPrefix(fieldType, "ARRAY<") {
		elements, ok := cell.V.([]Cell)
		if !ok {
			return cell
		}
		elemField := arrayElementFieldSchema(field)
		out := make([]Cell, len(elements))
		for i, el := range elements {
			out[i] = encodeCellForField(el, elemField)
		}
		return Cell{V: out}
	}
	switch fieldType {
	case "TIMESTAMP":
		s, ok := cell.V.(string)
		if !ok {
			return cell
		}
		if micros, err := TimestampStringToMicros(s); err == nil {
			return Cell{V: micros}
		}
		return cell
	case "STRUCT", "RECORD":
		row, ok := cell.V.(Row)
		if !ok {
			return cell
		}
		subFields := field.GetFields()
		out := make([]Cell, len(row.F))
		for i, subCell := range row.F {
			var subField *enginepb.FieldSchema
			if i < len(subFields) {
				subField = subFields[i]
			}
			out[i] = encodeCellForField(subCell, subField)
		}
		return Cell{V: Row{F: out}}
	default:
		return cell
	}
}

func arrayElementFieldSchema(field *enginepb.FieldSchema) *enginepb.FieldSchema {
	t := field.GetType()
	if !strings.HasPrefix(t, "ARRAY<") {
		return field
	}
	inner := strings.TrimSuffix(strings.TrimPrefix(t, "ARRAY<"), ">")
	return &enginepb.FieldSchema{Type: inner}
}

// TimestampStringToMicros parses an engine TIMESTAMP wire string and
// returns the BigQuery REST query-result encoding: decimal microseconds
// since 1970-01-01 UTC.
func TimestampStringToMicros(s string) (string, error) {
	t, err := parseTimestampWireString(s)
	if err != nil {
		return "", err
	}
	utc := t.UTC()
	micros := utc.Unix()*1_000_000 + int64(utc.Nanosecond()/1000)
	return strconv.FormatInt(micros, 10), nil
}

func parseTimestampWireString(s string) (time.Time, error) {
	s = strings.TrimSpace(s)
	s = strings.Replace(s, "T", " ", 1)
	s = strings.Replace(s, "+00:00", "+00", 1)
	s = strings.Replace(s, "Z", "+00", 1)
	layouts := []string{
		"2006-01-02 15:04:05.999999-07",
		"2006-01-02 15:04:05-07",
		"2006-01-02 15:04:05.999999",
		"2006-01-02 15:04:05",
	}
	var lastErr error
	for _, layout := range layouts {
		t, err := time.Parse(layout, s)
		if err == nil {
			return t, nil
		}
		lastErr = err
	}
	return time.Time{}, lastErr
}
