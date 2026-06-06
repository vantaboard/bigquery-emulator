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

// MetadataStore mirrors the handlers.MetadataStore surface needed to
// stash REST-only destination table metadata without an import cycle.
type MetadataStore interface {
	MergeTable(projectID, datasetID, tableID string, patch bqtypes.Table)
}

const implicitDestDatasetID = "_bqemu_query_results"

var nonIdentRE = regexp.MustCompile(`[^a-zA-Z0-9_]+`)

const (
	writeTruncate = "WRITE_TRUNCATE"
	writeEmpty    = "WRITE_EMPTY"
	writeAppend   = "WRITE_APPEND"
)

// AppendResultsFromQueryRequest writes jobs.query output into an
// explicit destinationTable when the synchronous QueryRequest carries
// destination metadata (authViewTutorial, client.query with destination).
func AppendResultsFromQueryRequest(ctx context.Context, catalog enginepb.CatalogClient,
	req *bqtypes.QueryRequest, projectID string,
	resultSchema *bqtypes.TableSchema, rows []bqtypes.Row,
) error {
	if req == nil || req.DestinationTable == nil || req.DestinationTable.TableID == "" {
		return nil
	}
	cfg := &jobs.JobConfigurationQuery{
		DestinationTable:    req.DestinationTable,
		WriteDisposition:    req.WriteDisposition,
		SchemaUpdateOptions: req.SchemaUpdateOptions,
	}
	return AppendResults(ctx, catalog, cfg, projectID, resultSchema, rows)
}

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

	if len(rows) == 0 {
		switch wd {
		case writeTruncate, writeEmpty:
			return load.EnsureDestinationTable(ctx, catalog, destProject, destDataset, destTable,
				wd, protoResult)
		case writeAppend:
			if len(cfg.SchemaUpdateOptions) == 0 {
				return nil
			}
			if err := load.EnsureDestinationTable(ctx, catalog, destProject, destDataset, destTable,
				writeAppend, protoResult); err != nil {
				return fmt.Errorf("ensure query destination table: %w", err)
			}
			if _, err := load.ApplySchemaUpdate(
				ctx,
				catalog,
				tableRef,
				resultSchema,
				cfg.SchemaUpdateOptions,
			); err != nil {
				return err
			}
			return nil
		default:
			return nil
		}
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

// PersistDestinationMetadata stashes REST-only destination metadata
// (clustering, CMEK, time partitioning) so tables.get round-trips what
// the query job supplied.
func PersistDestinationMetadata(store MetadataStore, cfg *jobs.JobConfigurationQuery, projectID string) {
	if store == nil || cfg == nil || cfg.DestinationTable == nil {
		return
	}
	if cfg.Clustering == nil && cfg.TimePartitioning == nil &&
		cfg.DestinationEncryptionConfiguration == nil {
		return
	}
	destProject := cfg.DestinationTable.ProjectID
	if destProject == "" {
		destProject = projectID
	}
	patch := bqtypes.Table{
		Clustering:              cfg.Clustering,
		TimePartitioning:        cfg.TimePartitioning,
		EncryptionConfiguration: cfg.DestinationEncryptionConfiguration,
	}
	store.MergeTable(destProject, cfg.DestinationTable.DatasetID,
		cfg.DestinationTable.TableID, patch)
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
				m[f.Name] = restFieldValue(f, row.F[i])
			}
		}
		out = append(out, m)
	}
	return out
}

// restFieldValue converts a REST query-result cell into the map-shaped
// value seed.InsertRows expects. STRUCT columns arrive as nested Row
// objects (positional f/v), not map[string]any.
func restFieldValue(f bqtypes.TableFieldSchema, c bqtypes.Cell) any {
	if isRESTStructFieldType(f.Type) {
		if nested, ok := c.V.(bqtypes.Row); ok {
			m := make(map[string]any, len(f.Fields))
			for j, sub := range f.Fields {
				if j < len(nested.F) {
					m[sub.Name] = restFieldValue(sub, nested.F[j])
				}
			}
			return m
		}
	}
	if strings.EqualFold(f.Mode, "REPEATED") {
		if arr, ok := c.V.([]bqtypes.Cell); ok {
			elem := f
			elem.Mode = ""
			vals := make([]any, len(arr))
			for i, el := range arr {
				vals[i] = restFieldValue(elem, el)
			}
			return vals
		}
	}
	return c.V
}

func isRESTStructFieldType(t string) bool {
	switch strings.ToUpper(strings.TrimSpace(t)) {
	case "STRUCT", "RECORD":
		return true
	default:
		return false
	}
}
