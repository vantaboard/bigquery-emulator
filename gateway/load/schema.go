package load

import (
	"context"
	"fmt"
	"slices"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"github.com/vantaboard/bigquery-emulator/gateway/seed"
)

const (
	schemaUpdateAllowFieldAddition   = "ALLOW_FIELD_ADDITION"
	schemaUpdateAllowFieldRelaxation = "ALLOW_FIELD_RELAXATION"
)

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
	merged, changed := mergeSchemas(existing, explicit, cfg.SchemaUpdateOptions)
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
	return mergeSchemas(existing, query, opts)
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
	merged, changed := mergeSchemas(existing, querySchema, opts)
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
) (*bqtypes.TableSchema, bool) {
	if existing == nil {
		existing = &bqtypes.TableSchema{}
	}
	allowAdd := slices.Contains(opts, schemaUpdateAllowFieldAddition)
	allowRelax := slices.Contains(opts, schemaUpdateAllowFieldRelaxation)
	if !allowAdd && !allowRelax {
		return existing, false
	}

	merged := cloneBQSchema(existing)
	changed := false

	if allowAdd && load != nil {
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
	}
	if allowRelax {
		for i := range merged.Fields {
			if merged.Fields[i].Mode != fieldModeRequired {
				continue
			}
			if load != nil {
				if idx := fieldIndex(load.Fields, merged.Fields[i].Name); idx >= 0 {
					// Query/load results default to NULLABLE; only keep
					// REQUIRED when the incoming schema still requires it.
					if load.Fields[idx].Mode == fieldModeRequired {
						continue
					}
				}
			}
			merged.Fields[i].Mode = ""
			changed = true
		}
	}
	return merged, changed
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
