package handlers

import (
	"strconv"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

// schemaToProto converts a REST TableSchema into the gRPC TableSchema
// the engine accepts. Returns nil when the REST schema is nil so the
// proto's default zero-value gets sent on the wire.
func schemaToProto(s *bqtypes.TableSchema) *enginepb.TableSchema {
	if s == nil {
		return nil
	}
	out := &enginepb.TableSchema{Fields: make([]*enginepb.FieldSchema, 0, len(s.Fields))}
	for i := range s.Fields {
		out.Fields = append(out.Fields, fieldToProto(s.Fields[i]))
	}
	return out
}

// fieldToProto recursively converts a REST TableFieldSchema into the
// gRPC FieldSchema. Nested STRUCT/RECORD fields are walked verbatim.
func fieldToProto(f bqtypes.TableFieldSchema) *enginepb.FieldSchema {
	out := &enginepb.FieldSchema{
		Name:        f.Name,
		Type:        f.Type,
		Mode:        f.Mode,
		Description: f.Description,
	}
	for i := range f.Fields {
		out.Fields = append(out.Fields, fieldToProto(f.Fields[i]))
	}
	return out
}

// schemaFromProto is the inverse of schemaToProto: turns a gRPC
// TableSchema into the REST TableSchema. Returns nil for an absent or
// empty schema so the JSON response omits the field.
func schemaFromProto(s *enginepb.TableSchema) *bqtypes.TableSchema {
	if s == nil || len(s.Fields) == 0 {
		return nil
	}
	out := &bqtypes.TableSchema{Fields: make([]bqtypes.TableFieldSchema, 0, len(s.Fields))}
	for _, f := range s.Fields {
		out.Fields = append(out.Fields, fieldFromProto(f))
	}
	return out
}

func fieldFromProto(f *enginepb.FieldSchema) bqtypes.TableFieldSchema {
	fieldType := normalizeRESTFieldType(f.GetType())
	if strings.EqualFold(fieldType, "STRUCT") {
		fieldType = "RECORD"
	}
	out := bqtypes.TableFieldSchema{
		Name:        normalizeRESTFieldName(f.GetName()),
		Type:        fieldType,
		Mode:        f.GetMode(),
		Description: f.GetDescription(),
	}
	for _, sub := range f.GetFields() {
		out.Fields = append(out.Fields, fieldFromProto(sub))
	}
	return out
}

// normalizeRESTFieldName maps analyzer-synthesized column names ($col1, …)
// to the f0_, f1_, … aliases the Node client expects for anonymous SELECT
// outputs (queryParamsTimestamps sample reads row.f0_).
func normalizeRESTFieldName(name string) string {
	if len(name) >= 5 && strings.HasPrefix(name, "$col") {
		if n, err := strconv.Atoi(name[4:]); err == nil && n > 0 {
			return "f" + strconv.Itoa(n-1) + "_"
		}
	}
	return name
}

func normalizeRESTFieldType(t string) string {
	switch strings.ToUpper(strings.TrimSpace(t)) {
	case sqlTypeINT64:
		return sqlTypeINTEGER
	case "FLOAT64":
		return "FLOAT"
	case "BOOL":
		return "BOOLEAN"
	default:
		return t
	}
}

func normalizeRESTTableSchema(s *bqtypes.TableSchema) *bqtypes.TableSchema {
	if s == nil {
		return nil
	}
	out := *s
	out.Fields = make([]bqtypes.TableFieldSchema, len(s.Fields))
	for i, f := range s.Fields {
		out.Fields[i] = f
		out.Fields[i].Type = normalizeRESTFieldType(f.Type)
		if len(f.Fields) > 0 {
			nested := &bqtypes.TableSchema{Fields: f.Fields}
			if norm := normalizeRESTTableSchema(nested); norm != nil {
				out.Fields[i].Fields = norm.Fields
			}
		}
	}
	return &out
}
