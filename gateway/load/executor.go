package load

import (
	"context"
	"errors"
	"fmt"
	"strconv"

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
	if cfg == nil {
		return Result{}, errors.New("load configuration is required")
	}
	if cfg.DestinationTable == nil || cfg.DestinationTable.TableID == "" {
		return Result{}, errors.New("destinationTable.tableId is required")
	}
	if len(cfg.SourceURIs) == 0 {
		return Result{}, errors.New("sourceUris must not be empty")
	}

	projectID := cfg.DestinationTable.ProjectID
	if projectID == "" {
		projectID = defaultProject
	}
	datasetID := cfg.DestinationTable.DatasetID
	tableID := cfg.DestinationTable.TableID

	var totalBytes int64
	var parsed ParsedRows
	for i, uri := range cfg.SourceURIs {
		data, err := FetchSource(ctx, uri)
		if err != nil {
			return Result{}, err
		}
		totalBytes += int64(len(data))
		chunk, err := ParseSource(cfg.SourceFormat, data, cfg.Schema, cfg.SkipLeadingRows(), cfg.Autodetect)
		if err != nil {
			return Result{}, err
		}
		if i == 0 {
			parsed = chunk
		} else {
			parsed.Rows = append(parsed.Rows, chunk.Rows...)
		}
	}

	if err := ensureDataset(ctx, catalog, projectID, datasetID); err != nil {
		return Result{}, err
	}
	protoSchema := schemaToProto(parsed.Schema)
	if err := applyWriteDisposition(ctx, catalog, cfg, projectID, datasetID, tableID, protoSchema); err != nil {
		return Result{}, err
	}

	ref := seed.TableRef{ProjectID: projectID, DatasetID: datasetID, TableID: tableID}
	applier := seed.NewCatalogApplier(catalog)
	inserted, err := applier.InsertRows(ctx, ref, protoSchema, parsed.Rows)
	if err != nil {
		return Result{}, err
	}

	return Result{
		InputFiles:     len(cfg.SourceURIs),
		InputFileBytes: totalBytes,
		OutputRows:     int64(inserted),
		OutputBytes:    totalBytes,
	}, nil
}

func ensureDataset(ctx context.Context, catalog enginepb.CatalogClient, projectID, datasetID string) error {
	applier := seed.NewCatalogApplier(catalog)
	_, err := applier.EnsureDataset(ctx, projectID, datasetID, "US")
	return err
}

func applyWriteDisposition(ctx context.Context, catalog enginepb.CatalogClient,
	cfg *jobs.JobConfigurationLoad, projectID, datasetID, tableID string, schema *enginepb.TableSchema,
) error {
	wd := cfg.WriteDisposition
	if wd == "" {
		wd = "WRITE_APPEND"
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
