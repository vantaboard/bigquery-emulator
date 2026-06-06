package bqtypes

// ApplyDefaultCollationToStringFields stamps `collation` onto STRING (and
// STRING-like) top-level schema fields when the table carries a
// `defaultCollation` and the field does not already specify one.
func ApplyDefaultCollationToStringFields(schema *TableSchema, defaultCollation string) *TableSchema {
	if schema == nil || defaultCollation == "" || len(schema.Fields) == 0 {
		return schema
	}
	out := *schema
	out.Fields = make([]TableFieldSchema, len(schema.Fields))
	for i, f := range schema.Fields {
		out.Fields[i] = applyDefaultCollationField(f, defaultCollation)
	}
	return &out
}

func applyDefaultCollationField(f TableFieldSchema, defaultCollation string) TableFieldSchema {
	out := f
	if f.Collation == "" && isStringLikeFieldType(f.Type) {
		out.Collation = defaultCollation
	}
	if len(f.Fields) > 0 {
		nested := make([]TableFieldSchema, len(f.Fields))
		for i, sub := range f.Fields {
			nested[i] = applyDefaultCollationField(sub, defaultCollation)
		}
		out.Fields = nested
	}
	return out
}

func isStringLikeFieldType(t string) bool {
	switch t {
	case "STRING", "JSON", "GEOGRAPHY":
		return true
	default:
		return false
	}
}
