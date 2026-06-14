// Package external materializes BigQuery external tables into the
// engine catalog by fetching GCS (fake-gcs), Google Sheets (fixture/live),
// or local snapshot sources and bulk-inserting parsed rows.
package external

import (
	"context"
	"errors"
	"fmt"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/external/sourceconfig"
	"github.com/vantaboard/bigquery-emulator/gateway/load"
	"github.com/vantaboard/bigquery-emulator/gateway/seed"
)

// TempDatasetID is the internal dataset ephemeral tableDefinitions are
// registered under when the query omits defaultDataset. The gateway
// sets this as default_dataset_id on the engine QueryRequest so
// unqualified table ids in SQL resolve.
const TempDatasetID = "_bq_external_temp"

// Materialize fetches external source bytes, parses them, and registers
// the destination table with rows in the engine catalog (WRITE_TRUNCATE).
func Materialize(
	ctx context.Context,
	catalog enginepb.CatalogClient,
	target Target,
	cfg *bqtypes.ExternalDataConfiguration,
) error {
	return MaterializeWith(ctx, catalog, target, cfg, nil)
}

// MaterializeWith materializes using an optional per-source resolver.
func MaterializeWith(
	ctx context.Context,
	catalog enginepb.CatalogClient,
	target Target,
	cfg *bqtypes.ExternalDataConfiguration,
	resolver *Resolver,
) error {
	if catalog == nil {
		return errors.New("external: nil CatalogClient")
	}
	if cfg == nil {
		return errors.New("external: externalDataConfiguration is required")
	}
	if err := validateExternalConfig(cfg); err != nil {
		return err
	}
	if target.ProjectID == "" || target.DatasetID == "" || target.TableID == "" {
		return errors.New("external: project, dataset, and table id are required")
	}

	schema := target.Schema
	if schema == nil {
		schema = cfg.Schema
	}
	skip := 0
	if cfg.CsvOptions != nil {
		skip = cfg.CsvOptions.SkipLeadingRows()
	}

	parsed, err := fetchAndParse(ctx, resolver, cfg, schema, skip)
	if err != nil {
		return err
	}
	return registerParsedRows(ctx, catalog, target, schema, parsed)
}

// Target names a catalog table to materialize.
type Target struct {
	ProjectID string
	DatasetID string
	TableID   string
	// Schema from the enclosing Table resource; wins over config.Schema
	// when both are set (permanent external table inserts).
	Schema *bqtypes.TableSchema
}

func registerParsedRows(
	ctx context.Context,
	catalog enginepb.CatalogClient,
	target Target,
	fallbackSchema *bqtypes.TableSchema,
	parsed load.ParsedRows,
) error {
	protoSchema := schemaToProto(parsed.Schema)
	if protoSchema == nil {
		protoSchema = schemaToProto(fallbackSchema)
	}
	if protoSchema == nil || len(protoSchema.GetFields()) == 0 {
		return errors.New("external table requires schema or autodetect=true for CSV")
	}
	if err := ensureDataset(ctx, catalog, target.ProjectID, target.DatasetID); err != nil {
		return err
	}
	tableRef := &enginepb.TableRef{
		ProjectId: target.ProjectID,
		DatasetId: target.DatasetID,
		TableId:   target.TableID,
	}
	if tableExists(ctx, catalog, tableRef) {
		if _, err := catalog.DropTable(ctx, &enginepb.DropTableRequest{Table: tableRef}); err != nil {
			return fmt.Errorf("external drop table: %w", err)
		}
	}
	if _, err := catalog.RegisterTable(ctx, &enginepb.RegisterTableRequest{
		Table:  tableRef,
		Schema: protoSchema,
	}); err != nil {
		return fmt.Errorf("external register table: %w", err)
	}
	ref := seed.TableRef{
		ProjectID: target.ProjectID,
		DatasetID: target.DatasetID,
		TableID:   target.TableID,
	}
	applier := seed.NewCatalogApplier(catalog)
	if _, err := applier.InsertRows(ctx, ref, protoSchema, parsed.Rows); err != nil {
		return fmt.Errorf("external insert rows: %w", err)
	}
	return nil
}

