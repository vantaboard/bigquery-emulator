package bqstorage

import (
	"encoding/json"
	"errors"
	"fmt"
	"strconv"
	"strings"
	"time"

	"cloud.google.com/go/bigquery/storage/apiv1/storagepb"
	goavro "github.com/linkedin/goavro/v2"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

const avroRecordName = "root"

func serializeAvroSchema(schema *enginepb.TableSchema) (*storagepb.AvroSchema, error) {
	schemaJSON, err := avroSchemaJSONFromEngine(schema)
	if err != nil {
		return nil, err
	}
	return &storagepb.AvroSchema{Schema: schemaJSON}, nil
}

func avroSchemaJSONFromEngine(schema *enginepb.TableSchema) (string, error) {
	fields := make([]map[string]any, 0, len(schema.GetFields()))
	for _, f := range schema.GetFields() {
		avroField, err := engineFieldToAvroField(f)
		if err != nil {
			return "", err
		}
		fields = append(fields, avroField)
	}
	root := map[string]any{
		"type":   "record",
		"name":   avroRecordName,
		"fields": fields,
	}
	b, err := json.Marshal(root)
	if err != nil {
		return "", fmt.Errorf("marshal Avro schema: %w", err)
	}
	return string(b), nil
}

func engineFieldToAvroField(f *enginepb.FieldSchema) (map[string]any, error) {
	if f == nil {
		return nil, errors.New("nil field schema")
	}
	typ := bqTypeToAvroType(f.GetType())
	if strings.ToUpper(f.GetMode()) != "REQUIRED" {
		typ = []any{"null", typ}
	}
	return map[string]any{
		"name": f.GetName(),
		"type": typ,
	}, nil
}

func bqTypeToAvroType(t string) any {
	switch strings.ToUpper(strings.TrimSpace(t)) {
	case "BOOL":
		return "boolean"
	case "INT64", "INTEGER":
		return "long"
	case "FLOAT64", "FLOAT":
		return "double"
	case "BYTES":
		return "bytes"
	case "STRING":
		return "string"
	case "DATE":
		return map[string]any{"type": "int", "logicalType": "date"}
	case "DATETIME":
		return map[string]any{"type": "string", "logicalType": "datetime"}
	case "TIMESTAMP":
		return map[string]any{"type": "long", "logicalType": "timestamp-micros"}
	case "TIME":
		return map[string]any{"type": "long", "logicalType": "time-micros"}
	case "NUMERIC":
		return map[string]any{
			"type":        "bytes",
			"logicalType": "decimal",
			"precision":   38,
			"scale":       9,
		}
	case "BIGNUMERIC":
		return map[string]any{
			"type":        "bytes",
			"logicalType": "decimal",
			"precision":   77,
			"scale":       38,
		}
	case "GEOGRAPHY":
		return map[string]any{"type": "string", "sqlType": "GEOGRAPHY"}
	case "JSON":
		return map[string]any{"type": "string", "sqlType": "JSON"}
	case "STRUCT", "RECORD":
		// Nested structs are lowered to string cells in the engine shim today.
		return "string"
	default:
		return "string"
	}
}

func rowsToAvroBatch(
	schema *enginepb.TableSchema,
	rows []*enginepb.DataRow,
) (*storagepb.AvroRows, error) {
	schemaJSON, err := avroSchemaJSONFromEngine(schema)
	if err != nil {
		return nil, err
	}
	codec, err := goavro.NewCodec(schemaJSON)
	if err != nil {
		return nil, fmt.Errorf("create Avro codec: %w", err)
	}

	var binary []byte
	for _, row := range rows {
		native, convErr := engineRowToAvroNative(schema, row)
		if convErr != nil {
			return nil, convErr
		}
		buf, encErr := codec.BinaryFromNative(nil, native)
		if encErr != nil {
			return nil, fmt.Errorf("encode Avro row: %w", encErr)
		}
		binary = append(binary, buf...)
	}
	return &storagepb.AvroRows{
		SerializedBinaryRows: binary,
		RowCount:             int64(len(rows)),
	}, nil
}

func engineRowToAvroNative(
	schema *enginepb.TableSchema,
	row *enginepb.DataRow,
) (map[string]any, error) {
	out := make(map[string]any, len(schema.GetFields()))
	for colIdx, field := range schema.GetFields() {
		var cell *enginepb.Cell
		if colIdx < len(row.GetCells()) {
			cell = row.GetCells()[colIdx]
		}
		val, err := cellToAvroNative(field, cell)
		if err != nil {
			return nil, fmt.Errorf("column %q: %w", field.GetName(), err)
		}
		out[field.GetName()] = val
	}
	return out, nil
}

func cellToAvroNative(field *enginepb.FieldSchema, cell *enginepb.Cell) (any, error) {
	nullable := strings.ToUpper(field.GetMode()) != "REQUIRED"
	nullCell := cell == nil || cell.GetNullValue()
	if nullCell {
		if nullable {
			return nil, nil
		}
		return nil, errors.New("required column is null")
	}

	raw := strings.TrimSpace(cell.GetStringValue())
	typ := strings.ToUpper(strings.TrimSpace(field.GetType()))
	var val any
	var err error
	switch typ {
	case "BOOL":
		val, err = strconv.ParseBool(raw)
	case "INT64", "INTEGER":
		val, err = strconv.ParseInt(raw, 10, 64)
	case "FLOAT64", "FLOAT":
		val, err = strconv.ParseFloat(raw, 64)
	case "TIMESTAMP":
		micros, tsErr := timestampCellToMicros(raw)
		if tsErr != nil {
			err = tsErr
		} else {
			val = micros
		}
	case "DATE":
		val, err = dateStringToDays(raw)
	case "BYTES":
		val = []byte(raw)
	default:
		val = raw
	}
	if err != nil {
		return nil, err
	}
	if nullable {
		return unionNative(typ, val), nil
	}
	return val, nil
}

func dateStringToDays(s string) (int32, error) {
	t, err := time.Parse("2006-01-02", strings.TrimSpace(s))
	if err != nil {
		return 0, err
	}
	epoch := time.Date(1970, 1, 1, 0, 0, 0, 0, time.UTC)
	return int32(t.Sub(epoch).Hours() / 24), nil
}

func unionNative(bqType string, val any) map[string]any {
	switch strings.ToUpper(strings.TrimSpace(bqType)) {
	case "BOOL":
		return map[string]any{"boolean": val}
	case "INT64", "INTEGER", "TIMESTAMP":
		return map[string]any{"long": val}
	case "FLOAT64", "FLOAT":
		return map[string]any{"double": val}
	case "BYTES", "NUMERIC", "BIGNUMERIC":
		return map[string]any{"bytes": val}
	case "DATE":
		return map[string]any{"int": val}
	default:
		return map[string]any{"string": val}
	}
}
