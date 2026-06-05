package external

import (
	"errors"
	"fmt"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/load"
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

func applyHivePartitions(
	rows []map[string]any,
	partitionValues map[string]string,
) {
	for _, row := range rows {
		for k, v := range partitionValues {
			row[k] = v
		}
	}
}

func mergeHiveSchema(
	dataSchema *bqtypes.TableSchema,
	partitionFields []hivePartitionField,
) *bqtypes.TableSchema {
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

func resolveHivePartitionFields(
	opts *bqtypes.HivePartitioningOptions,
) ([]hivePartitionField, error) {
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

func extractHivePartitions(
	objectURI string,
	opts *bqtypes.HivePartitioningOptions,
) (map[string]string, error) {
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
			// Trailing slash after a placeholder separates the partition
			// segment from the object name; it is not part of the fixed prefix.
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
	objPath, err := load.ObjectPathFromURI(objectURI)
	if err != nil {
		return nil, err
	}
	objBucket, err := load.BucketFromURI(objectURI)
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
			key, val := before, after
			if key != field.Name {
				return nil, fmt.Errorf("partition segment %q, want key %q", seg, field.Name)
			}
			out[field.Name] = val
		} else {
			out[field.Name] = seg
		}
	}
	return out, nil
}

func extractAutoPartitions(objectURI, sourceURIPrefix string) (map[string]string, error) {
	prefixPath, err := load.ObjectPathFromURI(sourceURIPrefix)
	if err != nil {
		return nil, err
	}
	if !strings.HasSuffix(prefixPath, "/") {
		prefixPath += "/"
	}
	objPath, err := load.ObjectPathFromURI(objectURI)
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
