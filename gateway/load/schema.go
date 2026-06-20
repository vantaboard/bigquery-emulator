package load

import (
	"context"
	"fmt"
	"slices"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"github.com/vantaboard/bigquery-emulator/gateway/seed"
)

const (
	schemaUpdateAllowFieldAddition   = "ALLOW_FIELD_ADDITION"
	schemaUpdateAllowFieldRelaxation = "ALLOW_FIELD_RELAXATION"
)

// TablePatchSchemaOptions are the schemaUpdateOptions honored by
// tables.patch when syncing schema changes to the engine catalog.
var TablePatchSchemaOptions = []string{
	schemaUpdateAllowFieldAddition,
	schemaUpdateAllowFieldRelaxation,
}

// existingDestinationSchema returns the catalog schema for a destination
// table when the load job omits an explicit schema and autodetect=false.
func existingDestinationSchema(ctx context.Context, catalog enginepb.CatalogClient,
	projectID, datasetID, tableID string,
) *bqtypes.TableSchema {
	tableRef := &enginepb.TableRef{
		ProjectId: projectID,
		DatasetId: datasetID,
		TableId:   tableID,
	}
	if !tableExists(ctx, catalog, tableRef) {
		return nil
	}
	desc, err := catalog.DescribeTable(ctx, &enginepb.DescribeTableRequest{Table: tableRef})
	if err != nil {
		return nil
	}
	return schemaFromProto(desc.GetSchema())
}

// resolveDestinationSchema merges load-time schema updates into the destination
// table schema for WRITE_APPEND jobs. When the merged schema differs from the
// engine catalog, existing rows are preserved via drop-and-recreate.
func resolveDestinationSchema(ctx context.Context, catalog enginepb.CatalogClient,
	cfg *jobs.JobConfigurationLoad, projectID, datasetID, tableID string,
	loadSchema *bqtypes.TableSchema,
) (*enginepb.TableSchema, error) {
	wd := cfg.WriteDisposition
	if wd == "" {
		wd = writeAppend
	}
	if wd != writeAppend || len(cfg.SchemaUpdateOptions) == 0 {
		return SchemaToProto(loadSchema), nil
	}

	tableRef := &enginepb.TableRef{
		ProjectId: projectID,
		DatasetId: datasetID,
		TableId:   tableID,
	}
	if !tableExists(ctx, catalog, tableRef) {
		return SchemaToProto(loadSchema), nil
	}

	desc, err := catalog.DescribeTable(ctx, &enginepb.DescribeTableRequest{Table: tableRef})
	if err != nil {
		return nil, fmt.Errorf("describe destination table: %w", err)
	}
	existing := schemaFromProto(desc.GetSchema())
	explicit := cfg.Schema
	if explicit == nil || len(explicit.Fields) == 0 {
		explicit = loadSchema
	}
	merged, changed, err := mergeSchemas(existing, explicit, cfg.SchemaUpdateOptions, false)
	if err != nil {
		return nil, err
	}
	if !changed {
		return SchemaToProto(existing), nil
	}

	preserved, err := listAllRows(ctx, catalog, tableRef, desc.GetSchema())
	if err != nil {
		return nil, err
	}
	protoMerged := SchemaToProto(merged)
	if _, err := catalog.DropTable(ctx, &enginepb.DropTableRequest{Table: tableRef}); err != nil {
		return nil, fmt.Errorf("schema update drop table: %w", err)
	}
	if _, err := catalog.RegisterTable(ctx, &enginepb.RegisterTableRequest{
		Table:  tableRef,
		Schema: protoMerged,
	}); err != nil {
		return nil, fmt.Errorf("schema update register table: %w", err)
	}
	if len(preserved) > 0 {
		ref := seed.TableRef{ProjectID: projectID, DatasetID: datasetID, TableID: tableID}
		applier := seed.NewCatalogApplier(catalog)
		if _, err := applier.InsertRows(ctx, ref, protoMerged, preserved); err != nil {
			return nil, fmt.Errorf("schema update re-insert rows: %w", err)
		}
	}
	return protoMerged, nil
}

// MergeSchemasForAppend merges an existing table schema with a query
// result schema honoring BigQuery schemaUpdateOptions.
func MergeSchemasForAppend(
	existing *bqtypes.TableSchema,
	query *bqtypes.TableSchema,
	opts []string,
) (*bqtypes.TableSchema, bool) {
	merged, changed, err := mergeSchemas(existing, query, opts, false)
	if err != nil {
		return existing, false
	}
	return merged, changed
}

// MergeSchemasForTablePatch merges a PATCH body schema into the catalog
// schema, updating descriptions and relaxing REQUIRED→NULLABLE. Returns
// an error when the patch narrows modes or changes types.
func MergeSchemasForTablePatch(
	existing *bqtypes.TableSchema,
	patch *bqtypes.TableSchema,
) (*bqtypes.TableSchema, bool, error) {
	return mergeSchemas(existing, patch, TablePatchSchemaOptions, true)
}

