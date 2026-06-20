package load

import (
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func TestMergeSchemasForTablePatchRelaxesRequired(t *testing.T) {
	t.Parallel()
	existing := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "id", Type: fieldTypeInteger, Mode: fieldModeRequired},
	}}
	patch := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "id", Type: fieldTypeInteger, Mode: ""},
	}}
	merged, changed, err := MergeSchemasForTablePatch(existing, patch)
	if err != nil {
		t.Fatalf("MergeSchemasForTablePatch: %v", err)
	}
	if !changed {
		t.Fatal("expected schema change")
	}
	if merged.Fields[0].Mode != "" {
		t.Fatalf("mode = %q, want NULLABLE (empty)", merged.Fields[0].Mode)
	}
}

func TestMergeSchemasForTablePatchUpdatesDescription(t *testing.T) {
	t.Parallel()
	existing := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "id", Type: fieldTypeInteger, Description: "old"},
	}}
	patch := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "id", Type: fieldTypeInteger, Description: "new"},
	}}
	merged, changed, err := MergeSchemasForTablePatch(existing, patch)
	if err != nil {
		t.Fatalf("MergeSchemasForTablePatch: %v", err)
	}
	if !changed {
		t.Fatal("expected schema change")
	}
	if merged.Fields[0].Description != "new" {
		t.Fatalf("description = %q, want new", merged.Fields[0].Description)
	}
}

func TestMergeSchemasForTablePatchAddsField(t *testing.T) {
	t.Parallel()
	existing := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "id", Type: fieldTypeInteger},
	}}
	patch := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "id", Type: fieldTypeInteger},
		{Name: "extra", Type: fieldTypeString},
	}}
	merged, changed, err := MergeSchemasForTablePatch(existing, patch)
	if err != nil {
		t.Fatalf("MergeSchemasForTablePatch: %v", err)
	}
	if !changed || len(merged.Fields) != 2 {
		t.Fatalf("merged = %#v, changed=%v", merged, changed)
	}
}

func TestMergeSchemasForTablePatchRejectsNarrowing(t *testing.T) {
	t.Parallel()
	existing := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "id", Type: fieldTypeInteger, Mode: ""},
	}}
	patch := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "id", Type: fieldTypeInteger, Mode: fieldModeRequired},
	}}
	_, _, err := MergeSchemasForTablePatch(existing, patch)
	if err == nil {
		t.Fatal("expected error for NULLABLE→REQUIRED")
	}
}

func TestMergeSchemasForTablePatchRejectsTypeChange(t *testing.T) {
	t.Parallel()
	existing := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "id", Type: fieldTypeInteger},
	}}
	patch := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "id", Type: fieldTypeString},
	}}
	_, _, err := MergeSchemasForTablePatch(existing, patch)
	if err == nil {
		t.Fatal("expected error for type change")
	}
}