// PrepareTableDefinitions materializes every ephemeral external table in
// defs. Returns the default dataset id the caller should forward to the
// engine when the query omitted defaultDataset.
func PrepareTableDefinitions(
	ctx context.Context,
	catalog enginepb.CatalogClient,
	projectID string,
	defs map[string]bqtypes.ExternalDataConfiguration,
	defaultDataset string,
) (string, error) {
	return PrepareTableDefinitionsWith(ctx, catalog, projectID, defs, defaultDataset, nil)
}

// PrepareTableDefinitionsWith materializes defs with an optional resolver.
func PrepareTableDefinitionsWith(
	ctx context.Context,
	catalog enginepb.CatalogClient,
	projectID string,
	defs map[string]bqtypes.ExternalDataConfiguration,
	defaultDataset string,
	resolver *Resolver,
) (string, error) {
	if len(defs) == 0 {
		return defaultDataset, nil
	}
	ds := defaultDataset
	if ds == "" {
		ds = TempDatasetID
	}
	for tableID, cfg := range defs {
		cfgCopy := cfg
		if err := MaterializeWith(ctx, catalog, Target{
			ProjectID: projectID,
			DatasetID: ds,
			TableID:   tableID,
			Schema:    cfg.Schema,
		}, &cfgCopy, resolver); err != nil {
			return "", err
		}
	}
	if defaultDataset == "" {
		return TempDatasetID, nil
	}
	return defaultDataset, nil
}

func isGoogleSheets(cfg *bqtypes.ExternalDataConfiguration) bool {
	if strings.EqualFold(strings.TrimSpace(cfg.SourceFormat), "GOOGLE_SHEETS") {
		return true
	}
	if cfg.GoogleSheetsOptions != nil {
		return true
	}
	for _, uri := range cfg.SourceURIs {
		if strings.Contains(uri, "docs.google.com/spreadsheets") {
			return true
		}
	}
	return false
}

func validateExternalConfig(cfg *bqtypes.ExternalDataConfiguration) error {
	if isGoogleSheets(cfg) {
		if len(cfg.SourceURIs) == 0 {
			return errors.New("google sheets external table requires sourceUri")
		}
		return nil
	}
	if len(cfg.SourceURIs) == 0 {
		return errors.New("external table requires at least one sourceUri")
	}
	return nil
}

func fetchAndParse(
	ctx context.Context,
	resolver *Resolver,
	cfg *bqtypes.ExternalDataConfiguration,
	schema *bqtypes.TableSchema,
	skipLeading int,
) (load.ParsedRows, error) {
	if isGoogleSheets(cfg) {
		return parseSheetsExternal(ctx, resolver, cfg, schema, skipLeading)
	}
	parsed, _, _, err := load.ParseExternalGCS(ctx, cfg, schema, skipLeading)
	return parsed, err
}

func ensureDataset(ctx context.Context, catalog enginepb.CatalogClient, projectID, datasetID string) error {
	applier := seed.NewCatalogApplier(catalog)
	_, err := applier.EnsureDataset(ctx, projectID, datasetID, "US")
	return err
}

func tableExists(ctx context.Context, catalog enginepb.CatalogClient, ref *enginepb.TableRef) bool {
	_, err := catalog.DescribeTable(ctx, &enginepb.DescribeTableRequest{Table: ref})
	return err == nil
}

func schemaToProto(s *bqtypes.TableSchema) *enginepb.TableSchema {
	if s == nil {
		return nil
	}
	out := &enginepb.TableSchema{Fields: make([]*enginepb.FieldSchema, 0, len(s.Fields))}
	for i := range s.Fields {
		out.Fields = append(out.Fields, fieldToProto(s.Fields[i]))
	}
	return out
}

func fieldToProto(f bqtypes.TableFieldSchema) *enginepb.FieldSchema {
	out := &enginepb.FieldSchema{
		Name:        f.Name,
		Type:        f.Type,
		Mode:        f.Mode,
		Description: f.Description,
	}
	for i := range f.Fields {
		out.Fields = append(out.Fields, fieldToProto(f.Fields[i]))
	}
	return out
}

// LoadSourceConfig loads external source resolution rules for dataDir.
func LoadSourceConfig(dataDir string) (*sourceconfig.Config, error) {
	return sourceconfig.Load(dataDir)
}
