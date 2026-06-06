package load

import (
	"context"
	"errors"
	"fmt"
	"strconv"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"github.com/vantaboard/bigquery-emulator/gateway/seed"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

// Result captures load-job statistics for jobs.insert responses.
type Result struct {
	InputFiles     int
	InputFileBytes int64
	OutputRows     int64
	OutputBytes    int64
}

// Execute runs a synchronous LOAD job against the engine catalog.
func Execute(ctx context.Context, catalog enginepb.CatalogClient, cfg *jobs.JobConfigurationLoad,
	defaultProject string,
) (Result, error) {
	return execute(ctx, catalog, cfg, defaultProject, nil)
}

// ExecuteFromBytes runs a LOAD job using inline upload bytes instead of sourceUris.
func ExecuteFromBytes(ctx context.Context, catalog enginepb.CatalogClient, cfg *jobs.JobConfigurationLoad,
	defaultProject string, media []byte,
) (Result, error) {
	return execute(ctx, catalog, cfg, defaultProject, [][]byte{media})
}

func execute(ctx context.Context, catalog enginepb.CatalogClient, cfg *jobs.JobConfigurationLoad,
	defaultProject string, inline [][]byte,
) (Result, error) {
	if cfg == nil {
		return Result{}, errors.New("load configuration is required")
	}
	if cfg.DestinationTable == nil || cfg.DestinationTable.TableID == "" {
		return Result{}, errors.New("destinationTable.tableId is required")
	}
	if len(cfg.SourceURIs) == 0 && len(inline) == 0 {
		return Result{}, errors.New("sourceUris or upload media is required")
	}

	projectID := cfg.DestinationTable.ProjectID
	if projectID == "" {
		projectID = defaultProject
	}
	datasetID := cfg.DestinationTable.DatasetID
	tableID := cfg.DestinationTable.TableID

	parseSchema := cfg.Schema
	if parseSchema == nil || len(parseSchema.Fields) == 0 {
		if !cfg.Autodetect {
			parseSchema = existingDestinationSchema(ctx, catalog, projectID, datasetID, tableID)
		}
	}
	parsed, totalBytes, inputFiles, err := parseLoadSources(ctx, cfg, inline, parseSchema)
	if err != nil {
		return Result{}, err
	}

	if err = EnsureDataset(ctx, catalog, projectID, datasetID); err != nil {
		return Result{}, err
	}
	protoSchema, err := resolveDestinationSchema(ctx, catalog, cfg, projectID, datasetID, tableID, parsed.Schema)
	if err != nil {
		return Result{}, err
	}
	if protoSchema == nil {
		protoSchema = SchemaToProto(parsed.Schema)
	}
	if err = applyWriteDisposition(ctx, catalog, cfg, projectID, datasetID, tableID, protoSchema); err != nil {
		return Result{}, err
	}

	ref := seed.TableRef{ProjectID: projectID, DatasetID: datasetID, TableID: tableID}
	applier := seed.NewCatalogApplier(catalog)
	inserted, err := applier.InsertRows(ctx, ref, protoSchema, parsed.Rows)
	if err != nil {
		return Result{}, err
	}

	return Result{
		InputFiles:     inputFiles,
		InputFileBytes: totalBytes,
		OutputRows:     int64(inserted),
		OutputBytes:    totalBytes,
	}, nil
}

func parseLoadSources(ctx context.Context, cfg *jobs.JobConfigurationLoad, inline [][]byte,
	parseSchema *bqtypes.TableSchema,
) (parsed ParsedRows, totalBytes int64, inputFiles int, err error) {
	if len(inline) > 0 {
		return parseInlineSources(cfg, inline, parseSchema)
	}
	return parseURISources(ctx, cfg, parseSchema)
}

func parseInlineSources(cfg *jobs.JobConfigurationLoad, inline [][]byte,
	parseSchema *bqtypes.TableSchema,
) (ParsedRows, int64, int, error) {
	var parsed ParsedRows
	var totalBytes int64
	for i, data := range inline {
		totalBytes += int64(len(data))
		chunk, err := ParseSource(cfg.SourceFormat, data, parseSchema, cfg.SkipLeadingRows(), cfg.Autodetect)
		if err != nil {
			return ParsedRows{}, 0, 0, err
		}
		parsed = mergeParsedChunk(parsed, chunk, i == 0)
	}
	return parsed, totalBytes, len(inline), nil
}

func parseURISources(ctx context.Context, cfg *jobs.JobConfigurationLoad,
	parseSchema *bqtypes.TableSchema,
) (ParsedRows, int64, int, error) {
	if cfg.HivePartitioningOptions != nil {
		return parseHiveURISources(ctx, cfg, parseSchema)
	}
	sourceFormat := strings.ToUpper(strings.TrimSpace(cfg.SourceFormat))
	if sourceFormat == "" {
		sourceFormat = inferSourceFormatFromURIs(cfg.SourceURIs)
	}
	if sourceFormat == sourceFormatDatastoreBackup {
		cfgCopy := *cfg
		cfgCopy.SourceFormat = sourceFormat
		return parseDatastoreBackupSources(ctx, &cfgCopy, parseSchema)
	}
	uris, err := ExpandSourceURIs(ctx, cfg.SourceURIs)
	if err != nil {
		return ParsedRows{}, 0, 0, err
	}
	var parsed ParsedRows
	var totalBytes int64
	for i, uri := range uris {
		data, err := FetchSource(ctx, uri)
		if err != nil {
			return ParsedRows{}, 0, 0, err
		}
		totalBytes += int64(len(data))
		chunk, err := ParseSource(sourceFormat, data, parseSchema, cfg.SkipLeadingRows(), cfg.Autodetect)
		if err != nil {
			return ParsedRows{}, 0, 0, err
		}
		parsed = mergeParsedChunk(parsed, chunk, i == 0)
	}
	return parsed, totalBytes, len(uris), nil
}

func inferSourceFormatFromURIs(uris []string) string {
	for _, uri := range uris {
		if strings.HasSuffix(uri, ".export_metadata") {
			return "DATASTORE_BACKUP"
		}
	}
	return ""
}

func mergeParsedChunk(acc, chunk ParsedRows, first bool) ParsedRows {
	if first {
		return chunk
	}
	acc.Rows = append(acc.Rows, chunk.Rows...)
	return acc
}

// EnsureDestinationTable applies write-disposition semantics for a
// destination table ref, registering the schema when missing.
func EnsureDestinationTable(ctx context.Context, catalog enginepb.CatalogClient,
	projectID, datasetID, tableID, writeDisposition string, schema *enginepb.TableSchema,
) error {
	cfg := &jobs.JobConfigurationLoad{
		DestinationTable: &bqtypes.TableReference{
			ProjectID: projectID,
			DatasetID: datasetID,
			TableID:   tableID,
		},
		WriteDisposition: writeDisposition,
	}
	return applyWriteDisposition(ctx, catalog, cfg, projectID, datasetID, tableID, schema)
}

// EnsureDataset registers the dataset when missing.
func EnsureDataset(ctx context.Context, catalog enginepb.CatalogClient, projectID, datasetID string) error {
	applier := seed.NewCatalogApplier(catalog)
	_, err := applier.EnsureDataset(ctx, projectID, datasetID, "US")
	return err
}

func applyWriteDisposition(ctx context.Context, catalog enginepb.CatalogClient,
	cfg *jobs.JobConfigurationLoad, projectID, datasetID, tableID string, schema *enginepb.TableSchema,
) error {
	wd := cfg.WriteDisposition
	if wd == "" {
		wd = writeAppend
	}
	tableRef := &enginepb.TableRef{
		ProjectId: projectID,
		DatasetId: datasetID,
		TableId:   tableID,
	}

	exists := tableExists(ctx, catalog, tableRef)

	switch wd {
	case "WRITE_TRUNCATE":
		if exists {
			if _, err := catalog.DropTable(ctx, &enginepb.DropTableRequest{Table: tableRef}); err != nil {
				return fmt.Errorf("WRITE_TRUNCATE drop table: %w", err)
			}
		}
		_, err := catalog.RegisterTable(ctx, &enginepb.RegisterTableRequest{
			Table:  tableRef,
			Schema: schema,
		})
		return err
	case "WRITE_EMPTY":
		if exists {
			return fmt.Errorf("destination table %s.%s.%s is not empty", projectID, datasetID, tableID)
		}
		_, err := catalog.RegisterTable(ctx, &enginepb.RegisterTableRequest{
			Table:  tableRef,
			Schema: schema,
		})
		return err
	default: // WRITE_APPEND and CREATE_IF_NEEDED semantics
		if !exists {
			_, err := catalog.RegisterTable(ctx, &enginepb.RegisterTableRequest{
				Table:  tableRef,
				Schema: schema,
			})
			if err != nil && status.Code(err) != codes.AlreadyExists {
				return err
			}
		}
		return nil
	}
}

func tableExists(ctx context.Context, catalog enginepb.CatalogClient, ref *enginepb.TableRef) bool {
	_, err := catalog.DescribeTable(ctx, &enginepb.DescribeTableRequest{Table: ref})
	return err == nil
}

// SchemaToProto converts a REST TableSchema to engine proto form.
func SchemaToProto(s *bqtypes.TableSchema) *enginepb.TableSchema {
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

// FormatStatistics maps a Result into jobs.LoadStatistics wire counters.
func FormatStatistics(r Result) *jobs.LoadStatistics {
	return &jobs.LoadStatistics{
		InputFiles:     strconv.Itoa(r.InputFiles),
		InputFileBytes: strconv.FormatInt(r.InputFileBytes, 10),
		OutputRows:     strconv.FormatInt(r.OutputRows, 10),
		OutputBytes:    strconv.FormatInt(r.OutputBytes, 10),
		BadRecords:     "0",
	}
}
