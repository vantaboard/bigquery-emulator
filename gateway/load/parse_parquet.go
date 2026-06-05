package load

import (
	"bytes"
	"errors"
	"fmt"
	"io"

	"github.com/parquet-go/parquet-go"
	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func parseParquet(data []byte, schema *bqtypes.TableSchema, autodetect bool) (ParsedRows, error) {
	f, err := parquet.OpenFile(bytes.NewReader(data), int64(len(data)))
	if err != nil {
		return ParsedRows{}, fmt.Errorf("parse Parquet: %w", err)
	}

	if schema == nil || len(schema.Fields) == 0 {
		if !autodetect {
			schema = parquetFileSchemaToBQ(f.Schema())
		}
	}

	reader := parquet.NewReader(f)
	defer func() { _ = reader.Close() }()

	rows := make([]map[string]any, 0, f.NumRows())
	for {
		row := make(map[string]any)
		err := reader.Read(&row)
		if err != nil {
			if errors.Is(err, io.EOF) {
				break
			}
			return ParsedRows{}, fmt.Errorf("read Parquet rows: %w", err)
		}
		rows = append(rows, normalizeParquetRow(row))
	}

	if schema == nil || len(schema.Fields) == 0 {
		schema = inferSchemaFromRows(rows)
		if schema == nil {
			schema = parquetFileSchemaToBQ(f.Schema())
		}
	}
	return ParsedRows{Schema: schema, Rows: rows}, nil
}

func normalizeParquetRow(row map[string]any) map[string]any {
	if row == nil {
		return map[string]any{}
	}
	out := make(map[string]any, len(row))
	for k, v := range row {
		out[k] = parquetValueToAny(v)
	}
	return out
}

func parquetValueToAny(v any) any {
	switch val := v.(type) {
	case map[string]any:
		out := make(map[string]any, len(val))
		for k, sub := range val {
			out[k] = parquetValueToAny(sub)
		}
		return out
	case []any:
		out := make([]any, len(val))
		for i, sub := range val {
			out[i] = parquetValueToAny(sub)
		}
		return out
	default:
		return val
	}
}

func parquetFileSchemaToBQ(s *parquet.Schema) *bqtypes.TableSchema {
	if s == nil {
		return &bqtypes.TableSchema{}
	}
	fields := s.Fields()
	out := make([]bqtypes.TableFieldSchema, 0, len(fields))
	for _, f := range fields {
		out = append(out, parquetFieldToBQ(f))
	}
	return &bqtypes.TableSchema{Fields: out}
}

func parquetFieldToBQ(f parquet.Field) bqtypes.TableFieldSchema {
	name := f.Name()
	typ := parquetNodeTypeToBQ(f)
	mode := ""
	if f.Required() {
		mode = fieldModeRequired
	}
	if f.Repeated() {
		mode = "REPEATED"
	}
	nested := f.Fields()
	if len(nested) > 0 && typ == fieldTypeRecord {
		sub := make([]bqtypes.TableFieldSchema, 0, len(nested))
		for _, nf := range nested {
			sub = append(sub, parquetFieldToBQ(nf))
		}
		return bqtypes.TableFieldSchema{Name: name, Type: typ, Mode: mode, Fields: sub}
	}
	return bqtypes.TableFieldSchema{Name: name, Type: typ, Mode: mode}
}

func parquetNodeTypeToBQ(f parquet.Field) string {
	if len(f.Fields()) > 0 {
		return fieldTypeRecord
	}
	switch f.Type().String() {
	case fieldTypeBoolean:
		return fieldTypeBoolean
	case "INT32", "INT64", "UINT32", "UINT64", "INT96":
		return fieldTypeInteger
	case "FLOAT", "DOUBLE":
		return fieldTypeFloat
	case "BYTE_ARRAY", "FIXED_LEN_BYTE_ARRAY":
		return fieldTypeString
	default:
		return fieldTypeString
	}
}
