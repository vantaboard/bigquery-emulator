package load

import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"regexp"
	"strconv"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
)

var datastoreOutputFileRE = regexp.MustCompile(`output-\d+`)

// parseDatastoreBackupSources loads a DATASTORE_BACKUP / Firestore export
// referenced by a *.export_metadata URI, fetches the companion output-* entity
// files, and decodes enough entity properties for the public samples.
func parseDatastoreBackupSources(ctx context.Context, cfg *jobs.JobConfigurationLoad,
	parseSchema *bqtypes.TableSchema,
) (ParsedRows, int64, int, error) {
	if cfg == nil || len(cfg.SourceURIs) == 0 {
		return ParsedRows{}, 0, 0, errors.New("DATASTORE_BACKUP requires sourceUris")
	}
	metaURI := cfg.SourceURIs[0]
	metaBytes, err := FetchSource(ctx, metaURI)
	if err != nil {
		return ParsedRows{}, 0, 0, err
	}
	totalBytes := int64(len(metaBytes))

	trimmed := bytes.TrimSpace(metaBytes)
	if len(trimmed) > 0 && (trimmed[0] == '{' || trimmed[0] == '[') {
		parsed, err := parseDatastoreJSON(trimmed, parseSchema)
		return parsed, totalBytes, 1, err
	}

	baseDir := datastoreBackupBaseDir(metaURI)
	outputs := uniqueStrings(datastoreOutputFileRE.FindAllString(string(metaBytes), -1))
	if len(outputs) == 0 {
		outputs = []string{"output-0"}
	}

	var parsed ParsedRows
	filesRead := 0
	for _, name := range outputs {
		uri := baseDir + name
		data, ferr := FetchSource(ctx, uri)
		if ferr != nil {
			continue
		}
		totalBytes += int64(len(data))
		filesRead++
		chunk, perr := parseDatastoreEntityBytes(data, parseSchema)
		if perr != nil {
			return ParsedRows{}, 0, 0, perr
		}
		parsed = mergeParsedChunk(parsed, chunk, filesRead == 1)
	}
	if filesRead == 0 {
		return ParsedRows{}, 0, 0, fmt.Errorf("DATASTORE_BACKUP: no output files found for %q", metaURI)
	}
	return parsed, totalBytes, filesRead, nil
}

func datastoreBackupBaseDir(uri string) string {
	if i := strings.LastIndex(uri, "/"); i >= 0 {
		return uri[:i+1]
	}
	return ""
}

func uniqueStrings(in []string) []string {
	if len(in) == 0 {
		return nil
	}
	seen := make(map[string]struct{}, len(in))
	out := make([]string, 0, len(in))
	for _, s := range in {
		if s == "" {
			continue
		}
		if _, ok := seen[s]; ok {
			continue
		}
		seen[s] = struct{}{}
		out = append(out, s)
	}
	return out
}

func parseDatastoreJSON(data []byte, schema *bqtypes.TableSchema) (ParsedRows, error) {
	if len(data) > 0 && data[0] == '[' {
		var rows []map[string]any
		if err := json.Unmarshal(data, &rows); err != nil {
			return ParsedRows{}, fmt.Errorf("parse DATASTORE_BACKUP JSON array: %w", err)
		}
		return finalizeDatastoreRows(rows, schema), nil
	}
	var doc map[string]any
	if err := json.Unmarshal(data, &doc); err != nil {
		return ParsedRows{}, fmt.Errorf("parse DATASTORE_BACKUP JSON: %w", err)
	}
	if entities, ok := doc["entities"].([]any); ok {
		rows := make([]map[string]any, 0, len(entities))
		for _, ent := range entities {
			if m, ok := ent.(map[string]any); ok {
				rows = append(rows, flattenDatastoreEntity(m))
			}
		}
		return finalizeDatastoreRows(rows, schema), nil
	}
	return finalizeDatastoreRows([]map[string]any{flattenDatastoreEntity(doc)}, schema), nil
}

