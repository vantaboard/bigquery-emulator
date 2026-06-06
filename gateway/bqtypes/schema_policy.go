package bqtypes

// PolicyTagList is the policyTags sub-object on TableFieldSchema.
type PolicyTagList struct {
	Names []string `json:"names,omitempty"`
}

// ExtractSchemaPolicyOverlay copies only policyTags-bearing fields from
// a REST schema so the gateway metadata store can round-trip column ACLs
// without shadowing engine-owned column types.
func ExtractSchemaPolicyOverlay(s *TableSchema) *TableSchema {
	if s == nil || len(s.Fields) == 0 {
		return nil
	}
	fields := extractPolicyFields(s.Fields)
	if len(fields) == 0 {
		return nil
	}
	return &TableSchema{Fields: fields}
}

func extractPolicyFields(fields []TableFieldSchema) []TableFieldSchema {
	out := make([]TableFieldSchema, 0, len(fields))
	for _, f := range fields {
		nested := extractPolicyFields(f.Fields)
		if f.PolicyTags != nil && len(f.PolicyTags.Names) > 0 ||
			f.Collation != "" || len(nested) > 0 {
			out = append(out, TableFieldSchema{
				Name:       f.Name,
				Collation:  f.Collation,
				PolicyTags: f.PolicyTags,
				Fields:     nested,
			})
			continue
		}
	}
	return out
}

// MergeSchemaPolicyTags overlays cached policyTags onto the engine schema
// returned by tables.get.
func MergeSchemaPolicyTags(base, overlay *TableSchema) *TableSchema {
	if base == nil {
		return overlay
	}
	if overlay == nil || len(overlay.Fields) == 0 {
		return base
	}
	merged := *base
	merged.Fields = mergeFieldPolicyTags(base.Fields, overlay.Fields)
	return &merged
}

func mergeFieldPolicyTags(base, overlay []TableFieldSchema) []TableFieldSchema {
	if len(base) == 0 {
		return overlay
	}
	byName := map[string]TableFieldSchema{}
	for _, f := range overlay {
		byName[f.Name] = f
	}
	out := append([]TableFieldSchema(nil), base...)
	for i, f := range base {
		out[i] = f
		ov, ok := byName[f.Name]
		if !ok {
			continue
		}
		if ov.PolicyTags != nil {
			out[i].PolicyTags = ov.PolicyTags
		}
		if ov.Collation != "" {
			out[i].Collation = ov.Collation
		}
		if len(f.Fields) > 0 || len(ov.Fields) > 0 {
			out[i].Fields = mergeFieldPolicyTags(f.Fields, ov.Fields)
		}
	}
	for _, ov := range overlay {
		if _, ok := indexFieldByName(base, ov.Name); ok {
			continue
		}
		out = append(out, ov)
	}
	return out
}

func indexFieldByName(fields []TableFieldSchema, name string) (TableFieldSchema, bool) {
	for _, f := range fields {
		if f.Name == name {
			return f, true
		}
	}
	return TableFieldSchema{}, false
}
