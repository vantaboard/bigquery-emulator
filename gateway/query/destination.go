package query

import (
	"context"
	"errors"
	"fmt"
	"regexp"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"github.com/vantaboard/bigquery-emulator/gateway/load"
	"github.com/vantaboard/bigquery-emulator/gateway/seed"
)

const implicitDestDatasetID = "_bqemu_query_results"

var nonIdentRE = regexp.MustCompile(`[^a-zA-Z0-9_]+`)

const (
	writeTruncate = "WRITE_TRUNCATE"
	writeEmpty    = "WRITE_EMPTY"
	writeAppend   = "WRITE_APPEND"
)

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
	if len(rows) == 0 {
		return nil
	}
	wd := cfg.WriteDisposition
	if wd == "" {
		wd = writeTruncate
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
	protoResult := load.SchemaToProto(resultSchema)
	if err := load.EnsureDataset(ctx, catalog, destProject, destDataset); err != nil {
		return err
	}

	var protoSchema *enginepb.TableSchema
	switch wd {
	case writeAppend:
		if err := load.EnsureDestinationTable(ctx, catalog, destProject, destDataset, destTable,
			writeAppend, protoResult); err != nil {
			return fmt.Errorf("ensure query destination table: %w", err)
		}
		var err error
		protoSchema, err = load.ApplySchemaUpdate(ctx, catalog, tableRef, resultSchema, cfg.SchemaUpdateOptions)
		if err != nil {
			return err
		}
	case writeTruncate, writeEmpty:
		if err := load.EnsureDestinationTable(ctx, catalog, destProject, destDataset, destTable,
			wd, protoResult); err != nil {
			return fmt.Errorf("ensure query destination table: %w", err)
		}
		protoSchema = protoResult
	default:
		return fmt.Errorf("query destination writeDisposition %q is not supported", wd)
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

// MaterializeImplicitDestination registers an anonymous results table for
// SELECT jobs that omit destinationTable so clients can read
// query_job.destination and list_rows for pagination samples.
func MaterializeImplicitDestination(ctx context.Context, catalog enginepb.CatalogClient,
	projectID, defaultDatasetID, jobID string,
	resultSchema *bqtypes.TableSchema, rows []bqtypes.Row,
) (*bqtypes.TableReference, error) {
	if catalog == nil || resultSchema == nil || len(rows) == 0 {
		return nil, errors.New("implicit destination requires catalog, schema, and rows")
	}
	datasetID := strings.TrimSpace(defaultDatasetID)
	if datasetID == "" {
		datasetID = implicitDestDatasetID
	}
	tableID := sanitizeJobTableID(jobID)
	if err := load.EnsureDataset(ctx, catalog, projectID, datasetID); err != nil {
		return nil, err
	}
	protoSchema := load.SchemaToProto(resultSchema)
	if err := load.EnsureDestinationTable(ctx, catalog, projectID, datasetID, tableID,
		writeTruncate, protoSchema); err != nil {
		return nil, err
	}
	ref := seed.TableRef{ProjectID: projectID, DatasetID: datasetID, TableID: tableID}
	applier := seed.NewCatalogApplier(catalog)
	if _, err := applier.InsertRows(ctx, ref, protoSchema, restRowsToMaps(resultSchema, rows)); err != nil {
		return nil, err
	}
	return &bqtypes.TableReference{
		ProjectID: projectID,
		DatasetID: datasetID,
		TableID:   tableID,
	}, nil
}

func sanitizeJobTableID(jobID string) string {
	id := nonIdentRE.ReplaceAllString(jobID, "_")
	if id == "" {
		return "query_results"
	}
	return id
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
