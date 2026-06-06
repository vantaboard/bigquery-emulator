package load

import (
	"context"
	"errors"
	"fmt"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
)

const (
	hiveModeCustom                = "CUSTOM"
	hiveModeAuto                  = "AUTO"
	hiveModeStrings               = "STRINGS"
	defaultHivePartitionFieldType = "STRING"
)

type hivePartitionField struct {
	Name string
	Type string
}

func parseHiveURISources(ctx context.Context, cfg *jobs.JobConfigurationLoad,
	parseSchema *bqtypes.TableSchema,
) (ParsedRows, int64, int, error) {
	ext := &bqtypes.ExternalDataConfiguration{
		SourceURIs:              cfg.SourceURIs,
		SourceFormat:            cfg.SourceFormat,
		Schema:                  parseSchema,
		Autodetect:              cfg.Autodetect,
		HivePartitioningOptions: cfg.HivePartitioningOptions,
	}
	parsed, totalBytes, inputFiles, err := parseExternalGCS(ctx, ext, parseSchema, cfg.SkipLeadingRows())
	return parsed, totalBytes, inputFiles, err
}

// ParseExternalGCS fetches GCS objects (including wildcards) and applies hive
// partition columns when configured. Shared by load jobs and external tables.
func ParseExternalGCS(
	ctx context.Context,
	cfg *bqtypes.ExternalDataConfiguration,
	schema *bqtypes.TableSchema,
	skipLeading int,
) (ParsedRows, int64, int, error) {
	return parseExternalGCS(ctx, cfg, schema, skipLeading)
}

func parseExternalGCS(
	ctx context.Context,
	cfg *bqtypes.ExternalDataConfiguration,
	schema *bqtypes.TableSchema,
	skipLeading int,
) (ParsedRows, int64, int, error) {
	uris, err := ExpandSourceURIs(ctx, cfg.SourceURIs)
	if err != nil {
		return ParsedRows{}, 0, 0, err
	}

	var partitionFields []hivePartitionField
	if cfg.HivePartitioningOptions != nil {
		partitionFields, err = resolveHivePartitionFields(cfg.HivePartitioningOptions)
		if err != nil {
			return ParsedRows{}, 0, 0, err
		}
	}

	var parsed ParsedRows
	var totalBytes int64
	for i, uri := range uris {
		data, err := FetchSource(ctx, uri)
		if err != nil {
			return ParsedRows{}, 0, 0, err
		}
		totalBytes += int64(len(data))
		chunk, err := ParseSource(cfg.SourceFormat, data, schema, skipLeading, cfg.Autodetect)
		if err != nil {
			return ParsedRows{}, 0, 0, err
		}
		if cfg.HivePartitioningOptions != nil {
			partitionValues, partErr := extractHivePartitions(uri, cfg.HivePartitioningOptions)
			if partErr != nil {
				return ParsedRows{}, 0, 0, partErr
			}
			applyHivePartitions(chunk.Rows, partitionValues)
			if len(partitionFields) == 0 && len(partitionValues) > 0 {
				partitionFields = partitionFieldsFromValues(partitionValues, cfg.HivePartitioningOptions)
			}
		}
		parsed = mergeParsedChunk(parsed, chunk, i == 0)
	}
	if cfg.HivePartitioningOptions != nil && len(partitionFields) > 0 {
		parsed.Schema = mergeHiveSchema(parsed.Schema, partitionFields)
	}
	return parsed, totalBytes, len(uris), nil
}

func applyHivePartitions(rows []map[string]any, partitionValues map[string]string) {
	for _, row := range rows {
		for k, v := range partitionValues {
			row[k] = v
		}
	}
}

func mergeHiveSchema(dataSchema *bqtypes.TableSchema, partitionFields []hivePartitionField) *bqtypes.TableSchema {
	if len(partitionFields) == 0 {
		return dataSchema
	}
	existing := map[string]struct{}{}
	if dataSchema != nil {
		for _, f := range dataSchema.Fields {
			existing[f.Name] = struct{}{}
		}
	}
	out := &bqtypes.TableSchema{}
	if dataSchema != nil {
		out.Fields = append(out.Fields, dataSchema.Fields...)
	}
	for _, pf := range partitionFields {
		if _, ok := existing[pf.Name]; ok {
			continue
		}
		out.Fields = append(out.Fields, bqtypes.TableFieldSchema{
			Name: pf.Name,
			Type: pf.Type,
		})
	}
	return out
}

func resolveHivePartitionFields(opts *bqtypes.HivePartitioningOptions) ([]hivePartitionField, error) {
	if opts == nil {
		return nil, nil
	}
	mode := strings.ToUpper(strings.TrimSpace(opts.Mode))
	switch mode {
	case hiveModeCustom:
		_, _, fields, err := parseHiveCustomPrefix(opts.SourceURIPrefix)
		return fields, err
	case hiveModeAuto, hiveModeStrings:
		if strings.TrimSpace(opts.SourceURIPrefix) == "" {
			return nil, errors.New("hive AUTO/STRINGS mode requires sourceUriPrefix")
		}
		return nil, nil
	default:
		return nil, fmt.Errorf("unsupported hive partitioning mode %q", opts.Mode)
	}
}

