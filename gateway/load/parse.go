package load

import (
	"bytes"
	"encoding/csv"
	"encoding/json"
	"errors"
	"fmt"
	"strconv"
	"strings"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

const (
	fieldTypeString   = "STRING"
	fieldTypeInteger  = "INTEGER"
	fieldTypeInt64    = "INT64"
	fieldTypeFloat    = "FLOAT"
	fieldTypeFloat64  = "FLOAT64"
	fieldTypeBoolean  = "BOOLEAN"
	fieldTypeBool     = "BOOL"
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
	case sourceFormatDatastoreBackup:
		return parseDatastoreEntityBytes(data, schema)
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
		if autodetect && skipLeading > 0 {
			header := all[skipLeading-1]
			schema = inferSchemaFromCSVHeader(header, dataRows)
		} else {
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
	}
	fields := schema.Fields
	out := make([]map[string]any, 0, len(dataRows))
	for _, rec := range dataRows {
		row := make(map[string]any, len(fields))
		for i, f := range fields {
			if i < len(rec) {
				row[f.Name] = coerceCSVCell(rec[i], f.Type)
			} else {
				row[f.Name] = nil
			}
		}
		out = append(out, row)
	}
	return ParsedRows{Schema: schema, Rows: out}, nil
}

func inferSchemaFromCSVHeader(header []string, dataRows [][]string) *bqtypes.TableSchema {
	fields := make([]bqtypes.TableFieldSchema, len(header))
	for i, name := range header {
		fields[i] = bqtypes.TableFieldSchema{
			Name: strings.TrimSpace(name),
			Type: inferCSVColumnType(columnValues(dataRows, i)),
		}
	}
	return &bqtypes.TableSchema{Fields: fields}
}

func columnValues(rows [][]string, col int) []string {
	out := make([]string, 0, len(rows))
	for _, row := range rows {
		if col < len(row) {
			out = append(out, strings.TrimSpace(row[col]))
		}
	}
	return out
}

func inferCSVColumnType(values []string) string {
	if len(values) == 0 {
		return fieldTypeString
	}
	allInt := true
	for _, v := range values {
		if v == "" {
			continue
		}
		if _, err := strconv.ParseInt(v, 10, 64); err != nil {
			allInt = false
			break
		}
	}
	if allInt {
		return fieldTypeInteger
	}
	return fieldTypeString
}

func coerceCSVCell(raw string, fieldType string) any {
	raw = strings.TrimSpace(raw)
	if raw == "" {
		return nil
	}
	switch strings.ToUpper(strings.TrimSpace(fieldType)) {
	case fieldTypeInteger, "INT64":
		if n, err := strconv.ParseInt(raw, 10, 64); err == nil {
			return int(n)
		}
	case fieldTypeFloat, "FLOAT64":
		if f, err := strconv.ParseFloat(raw, 64); err == nil {
			return f
		}
	case fieldTypeBoolean, "BOOL":
		switch strings.ToLower(raw) {
		case "true", "t", "1", "yes":
			return true
		case "false", "f", "0", "no":
			return false
		}
	case fieldTypeTimestamp:
		if ts, ok := parseCSVDateTime(raw, true); ok {
			return ts
		}
	case "DATETIME":
		if ts, ok := parseCSVDateTime(raw, false); ok {
			return ts
		}
	}
	return raw
}

// parseCSVDateTime parses RFC3339/RFC3339Nano timestamps from CSV cells.
// TIMESTAMP values keep timezone information; DATETIME values are normalized
// to a UTC wall-clock string without a zone suffix.
func parseCSVDateTime(raw string, keepZone bool) (string, bool) {
	layouts := []string{
		time.RFC3339Nano,
		time.RFC3339,
		"2006-01-02 15:04:05.999999 UTC",
		"2006-01-02 15:04:05 UTC",
		"2006-01-02 15:04:05.999999",
		"2006-01-02 15:04:05",
		"2006-01-02T15:04:05",
	}
	for _, layout := range layouts {
		if ts, err := time.Parse(layout, raw); err == nil {
			if keepZone {
				return ts.UTC().Format(time.RFC3339Nano), true
			}
			return ts.UTC().Format("2006-01-02T15:04:05.999999"), true
		}
	}
	return "", false
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