// ApplySchemaUpdate merges querySchema into the destination catalog
// table when schemaUpdateOptions require field addition or relaxation.
// Existing rows are preserved via drop-and-recreate when the merged
// schema differs from the catalog.
func ApplySchemaUpdate(ctx context.Context, catalog enginepb.CatalogClient,
	tableRef *enginepb.TableRef, querySchema *bqtypes.TableSchema, opts []string,
) (*enginepb.TableSchema, error) {
	if len(opts) == 0 || querySchema == nil {
		desc, err := catalog.DescribeTable(ctx, &enginepb.DescribeTableRequest{Table: tableRef})
		if err != nil {
			return nil, fmt.Errorf("describe destination table: %w", err)
		}
		return desc.GetSchema(), nil
	}
	desc, err := catalog.DescribeTable(ctx, &enginepb.DescribeTableRequest{Table: tableRef})
	if err != nil {
		return nil, fmt.Errorf("describe destination table: %w", err)
	}
	existing := schemaFromProto(desc.GetSchema())
	merged, changed, err := mergeSchemas(existing, querySchema, opts, false)
	if err != nil {
		return nil, err
	}
	if !changed {
		return desc.GetSchema(), nil
	}
	preserved, err := listAllRows(ctx, catalog, tableRef, desc.GetSchema())
	if err != nil {
		return nil, err
	}
	protoMerged := SchemaToProto(merged)
	if _, err := catalog.DropTable(ctx, &enginepb.DropTableRequest{Table: tableRef}); err != nil {
		return nil, fmt.Errorf("schema update drop table: %w", err)
	}
	if _, err := catalog.RegisterTable(ctx, &enginepb.RegisterTableRequest{
		Table:  tableRef,
		Schema: protoMerged,
	}); err != nil {
		return nil, fmt.Errorf("schema update register table: %w", err)
	}
	if len(preserved) > 0 {
		ref := seed.TableRef{
			ProjectID: tableRef.GetProjectId(),
			DatasetID: tableRef.GetDatasetId(),
			TableID:   tableRef.GetTableId(),
		}
		applier := seed.NewCatalogApplier(catalog)
		if _, err := applier.InsertRows(ctx, ref, protoMerged, preserved); err != nil {
			return nil, fmt.Errorf("schema update re-insert rows: %w", err)
		}
	}
	return protoMerged, nil
}

func mergeSchemas(
	existing *bqtypes.TableSchema,
	load *bqtypes.TableSchema,
	opts []string,
	strictPatch bool,
) (*bqtypes.TableSchema, bool, error) {
	if existing == nil {
		existing = &bqtypes.TableSchema{}
	}
	if load == nil {
		return existing, false, nil
	}
	allowAdd := slices.Contains(opts, schemaUpdateAllowFieldAddition)
	allowRelax := slices.Contains(opts, schemaUpdateAllowFieldRelaxation)
	if !allowAdd && !allowRelax {
		return existing, false, nil
	}

	if strictPatch {
		if err := validatePatchAgainstExisting(existing, load); err != nil {
			return nil, false, err
		}
	}

	merged := cloneBQSchema(existing)
	changed := applySchemaFieldAdditions(merged, load, allowAdd)
	changed = applySchemaRelaxationAndDescriptions(merged, load, allowRelax) || changed
	return merged, changed, nil
}

func validatePatchAgainstExisting(existing *bqtypes.TableSchema, load *bqtypes.TableSchema) error {
	for _, f := range load.Fields {
		idx := fieldIndex(existing.Fields, f.Name)
		if idx < 0 {
			continue
		}
		if err := validatePatchFieldCompatibility(existing.Fields[idx], f); err != nil {
			return err
		}
	}
	return nil
}

func applySchemaFieldAdditions(merged *bqtypes.TableSchema, load *bqtypes.TableSchema, allowAdd bool) bool {
	if !allowAdd {
		return false
	}
	changed := false
	for _, f := range load.Fields {
		if fieldIndex(merged.Fields, f.Name) >= 0 {
			continue
		}
		nf := f
		if nf.Mode == fieldModeRequired {
			nf.Mode = ""
		}
		merged.Fields = append(merged.Fields, nf)
		changed = true
	}
	return changed
}

func applySchemaRelaxationAndDescriptions(
	merged *bqtypes.TableSchema,
	load *bqtypes.TableSchema,
	allowRelax bool,
) bool {
	changed := false
	for i := range merged.Fields {
		name := merged.Fields[i].Name
		if allowRelax &&
			merged.Fields[i].Mode == fieldModeRequired &&
			!loadKeepsFieldRequired(load, name) {
			merged.Fields[i].Mode = ""
			changed = true
		}
		patchIdx := fieldIndex(load.Fields, name)
		if patchIdx < 0 {
			continue
		}
		patchField := load.Fields[patchIdx]
		if patchField.Description != "" &&
			merged.Fields[i].Description != patchField.Description {
			merged.Fields[i].Description = patchField.Description
			changed = true
		}
	}
	return changed
}

