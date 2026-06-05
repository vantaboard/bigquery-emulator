package load

import (
	"bytes"
	"encoding/csv"
	"encoding/json"
	"errors"
	"fmt"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

const (
	fieldTypeString   = "STRING"
	fieldTypeInteger  = "INTEGER"
	fieldTypeFloat    = "FLOAT"
	fieldTypeBoolean  = "BOOLEAN"
	fieldTypeRecord   = "RECORD"
	fieldModeRequired = "REQUIRED"
	fieldModeRepeated = "REPEATED"
	writeAppend       = "WRITE_APPEND"
)

// ParsedRows is the in-memory row batch produced by a format parser.
type ParsedRows struct {
	Schema *bqtypes.TableSchema
	Rows   []map[string]any
}

// ParseSource decodes load bytes according to sourceFormat.
func ParseSource(format string, data []byte, schema *bqtypes.TableSchema,
	skipLeading int, autodetect bool,
) (ParsedRows, error) {
	switch strings.ToUpper(strings.TrimSpace(format)) {
	case "", "CSV":
		return parseCSV(data, schema, skipLeading, autodetect)
	case "NEWLINE_DELIMITED_JSON":
		return parseNDJSON(data, schema, autodetect)
	case "PARQUET":
		return parseParquet(data, schema, autodetect)
	case "AVRO":
		return parseAvro(data, schema, autodetect)
	case "ORC":
		return parseORC(data, schema, autodetect)
	default:
		return ParsedRows{}, fmt.Errorf("unsupported sourceFormat %q", format)
	}
}

func parseCSV(data []byte, schema *bqtypes.TableSchema, skipLeading int, autodetect bool) (ParsedRows, error) {
	r := csv.NewReader(bytes.NewReader(data))
	r.TrimLeadingSpace = true
	all, err := r.ReadAll()
	if err != nil {
		return ParsedRows{}, fmt.Errorf("parse CSV: %w", err)
	}
	if len(all) <= skipLeading {
		return ParsedRows{Schema: schema, Rows: nil}, nil
	}
	dataRows := all[skipLeading:]
	if schema == nil || len(schema.Fields) == 0 {
		if !autodetect && len(dataRows) > 0 {
			return ParsedRows{}, errors.New("load job requires schema or autodetect=true for CSV")
		}
		if len(dataRows) == 0 {
			return ParsedRows{}, nil
		}
		width := len(dataRows[0])
		fields := make([]bqtypes.TableFieldSchema, width)
		for i := range fields {
			fields[i] = bqtypes.TableFieldSchema{
				Name: fmt.Sprintf("string_field_%d", i),
				Type: fieldTypeString,
			}
		}
		schema = &bqtypes.TableSchema{Fields: fields}
	}
	fields := schema.Fields
	out := make([]map[string]any, 0, len(dataRows))
	for _, rec := range dataRows {
		row := make(map[string]any, len(fields))
		for i, f := range fields {
			if i < len(rec) {
				row[f.Name] = rec[i]
			} else {
				row[f.Name] = nil
			}
		}
		out = append(out, row)
	}
	return ParsedRows{Schema: schema, Rows: out}, nil
}

func parseNDJSON(data []byte, schema *bqtypes.TableSchema, autodetect bool) (ParsedRows, error) {
	lines := bytes.Split(bytes.TrimSpace(data), []byte("\n"))
	out := make([]map[string]any, 0, len(lines))
	for _, line := range lines {
		if len(bytes.TrimSpace(line)) == 0 {
			continue
		}
		var row map[string]any
		if err := json.Unmarshal(line, &row); err != nil {
			return ParsedRows{}, fmt.Errorf("parse JSON line: %w", err)
		}
		out = append(out, row)
	}
	if schema == nil || len(schema.Fields) == 0 {
		if !autodetect {
			return ParsedRows{}, errors.New("load job requires schema or autodetect=true for JSON")
		}
		schema = inferSchemaFromRows(out)
	}
	return ParsedRows{Schema: schema, Rows: out}, nil
}

func inferSchemaFromRows(rows []map[string]any) *bqtypes.TableSchema {
	if len(rows) == 0 {
		return &bqtypes.TableSchema{}
	}
	seen := map[string]struct{}{}
	order := make([]string, 0)
	for _, row := range rows {
		for k := range row {
			if _, ok := seen[k]; !ok {
				seen[k] = struct{}{}
				order = append(order, k)
			}
		}
	}
	fields := make([]bqtypes.TableFieldSchema, 0, len(order))
	for _, name := range order {
		fields = append(fields, bqtypes.TableFieldSchema{Name: name, Type: fieldTypeString})
	}
	return &bqtypes.TableSchema{Fields: fields}
}
