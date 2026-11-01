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
	root, err := avroRecord(avroRecordName, schema.GetFields())
	if err != nil {
		return "", err
	}
	b, err := json.Marshal(root)
	if err != nil {
		return "", fmt.Errorf("marshal Avro schema: %w", err)
	}
	return string(b), nil
}

// avroRecord returns an Avro record with one field per schema field. Avro
// needs unique record names, so a nested record is named after its path.
func avroRecord(name string, fields []*enginepb.FieldSchema) (map[string]any, error) {
	avroFields := make([]map[string]any, 0, len(fields))
	for _, f := range fields {
		avroField, err := engineFieldToAvroField(f, name)
		if err != nil {
			return nil, err
		}
		avroFields = append(avroFields, avroField)
	}
	return map[string]any{
		avroKeyType: avroTypeRecord,
		avroKeyName: name,
		"fields":    avroFields,
	}, nil
}

func engineFieldToAvroField(f *enginepb.FieldSchema, parent string) (map[string]any, error) {
	if f == nil {
		return nil, errors.New("nil field schema")
	}
	path := nestedRecordName(parent, f)
	var typ any
	if element, ok := avroArrayElement(f); ok {
		items, err := avroValueType(element, path)
		if err != nil {
			return nil, err
		}
		typ = map[string]any{avroKeyType: avroTypeArray, "items": items}
	} else {
		valueType, err := avroValueType(f, path)
		if err != nil {
			return nil, err
		}
		typ = valueType
		if strings.ToUpper(f.GetMode()) != bqModeRequired {
			typ = []any{"null", typ}
		}
	}
	return map[string]any{
		avroKeyName: f.GetName(),
		avroKeyType: typ,
	}, nil
}

// avroArrayElement returns the schema of an array field's elements. The
// engine has two shapes for arrays: a REPEATED field, whose elements have
// the field's own type, and an ARRAY field, whose element schema is its
// only nested field.
func avroArrayElement(f *enginepb.FieldSchema) (*enginepb.FieldSchema, bool) {
	switch {
	case strings.ToUpper(f.GetMode()) == bqModeRepeated:
		return f, true
	case strings.ToUpper(strings.TrimSpace(f.GetType())) == bqTypeARRAY:
		if len(f.GetFields()) == 0 {
			return &enginepb.FieldSchema{Name: f.GetName(), Type: bqTypeSTRING}, true
		}
		return f.GetFields()[0], true
	default:
		return nil, false
	}
}

// avroValueType returns the Avro type of one non-null value of the field.
func avroValueType(f *enginepb.FieldSchema, path string) (any, error) {
	if isStruct(f) {
		return avroRecord(path, f.GetFields())
	}
	return bqTypeToAvroType(f.GetType()), nil
}

func nestedRecordName(parent string, f *enginepb.FieldSchema) string {
	return parent + "__" + f.GetName()
}

func isStruct(f *enginepb.FieldSchema) bool {
	switch strings.ToUpper(strings.TrimSpace(f.GetType())) {
	case bqTypeSTRUCT, bqTypeRECORD:
		return true
	default:
		return false
	}
}

