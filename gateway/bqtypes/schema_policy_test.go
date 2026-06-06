package bqtypes_test

import (
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func TestMergeSchemaPolicyTags(t *testing.T) {
	t.Parallel()
	base := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "Name", Type: "STRING"},
		{Name: "Age", Type: "INTEGER"},
	}}
	tag := "projects/p/locations/us/taxonomies/t/policyTags/pt"
	overlay := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "Age", PolicyTags: &bqtypes.PolicyTagList{Names: []string{tag}}},
	}}
	merged := bqtypes.MergeSchemaPolicyTags(base, overlay)
	if merged.Fields[1].PolicyTags == nil || merged.Fields[1].PolicyTags.Names[0] != tag {
		t.Fatalf("merged Age policyTags = %#v", merged.Fields[1].PolicyTags)
	}
	if merged.Fields[0].Type != "STRING" {
		t.Fatalf("Name type = %q", merged.Fields[0].Type)
	}
}

func TestExtractSchemaPolicyOverlay(t *testing.T) {
	t.Parallel()
	tag := "projects/p/locations/us/taxonomies/t/policyTags/pt"
	full := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "Age", Type: "INTEGER", PolicyTags: &bqtypes.PolicyTagList{Names: []string{tag}}},
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
