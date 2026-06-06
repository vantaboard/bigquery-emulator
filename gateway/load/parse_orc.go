package load

import (
	"bytes"
	"errors"
	"fmt"
	"strings"

	"github.com/scritchley/orc"
	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func parseORC(data []byte, schema *bqtypes.TableSchema, autodetect bool) (parsed ParsedRows, err error) {
	defer func() {
		if r := recover(); r != nil {
			err = fmt.Errorf("parse ORC: %v", r)
		}
	}()
	if len(data) < 3 || string(data[:3]) != "ORC" {
		return ParsedRows{}, errors.New("parse ORC: invalid ORC file header")
	}
	r, err := orc.NewReader(bytes.NewReader(data))
	if err != nil {
		return ParsedRows{}, fmt.Errorf("parse ORC: %w", err)
	}
	defer func() { _ = r.Close() }()

	orcSchema := r.Schema()
	cols := orcSchema.Columns()
	if len(cols) == 0 {
		return ParsedRows{}, errors.New("parse ORC: empty schema")
	}

	if schema == nil || len(schema.Fields) == 0 {
		if !autodetect {
			schema = orcSchemaToBQ(orcSchema)
		}
	}

	cur := r.Select(cols...)
	rows := make([]map[string]any, 0)
	for cur.Stripes() {
		for cur.Next() {
			vals := cur.Row()
			row := make(map[string]any, len(cols))
			for i, col := range cols {
				if i < len(vals) {
					row[col] = orcValueToAny(vals[i])
				} else {
					row[col] = nil
				}
			}
			rows = append(rows, row)
		}
	}

	if schema == nil || len(schema.Fields) == 0 {
		schema = orcSchemaToBQ(orcSchema)
		if schema == nil || len(schema.Fields) == 0 {
			schema = inferSchemaFromRows(rows)
		}
	}
	return ParsedRows{Schema: schema, Rows: rows}, nil
}

func orcSchemaToBQ(td *orc.TypeDescription) *bqtypes.TableSchema {
	if td == nil {
		return &bqtypes.TableSchema{}
	}
	fields := orcFieldsToBQ(td)
	return &bqtypes.TableSchema{Fields: fields}
}

func orcFieldsToBQ(td *orc.TypeDescription) []bqtypes.TableFieldSchema {
	out := make([]bqtypes.TableFieldSchema, 0, len(td.Columns()))
	for _, name := range td.Columns() {
		child, err := td.GetField(name)
		if err != nil {
			continue
		}
		out = append(out, orcFieldToBQ(name, child))
	}
	return out
}

func orcFieldToBQ(name string, td *orc.TypeDescription) bqtypes.TableFieldSchema {
	typ, mode, nested := orcTypeStringToBQ(td.String())
	return bqtypes.TableFieldSchema{Name: name, Type: typ, Mode: mode, Fields: nested}
}

func orcTypeStringToBQ(typeStr string) (typ, mode string, nested []bqtypes.TableFieldSchema) {
	typeStr = strings.TrimSpace(typeStr)
	if strings.HasPrefix(typeStr, "struct<") && strings.HasSuffix(typeStr, ">") {
		inner := strings.TrimSuffix(strings.TrimPrefix(typeStr, "struct<"), ">")
		return fieldTypeRecord, "", parseORCStructFields(inner)
	}
	if strings.HasPrefix(typeStr, "array<") && strings.HasSuffix(typeStr, ">") {
		inner := strings.TrimSuffix(strings.TrimPrefix(typeStr, "array<"), ">")
		elemTyp, _, _ := orcTypeStringToBQ(inner)
		return elemTyp, fieldModeRepeated, nil
	}
	switch typeStr {
	case "boolean":
		return fieldTypeBoolean, "", nil
	case "tinyint", "smallint", "int", "bigint":
		return fieldTypeInteger, "", nil
	case "float", "double":
		return fieldTypeFloat, "", nil
	case "string", "varchar", "char":
		return fieldTypeString, "", nil
	case "binary":
		return "BYTES", "", nil
	case "date", "timestamp":
		return fieldTypeTimestamp, "", nil
	case "decimal":
		return "NUMERIC", "", nil
	default:
		return fieldTypeString, "", nil
	}
}

func parseORCStructFields(inner string) []bqtypes.TableFieldSchema {
	parts := splitORCStructFields(inner)
	out := make([]bqtypes.TableFieldSchema, 0, len(parts))
	for _, part := range parts {
		colon := strings.Index(part, ":")
		if colon <= 0 {
			continue
		}
		name := strings.TrimSpace(part[:colon])
		typStr := strings.TrimSpace(part[colon+1:])
		typ, mode, nested := orcTypeStringToBQ(typStr)
		out = append(out, bqtypes.TableFieldSchema{Name: name, Type: typ, Mode: mode, Fields: nested})
	}
	return out
}

func splitORCStructFields(inner string) []string {
	var parts []string
	depth := 0
	start := 0
	for i, ch := range inner {
		switch ch {
		case '<':
			depth++
		case '>':
			depth--
		case ',':
			if depth == 0 {
				parts = append(parts, inner[start:i])
				start = i + 1
			}
		}
	}
	if start < len(inner) {
		parts = append(parts, inner[start:])
	}
	return parts
}

func orcValueToAny(v any) any {
	switch val := v.(type) {
	case map[string]any:
		out := make(map[string]any, len(val))
		for k, sub := range val {
			out[k] = orcValueToAny(sub)
		}
		return out
	case []any:
		out := make([]any, len(val))
		for i, sub := range val {
			out[i] = orcValueToAny(sub)
		}
		return out
	case []byte:
		return string(val)
	default:
		return val
	}
}