func bqTypeToAvroType(t string) any {
	switch strings.ToUpper(strings.TrimSpace(t)) {
	case bqTypeBOOL:
		return "boolean"
	case bqTypeINT64, bqTypeINTEGER:
		return avroTypeLong
	case bqTypeFLOAT64, bqTypeFLOAT:
		return "double"
	case bqTypeBYTES:
		return avroTypeBytes
	case bqTypeSTRING:
		return avroTypeString
	case bqTypeDATE:
		return map[string]any{avroKeyType: "int", avroKeyLogicalType: "date"}
	case bqTypeDATETIME:
		return map[string]any{avroKeyType: avroTypeString, avroKeyLogicalType: "datetime"}
	case bqTypeTIMESTAMP:
		return map[string]any{avroKeyType: avroTypeLong, avroKeyLogicalType: "timestamp-micros"}
	case bqTypeTIME:
		return map[string]any{avroKeyType: avroTypeLong, avroKeyLogicalType: "time-micros"}
	case bqTypeNUMERIC:
		return map[string]any{
			avroKeyType:        avroTypeBytes,
			avroKeyLogicalType: "decimal",
			"precision":        38,
			"scale":            9,
		}
	case bqTypeBIGNUMERIC:
		return map[string]any{
			avroKeyType:        avroTypeBytes,
			avroKeyLogicalType: "decimal",
			"precision":        77,
			"scale":            38,
		}
	case bqTypeGEOGRAPHY:
		return map[string]any{avroKeyType: avroTypeString, "sqlType": bqTypeGEOGRAPHY}
	case bqTypeJSON:
		return map[string]any{avroKeyType: avroTypeString, "sqlType": bqTypeJSON}
	default:
		return avroTypeString
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
	return cellsToAvroRecord(schema.GetFields(), row.GetCells(), avroRecordName)
}

func cellsToAvroRecord(
	fields []*enginepb.FieldSchema,
	cells []*enginepb.Cell,
	path string,
) (map[string]any, error) {
	out := make(map[string]any, len(fields))
	for colIdx, field := range fields {
		var cell *enginepb.Cell
		if colIdx < len(cells) {
			cell = cells[colIdx]
		}
		val, err := cellToAvroNative(field, cell, nestedRecordName(path, field))
		if err != nil {
			return nil, fmt.Errorf("column %q: %w", field.GetName(), err)
		}
		out[field.GetName()] = val
	}
	return out, nil
}

func cellToAvroNative(field *enginepb.FieldSchema, cell *enginepb.Cell, path string) (any, error) {
	if element, ok := avroArrayElement(field); ok {
		elements := cell.GetArray().GetElements()
		out := make([]any, 0, len(elements))
		for i, e := range elements {
			if e == nil || e.GetNullValue() {
				return nil, fmt.Errorf("array element %d is null", i)
			}
			val, err := cellToAvroValue(element, e, path)
			if err != nil {
				return nil, fmt.Errorf("array element %d: %w", i, err)
			}
			out = append(out, val)
		}
		return out, nil
	}

	nullable := strings.ToUpper(field.GetMode()) != bqModeRequired
	nullCell := cell == nil || cell.GetNullValue()
	if nullCell {
		if nullable {
			return nil, nil
		}
		return nil, errors.New("required column is null")
	}

	val, err := cellToAvroValue(field, cell, path)
	if err != nil {
		return nil, err
	}
	if !nullable {
		return val, nil
	}
	if isStruct(field) {
		return map[string]any{path: val}, nil
	}
	return unionNative(field.GetType(), val), nil
}

func cellToAvroValue(field *enginepb.FieldSchema, cell *enginepb.Cell, path string) (any, error) {
	if isStruct(field) {
		return cellsToAvroRecord(field.GetFields(), cell.GetStructValue().GetFields(), path)
	}

	raw := strings.TrimSpace(cell.GetStringValue())
	typ := strings.ToUpper(strings.TrimSpace(field.GetType()))
	var val any
	var err error
	switch typ {
	case bqTypeBOOL:
		val, err = strconv.ParseBool(raw)
	case bqTypeINT64, bqTypeINTEGER:
		val, err = strconv.ParseInt(raw, 10, 64)
	case bqTypeFLOAT64, bqTypeFLOAT:
		val, err = strconv.ParseFloat(raw, 64)
	case bqTypeTIMESTAMP:
		micros, tsErr := timestampCellToMicros(raw)
		if tsErr != nil {
			err = tsErr
		} else {
			val = micros
		}
	case bqTypeDATE:
		val, err = dateStringToDays(raw)
	case bqTypeBYTES:
		val = []byte(raw)
	default:
		val = raw
	}
	if err != nil {
		return nil, err
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
	case bqTypeBOOL:
		return map[string]any{"boolean": val}
	case bqTypeINT64, bqTypeINTEGER, bqTypeTIMESTAMP:
		return map[string]any{avroTypeLong: val}
	case bqTypeFLOAT64, bqTypeFLOAT:
		return map[string]any{"double": val}
	case bqTypeBYTES, bqTypeNUMERIC, bqTypeBIGNUMERIC:
		return map[string]any{avroTypeBytes: val}
	case bqTypeDATE:
		return map[string]any{"int": val}
	default:
		return map[string]any{avroTypeString: val}
	}
}
