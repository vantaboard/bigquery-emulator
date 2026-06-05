package load

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"

	goavro "github.com/linkedin/goavro/v2"
	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func parseAvro(data []byte, schema *bqtypes.TableSchema, autodetect bool) (ParsedRows, error) {
	ocf, err := goavro.NewOCFReader(bytes.NewReader(data))
	if err != nil {
		return ParsedRows{}, fmt.Errorf("parse Avro OCF: %w", err)
	}

	avroSchema := ocf.Codec().Schema()
	if schema == nil || len(schema.Fields) == 0 {
		if !autodetect {
			schema, err = avroJSONSchemaToBQ(avroSchema)
			if err != nil {
				return ParsedRows{}, fmt.Errorf("parse Avro schema: %w", err)
			}
		}
	}

	rows := make([]map[string]any, 0)
	for ocf.Scan() {
		rec, readErr := ocf.Read()
		if readErr != nil {
			return ParsedRows{}, fmt.Errorf("read Avro row: %w", readErr)
		}
		row, ok := rec.(map[string]any)
		if !ok {
			return ParsedRows{}, fmt.Errorf("read Avro row: unexpected type %T", rec)
		}
		rows = append(rows, normalizeAvroRow(row))
	}

	if schema == nil || len(schema.Fields) == 0 {
		schema, err = avroJSONSchemaToBQ(avroSchema)
		if err != nil {
			schema = inferSchemaFromRows(rows)
		}
	}
	return ParsedRows{Schema: schema, Rows: rows}, nil
}

func normalizeAvroRow(row map[string]any) map[string]any {
	if row == nil {
		return map[string]any{}
	}
	out := make(map[string]any, len(row))
	for k, v := range row {
		out[k] = avroValueToAny(v)
	}
	return out
}

func avroValueToAny(v any) any {
	switch val := v.(type) {
	case map[string]any:
		out := make(map[string]any, len(val))
		for k, sub := range val {
			out[k] = avroValueToAny(sub)
		}
		return out
	case []any:
		out := make([]any, len(val))
		for i, sub := range val {
			out[i] = avroValueToAny(sub)
		}
		return out
	case []byte:
		return string(val)
	default:
		return val
	}
}

func avroJSONSchemaToBQ(schemaJSON string) (*bqtypes.TableSchema, error) {
	var root any
	if err := json.Unmarshal([]byte(schemaJSON), &root); err != nil {
		return nil, fmt.Errorf("decode Avro schema JSON: %w", err)
	}
	fields, err := avroTypeToBQFields(root)
	if err != nil {
		return nil, err
	}
	return &bqtypes.TableSchema{Fields: fields}, nil
}

func avroTypeToBQFields(node any) ([]bqtypes.TableFieldSchema, error) {
	switch n := node.(type) {
	case map[string]any:
		if typ, ok := n["type"].(string); ok && typ == "record" {
			rawFields, ok := n["fields"].([]any)
			if !ok {
				return nil, errors.New("avro record missing fields")
			}
			out := make([]bqtypes.TableFieldSchema, 0, len(rawFields))
			for _, rf := range rawFields {
				fm, ok := rf.(map[string]any)
				if !ok {
					return nil, errors.New("avro field entry has unexpected shape")
				}
				name, _ := fm["name"].(string)
				if name == "" {
					return nil, errors.New("avro field missing name")
				}
				field, err := avroFieldTypeToBQ(name, fm["type"])
				if err != nil {
					return nil, err
				}
				out = append(out, field)
			}
			return out, nil
		}
		field, err := avroFieldTypeToBQ("", n)
		if err != nil {
			return nil, err
		}
		if field.Name == "" {
			return []bqtypes.TableFieldSchema{field}, nil
		}
		return []bqtypes.TableFieldSchema{field}, nil
	default:
		field, err := avroFieldTypeToBQ("", n)
		if err != nil {
			return nil, err
		}
		return []bqtypes.TableFieldSchema{field}, nil
	}
}

func avroFieldTypeToBQ(name string, node any) (bqtypes.TableFieldSchema, error) {
	typ, mode, nested, err := avroTypeNode(node)
	if err != nil {
		return bqtypes.TableFieldSchema{}, err
	}
	out := bqtypes.TableFieldSchema{Name: name, Type: typ, Mode: mode, Fields: nested}
	return out, nil
}

func avroTypeNode(node any) (typ, mode string, nested []bqtypes.TableFieldSchema, err error) {
	switch n := node.(type) {
	case string:
		return avroPrimitiveToBQ(n), "", nil, nil
	case []any:
		nullable := false
		var inner any
		for _, item := range n {
			if s, ok := item.(string); ok && s == "null" {
				nullable = true
				continue
			}
			if inner != nil {
				return "", "", nil, fmt.Errorf("unsupported Avro union: %v", n)
			}
			inner = item
		}
		if inner == nil {
			return "", "", nil, errors.New("avro union has no non-null member")
		}
		typ, mode, nested, err = avroTypeNode(inner)
		if err != nil {
			return "", "", nil, err
		}
		if nullable && mode == "" {
			mode = ""
		}
		return typ, mode, nested, nil
	case map[string]any:
		if t, ok := n["type"].(string); ok {
			switch t {
			case "array":
				elemTyp, _, elemNested, err := avroTypeNode(n["items"])
				if err != nil {
					return "", "", nil, err
				}
				return elemTyp, fieldModeRepeated, elemNested, nil
			case "record":
				fields, err := avroTypeToBQFields(n)
				if err != nil {
					return "", "", nil, err
				}
				return fieldTypeRecord, "", fields, nil
			default:
				return avroPrimitiveToBQ(t), "", nil, nil
			}
		}
		return "", "", nil, fmt.Errorf("unsupported Avro type map: %v", n)
	default:
		return "", "", nil, fmt.Errorf("unsupported Avro type node: %T", node)
	}
}

func avroPrimitiveToBQ(avroType string) string {
	switch avroType {
	case "boolean":
		return fieldTypeBoolean
	case "int", "long":
		return fieldTypeInteger
	case "float", "double":
		return fieldTypeFloat
	case "bytes", "fixed":
		return "BYTES"
	default:
		return fieldTypeString
	}
}
