package bqtypes_test

import (
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func TestMergeSchemaPolicyTags(t *testing.T) {
	t.Parallel()
	base := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "Name", Type: schemaTypeSTRING},
		{Name: fieldNameAge, Type: schemaTypeINTEGER},
	}}
	tag := "projects/p/locations/us/taxonomies/t/policyTags/pt"
	overlay := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: fieldNameAge, PolicyTags: &bqtypes.PolicyTagList{Names: []string{tag}}},
	}}
	merged := bqtypes.MergeSchemaPolicyTags(base, overlay)
	if merged.Fields[1].PolicyTags == nil || merged.Fields[1].PolicyTags.Names[0] != tag {
		t.Fatalf("merged Age policyTags = %#v", merged.Fields[1].PolicyTags)
	}
	if merged.Fields[0].Type != schemaTypeSTRING {
		t.Fatalf("Name type = %q", merged.Fields[0].Type)
	}
}

func TestExtractSchemaPolicyOverlay(t *testing.T) {
	t.Parallel()
	tag := "projects/p/locations/us/taxonomies/t/policyTags/pt"
	full := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: fieldNameAge, Type: schemaTypeINTEGER, PolicyTags: &bqtypes.PolicyTagList{Names: []string{tag}}},
		{Name: "Weight", Type: "FLOAT"},
	}}
	overlay := bqtypes.ExtractSchemaPolicyOverlay(full)
	if overlay == nil || len(overlay.Fields) != 1 {
		t.Fatalf("overlay = %#v", overlay)
	}
	if overlay.Fields[0].Type != "" {
		t.Fatalf("overlay field should omit engine type, got %q", overlay.Fields[0].Type)
	}
}

func TestExtractSchemaPolicyOverlayCollation(t *testing.T) {
	t.Parallel()
	full := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "name", Type: schemaTypeSTRING, Collation: collationUndCI},
		{Name: "nums", Type: schemaTypeINTEGER},
	}}
	overlay := bqtypes.ExtractSchemaPolicyOverlay(full)
	if overlay == nil || len(overlay.Fields) != 1 {
		t.Fatalf("overlay = %#v", overlay)
	}
	if overlay.Fields[0].Collation != collationUndCI {
		t.Fatalf("collation = %q, want %s", overlay.Fields[0].Collation, collationUndCI)
	}
	merged := bqtypes.MergeSchemaPolicyTags(full, overlay)
	if merged.Fields[0].Collation != collationUndCI {
		t.Fatalf("merged collation = %q", merged.Fields[0].Collation)
	}
}