func validatePatchFieldCompatibility(
	existing, patch bqtypes.TableFieldSchema,
) error {
	if !fieldTypesCompatible(existing.Type, patch.Type) {
		return fmt.Errorf(
			"schema update: cannot change type of field %q from %s to %s",
			existing.Name, existing.Type, patch.Type,
		)
	}
	exMode := normalizeFieldMode(existing.Mode)
	patchMode := normalizeFieldMode(patch.Mode)
	if exMode != fieldModeRequired && patchMode == fieldModeRequired {
		return fmt.Errorf(
			"schema update: cannot change mode of field %q from %s to REQUIRED",
			existing.Name, modeLabel(exMode),
		)
	}
	return nil
}

func fieldTypesCompatible(existingType, patchType string) bool {
	a := strings.ToUpper(strings.TrimSpace(existingType))
	b := strings.ToUpper(strings.TrimSpace(patchType))
	if a == b {
		return true
	}
	// REST INTEGER vs engine INT64 round-trip.
	if (a == fieldTypeInt64 || a == fieldTypeInteger) && (b == fieldTypeInt64 || b == fieldTypeInteger) {
		return true
	}
	if (a == fieldTypeFloat64 || a == fieldTypeFloat) && (b == fieldTypeFloat64 || b == fieldTypeFloat) {
		return true
	}
	if (a == fieldTypeBool || a == fieldTypeBoolean) && (b == fieldTypeBool || b == fieldTypeBoolean) {
		return true
	}
	return false
}

func normalizeFieldMode(mode string) string {
	if mode == "" || strings.EqualFold(mode, "NULLABLE") {
		return ""
	}
	return strings.ToUpper(mode)
}

func modeLabel(mode string) string {
	if mode == "" {
		return "NULLABLE"
	}
	return mode
}

func loadKeepsFieldRequired(load *bqtypes.TableSchema, name string) bool {
	if load == nil {
		return false
	}
	idx := fieldIndex(load.Fields, name)
	// Query/load results default to NULLABLE; only keep REQUIRED when the
	// incoming schema still requires it.
	return idx >= 0 && load.Fields[idx].Mode == fieldModeRequired
}

func cloneBQSchema(s *bqtypes.TableSchema) *bqtypes.TableSchema {
	if s == nil {
		return &bqtypes.TableSchema{}
	}
	out := &bqtypes.TableSchema{Fields: make([]bqtypes.TableFieldSchema, len(s.Fields))}
	copy(out.Fields, s.Fields)
	return out
}

func fieldIndex(fields []bqtypes.TableFieldSchema, name string) int {
	for i, f := range fields {
		if f.Name == name {
			return i
		}
	}
	return -1
}

func schemaFromProto(s *enginepb.TableSchema) *bqtypes.TableSchema {
	if s == nil {
		return nil
	}
	out := &bqtypes.TableSchema{Fields: make([]bqtypes.TableFieldSchema, 0, len(s.Fields))}
	for _, f := range s.Fields {
		out.Fields = append(out.Fields, bqtypes.TableFieldSchema{
			Name:        f.GetName(),
			Type:        f.GetType(),
			Mode:        f.GetMode(),
			Description: f.GetDescription(),
		})
	}
	return out
}

func listAllRows(ctx context.Context, catalog enginepb.CatalogClient,
	tableRef *enginepb.TableRef, schema *enginepb.TableSchema,
) ([]map[string]any, error) {
	const page = 10000
	var out []map[string]any
	start := int64(0)
	for {
		resp, err := catalog.ListRows(ctx, &enginepb.ListRowsRequest{
			Table:      tableRef,
			StartIndex: start,
			MaxResults: page,
		})
		if err != nil {
			return nil, fmt.Errorf("list rows for schema update: %w", err)
		}
		rows := resp.GetRows()
		if len(rows) == 0 {
			break
		}
		for _, row := range rows {
			out = append(out, protoRowToMap(schema, row))
		}
		start += int64(len(rows))
		if start >= resp.GetTotalRows() {
			break
		}
	}
	return out, nil
}

func protoRowToMap(schema *enginepb.TableSchema, row *enginepb.DataRow) map[string]any {
	fields := schema.GetFields()
	cells := row.GetCells()
	out := make(map[string]any, len(fields))
	for i, f := range fields {
		if i >= len(cells) {
			out[f.GetName()] = nil
			continue
		}
		out[f.GetName()] = protoCellToAny(cells[i])
	}
	return out
}

func protoCellToAny(c *enginepb.Cell) any {
	if c == nil || c.GetNullValue() {
		return nil
	}
	if v := c.GetStringValue(); v != "" || c.GetValue() != nil {
		return v
	}
	return nil
}