func extractHivePartitions(objectURI string, opts *bqtypes.HivePartitioningOptions) (map[string]string, error) {
	if opts == nil {
		return nil, nil
	}
	mode := strings.ToUpper(strings.TrimSpace(opts.Mode))
	switch mode {
	case hiveModeCustom:
		return extractCustomPartitions(objectURI, opts.SourceURIPrefix)
	case hiveModeAuto, hiveModeStrings:
		return extractAutoPartitions(objectURI, opts.SourceURIPrefix)
	default:
		return nil, fmt.Errorf("unsupported hive partitioning mode %q", opts.Mode)
	}
}

func parseHiveCustomPrefix(template string) (bucket, pathPrefix string, fields []hivePartitionField, err error) {
	if !strings.HasPrefix(template, "gs://") {
		return "", "", nil, errors.New("sourceUriPrefix must be a gs:// URI")
	}
	rest := strings.TrimPrefix(template, "gs://")
	slash := strings.Index(rest, "/")
	if slash <= 0 {
		return "", "", nil, errors.New("invalid sourceUriPrefix")
	}
	bucket = rest[:slash]
	pathTemplate := rest[slash+1:]

	var prefix strings.Builder
	for i := 0; i < len(pathTemplate); {
		if pathTemplate[i] == '{' {
			close := strings.Index(pathTemplate[i:], "}")
			if close < 0 {
				return "", "", nil, errors.New("unclosed { in sourceUriPrefix")
			}
			inner := pathTemplate[i+1 : i+close]
			parts := strings.SplitN(inner, ":", 2)
			if len(parts) != 2 || parts[0] == "" || parts[1] == "" {
				return "", "", nil, fmt.Errorf("invalid hive field %q in sourceUriPrefix", inner)
			}
			fields = append(fields, hivePartitionField{Name: parts[0], Type: parts[1]})
			i += close + 1
			if i < len(pathTemplate) && pathTemplate[i] == '/' {
				i++
			}
			continue
		}
		prefix.WriteByte(pathTemplate[i])
		i++
	}
	return bucket, prefix.String(), fields, nil
}

func extractCustomPartitions(objectURI, sourceURIPrefix string) (map[string]string, error) {
	bucket, pathPrefix, fields, err := parseHiveCustomPrefix(sourceURIPrefix)
	if err != nil {
		return nil, err
	}
	if len(fields) == 0 {
		return nil, errors.New("CUSTOM hive mode requires partition fields in sourceUriPrefix")
	}
	objPath, err := ObjectPathFromURI(objectURI)
	if err != nil {
		return nil, err
	}
	objBucket, err := BucketFromURI(objectURI)
	if err != nil {
		return nil, err
	}
	if objBucket != bucket {
		return nil, fmt.Errorf("object bucket %q does not match sourceUriPrefix bucket %q", objBucket, bucket)
	}
	if !strings.HasPrefix(objPath, pathPrefix) {
		return nil, fmt.Errorf("object %q does not match hive prefix %q", objectURI, sourceURIPrefix)
	}
	remainder := strings.TrimPrefix(objPath, pathPrefix)
	segments := strings.Split(remainder, "/")
	if len(segments) < len(fields)+1 {
		return nil, fmt.Errorf("object %q has too few path segments for hive layout", objectURI)
	}
	partSegments := segments[:len(segments)-1]
	out := make(map[string]string, len(fields))
	for i, field := range fields {
		seg := partSegments[i]
		before, after, ok := strings.Cut(seg, "=")
		if ok {
			if before != field.Name {
				return nil, fmt.Errorf("partition segment %q, want key %q", seg, field.Name)
			}
			out[field.Name] = after
		} else {
			out[field.Name] = seg
		}
	}
	return out, nil
}

func extractAutoPartitions(objectURI, sourceURIPrefix string) (map[string]string, error) {
	prefixPath, err := ObjectPathFromURI(sourceURIPrefix)
	if err != nil {
		return nil, err
	}
	if !strings.HasSuffix(prefixPath, "/") {
		prefixPath += "/"
	}
	objPath, err := ObjectPathFromURI(objectURI)
	if err != nil {
		return nil, err
	}
	if !strings.HasPrefix(objPath, prefixPath) {
		return nil, fmt.Errorf("object %q does not match hive prefix %q", objectURI, sourceURIPrefix)
	}
	remainder := strings.TrimPrefix(objPath, prefixPath)
	segments := strings.Split(remainder, "/")
	if len(segments) < 2 {
		return nil, fmt.Errorf("object %q has no partition segments", objectURI)
	}
	partSegments := segments[:len(segments)-1]
	out := make(map[string]string, len(partSegments))
	for _, seg := range partSegments {
		before, after, ok := strings.Cut(seg, "=")
		if !ok {
			return nil, fmt.Errorf("partition segment %q is not key=value", seg)
		}
		out[before] = after
	}
	return out, nil
}

func partitionFieldsFromValues(
	values map[string]string,
	opts *bqtypes.HivePartitioningOptions,
) []hivePartitionField {
	if len(values) == 0 {
		return nil
	}
	fieldType := defaultHivePartitionFieldType
	if strings.EqualFold(strings.TrimSpace(opts.Mode), hiveModeStrings) {
		fieldType = defaultHivePartitionFieldType
	}
	order := opts.Fields
	if len(order) == 0 {
		order = make([]string, 0, len(values))
		for name := range values {
			order = append(order, name)
		}
	}
	out := make([]hivePartitionField, 0, len(order))
	for _, name := range order {
		if _, ok := values[name]; !ok {
			continue
		}
		out = append(out, hivePartitionField{Name: name, Type: fieldType})
	}
	return out
}