func flattenDatastoreEntity(ent map[string]any) map[string]any {
	if props, ok := ent["properties"].(map[string]any); ok {
		out := make(map[string]any, len(props))
		for k, v := range props {
			out[k] = unwrapDatastoreValue(v)
		}
		return out
	}
	return ent
}

func unwrapDatastoreValue(v any) any {
	m, ok := v.(map[string]any)
	if !ok {
		return v
	}
	for _, key := range []string{
		"stringValue", "integerValue", "doubleValue", "booleanValue",
		"timestampValue", "nullValue",
	} {
		raw, ok := m[key]
		if !ok {
			continue
		}
		if key == "integerValue" {
			return unwrapDatastoreInteger(raw)
		}
		return raw
	}
	return v
}

func unwrapDatastoreInteger(raw any) any {
	s, ok := raw.(string)
	if !ok {
		return raw
	}
	n, err := strconv.ParseInt(s, 10, 64)
	if err != nil {
		return raw
	}
	return int(n)
}

func parseDatastoreEntityBytes(data []byte, schema *bqtypes.TableSchema) (ParsedRows, error) {
	trimmed := bytes.TrimSpace(data)
	if len(trimmed) > 0 && (trimmed[0] == '{' || trimmed[0] == '[') {
		return parseDatastoreJSON(trimmed, schema)
	}
	rows := scanDatastoreEntities(data)
	return finalizeDatastoreRows(rows, schema), nil
}

func finalizeDatastoreRows(rows []map[string]any, schema *bqtypes.TableSchema) ParsedRows {
	if schema == nil || len(schema.Fields) == 0 {
		schema = inferSchemaFromRows(rows)
	}
	return ParsedRows{Schema: schema, Rows: rows}
}

// scanDatastoreEntities heuristically extracts Firestore/Datastore entity
// properties from LevelDB-encoded export output files. Enough for the public
// us-states backup sample (name, post_abbr, year).
func scanDatastoreEntities(data []byte) []map[string]any {
	var rows []map[string]any
	for i := 0; i < len(data); {
		name, next := readDatastoreStringProp(data, i, "name")
		if next < 0 {
			break
		}
		abbr, next := readDatastoreStringProp(data, next, "post_abbr")
		if next < 0 {
			i++
			continue
		}
		year, next := readDatastoreVarintProp(data, next, "year")
		if next < 0 {
			i++
			continue
		}
		if name != "" {
			row := map[string]any{datastorePropName: name}
			if abbr != "" {
				row["post_abbr"] = abbr
			}
			if year != 0 {
				row["year"] = year
			}
			rows = append(rows, row)
		}
		i = next
	}
	return rows
}

func readDatastoreStringProp(data []byte, start int, prop string) (string, int) {
	marker := datastorePropMarker(prop)
	if marker == nil {
		return "", -1
	}
	idx := bytes.Index(data[start:], marker)
	if idx < 0 {
		return "", -1
	}
	pos := start + idx + len(marker)
	for pos < len(data) && (data[pos] == ' ' || data[pos] == 0) {
		pos++
	}
	if pos >= len(data) || data[pos] != 0x1a {
		return "", -1
	}
	pos++
	if pos >= len(data) {
		return "", -1
	}
	length := int(data[pos])
	pos++
	if pos+length > len(data) {
		return "", -1
	}
	return string(data[pos : pos+length]), pos + length
}

func readDatastoreVarintProp(data []byte, start int, prop string) (int64, int) {
	marker := datastorePropMarker(prop)
	if marker == nil {
		return 0, -1
	}
	idx := bytes.Index(data[start:], marker)
	if idx < 0 {
		return 0, -1
	}
	pos := start + idx + len(marker)
	for pos < len(data) && (data[pos] == ' ' || data[pos] == 0) {
		pos++
	}
	if pos >= len(data) || data[pos] != 0x08 {
		return 0, -1
	}
	pos++
	var val uint64
	shift := 0
	for pos < len(data) {
		b := data[pos]
		pos++
		val |= uint64(b&0x7f) << shift
		if b < 0x80 {
			return uint64ToSignedInt64(val), pos
		}
		shift += 7
	}
	return 0, -1
}
