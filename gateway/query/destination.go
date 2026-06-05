package query

import (
	"context"
	"errors"
	"fmt"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"github.com/vantaboard/bigquery-emulator/gateway/load"
	"github.com/vantaboard/bigquery-emulator/gateway/seed"
)

const writeAppend = "WRITE_APPEND"

// AppendResults writes synchronous query output into
// configuration.query.destinationTable when the job requests a
// destination table. Schema update options on WRITE_APPEND merge new
// columns or relax REQUIRED fields before rows are inserted.
func AppendResults(ctx context.Context, catalog enginepb.CatalogClient,
	cfg *jobs.JobConfigurationQuery, projectID string,
	resultSchema *bqtypes.TableSchema, rows []bqtypes.Row,
) error {
	if cfg == nil || cfg.DestinationTable == nil || cfg.DestinationTable.TableID == "" {
		return nil
	}
	if catalog == nil {
		return errors.New("query destination requires Catalog client")
	}
	wd := cfg.WriteDisposition
	if wd == "" {
		wd = writeAppend
	}
	if wd != writeAppend {
		return fmt.Errorf("query destination writeDisposition %q is not supported", wd)
	}

	destProject := cfg.DestinationTable.ProjectID
	if destProject == "" {
		destProject = projectID
	}
	destDataset := cfg.DestinationTable.DatasetID
	destTable := cfg.DestinationTable.TableID

	tableRef := &enginepb.TableRef{
		ProjectId: destProject,
		DatasetId: destDataset,
		TableId:   destTable,
	}
	protoSchema, err := load.ApplySchemaUpdate(ctx, catalog, tableRef, resultSchema, cfg.SchemaUpdateOptions)
	if err != nil {
		return err
	}
	if protoSchema == nil {
		desc, derr := catalog.DescribeTable(ctx, &enginepb.DescribeTableRequest{Table: tableRef})
		if derr != nil {
			return fmt.Errorf("describe destination table: %w", derr)
		}
		protoSchema = desc.GetSchema()
	}

	ref := seed.TableRef{ProjectID: destProject, DatasetID: destDataset, TableID: destTable}
	applier := seed.NewCatalogApplier(catalog)
	rowMaps := restRowsToMaps(resultSchema, rows)
	if _, err := applier.InsertRows(ctx, ref, protoSchema, rowMaps); err != nil {
		return fmt.Errorf("query destination insert rows: %w", err)
	}
	return nil
}

func restRowsToMaps(schema *bqtypes.TableSchema, rows []bqtypes.Row) []map[string]any {
	if schema == nil || len(rows) == 0 {
		return nil
	}
	out := make([]map[string]any, 0, len(rows))
	for _, row := range rows {
		m := make(map[string]any, len(schema.Fields))
		for i, f := range schema.Fields {
			if i < len(row.F) {
				m[f.Name] = row.F[i].V
			}
		}
		out = append(out, m)
	}
	return out
}
