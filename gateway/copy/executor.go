// Package copy implements synchronous BigQuery COPY jobs.
package copy

import (
	"context"
	"errors"
	"fmt"
	"io"
	"strconv"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"github.com/vantaboard/bigquery-emulator/gateway/seed"
	"github.com/vantaboard/bigquery-emulator/gateway/snapshots"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

const (
	writeTruncate = "WRITE_TRUNCATE"
	writeEmpty    = "WRITE_EMPTY"
	writeAppend   = "WRITE_APPEND"
	createNever   = "CREATE_NEVER"
)

// Result captures copy-job statistics.
type Result struct {
	CopiedRows         int64
	CopiedLogicalBytes int64
}

// Execute runs a synchronous COPY job.
func Execute(ctx context.Context, catalog enginepb.CatalogClient, query enginepb.QueryClient,
	snapStore *snapshots.Store, cfg *jobs.JobConfigurationCopy, defaultProject string,
) (Result, error) {
	if cfg == nil {
		return Result{}, errors.New("copy configuration is required")
	}
	if cfg.DestinationTable == nil || cfg.DestinationTable.TableID == "" {
		return Result{}, errors.New("destinationTable.tableId is required")
	}
	sources := sourceRefs(cfg, defaultProject)
	if len(sources) == 0 {
		return Result{}, errors.New("sourceTable or sourceTables is required")
	}

	destProject := cfg.DestinationTable.ProjectID
	if destProject == "" {
		destProject = defaultProject
	}
	destDataset := cfg.DestinationTable.DatasetID
	destTable := cfg.DestinationTable.TableID

	wd := cfg.WriteDisposition
	if wd == "" {
		if len(sources) > 1 {
			wd = writeAppend
		} else {
			wd = writeEmpty
		}
	}
	cd := cfg.CreateDisposition

	if err := checkCreateDisposition(ctx, catalog, cd, destProject, destDataset, destTable); err != nil {
		return Result{}, err
	}

	if hasSnapshotSource(sources) {
		return executeCatalogCopy(ctx, catalog, snapStore, sources, destProject, destDataset, destTable, wd)
	}
	if query != nil {
		if result, err := executeSQLCopy(
			ctx,
			catalog,
			query,
			sources,
			destProject,
			destDataset,
			destTable,
			wd,
			cd,
		); err == nil {
			return result, nil
		}
	}
	return executeCatalogCopy(ctx, catalog, snapStore, sources, destProject, destDataset, destTable, wd)
}

func checkCreateDisposition(ctx context.Context, catalog enginepb.CatalogClient,
	cd, projectID, datasetID, tableID string,
) error {
	if cd != createNever {
		return nil
	}
	ref := &enginepb.TableRef{ProjectId: projectID, DatasetId: datasetID, TableId: tableID}
	if !tableExists(ctx, catalog, ref) {
		return status.Error(codes.NotFound,
			fmt.Sprintf("Not found: Table %s:%s.%s", projectID, datasetID, tableID))
	}
	return nil
}

func sourceRefs(cfg *jobs.JobConfigurationCopy, defaultProject string) []bqtypes.TableReference {
	if len(cfg.SourceTables) > 0 {
		out := make([]bqtypes.TableReference, len(cfg.SourceTables))
		copy(out, cfg.SourceTables)
		for i := range out {
			if out[i].ProjectID == "" {
				out[i].ProjectID = defaultProject
			}
		}
		return out
	}
	if cfg.SourceTable != nil {
		ref := *cfg.SourceTable
		if ref.ProjectID == "" {
			ref.ProjectID = defaultProject
		}
		return []bqtypes.TableReference{ref}
	}
	return nil
}

func hasSnapshotSource(refs []bqtypes.TableReference) bool {
	for _, ref := range refs {
		if _, _, ok := snapshots.ParseDecorator(ref.TableID); ok {
			return true
		}
	}
	return false
}

func executeSQLCopy(ctx context.Context, catalog enginepb.CatalogClient, query enginepb.QueryClient,
	sources []bqtypes.TableReference, destProject, destDataset, destTable, wd, cd string,
) (Result, error) {
	if cd == createNever {
		ref := &enginepb.TableRef{ProjectId: destProject, DatasetId: destDataset, TableId: destTable}
		if !tableExists(ctx, catalog, ref) {
			return Result{}, status.Error(codes.NotFound,
				fmt.Sprintf("Not found: Table %s:%s.%s", destProject, destDataset, destTable))
		}
	}
	sql, err := buildCopySQL(sources, destDataset, destTable, wd)
	if err != nil {
		return Result{}, err
	}
	stream, err := query.ExecuteQuery(ctx, &enginepb.QueryRequest{
		ProjectId: destProject,
		Sql:       sql,
	})
	if err != nil {
		return Result{}, err
	}
	for {
		_, recvErr := stream.Recv()
		if recvErr != nil {
			if errors.Is(recvErr, io.EOF) {
				break
			}
			return Result{}, recvErr
		}
	}
	return countDestinationRows(ctx, catalog, destProject, destDataset, destTable)
}

func buildCopySQL(sources []bqtypes.TableReference, destDataset, destTable, wd string) (string, error) {
	selects := make([]string, 0, len(sources))
	for _, src := range sources {
		base, _, _ := snapshots.ParseDecorator(src.TableID)
		selects = append(selects, fmt.Sprintf("SELECT * FROM %s.%s",
			quoteIdent(src.DatasetID), quoteIdent(base)))
	}
	fromClause := strings.Join(selects, " UNION ALL ")
	dest := fmt.Sprintf("%s.%s", quoteIdent(destDataset), quoteIdent(destTable))
	switch wd {
	case writeTruncate:
		return fmt.Sprintf("CREATE OR REPLACE TABLE %s AS %s", dest, fromClause), nil
	case writeEmpty:
		return fmt.Sprintf("CREATE TABLE %s AS %s", dest, fromClause), nil
	case writeAppend:
		return fmt.Sprintf("INSERT INTO %s %s", dest, fromClause), nil
	default:
		return "", fmt.Errorf("unsupported writeDisposition %q", wd)
	}
}

func quoteIdent(id string) string {
	return "`" + strings.ReplaceAll(id, "`", "``") + "`"
}

func executeCatalogCopy(ctx context.Context, catalog enginepb.CatalogClient, snapStore *snapshots.Store,
	sources []bqtypes.TableReference, destProject, destDataset, destTable, wd string,
) (Result, error) {
	var mergedSchema *enginepb.TableSchema
	var mergedRows []*enginepb.DataRow
	var totalBytes int64

	for _, src := range sources {
		schema, rows, err := readSource(ctx, catalog, snapStore, src)
		if err != nil {
			return Result{}, err
		}
		if mergedSchema == nil {
			mergedSchema = schema
		} else if !schemasCompatible(mergedSchema, schema) {
			return Result{}, errors.New("source tables must have identical schemas for multi-source copy")
		}
		mergedRows = append(mergedRows, rows...)
		totalBytes += estimateRowBytes(rows)
	}
	if mergedSchema == nil {
		return Result{}, errors.New("could not resolve source table schema")
	}

	if err := ensureDataset(ctx, catalog, destProject, destDataset); err != nil {
		return Result{}, err
	}
	if err := applyWriteDisposition(ctx, catalog, destProject, destDataset, destTable, mergedSchema, wd); err != nil {
		return Result{}, err
	}

	ref := seed.TableRef{ProjectID: destProject, DatasetID: destDataset, TableID: destTable}
	applier := seed.NewCatalogApplier(catalog)
	rowMaps := protoRowsToMaps(mergedSchema, mergedRows)
	inserted, err := applier.InsertRows(ctx, ref, mergedSchema, rowMaps)
	if err != nil {
		return Result{}, err
	}
	return Result{
		CopiedRows:         int64(inserted),
		CopiedLogicalBytes: totalBytes,
	}, nil
}

func readSource(ctx context.Context, catalog enginepb.CatalogClient, snapStore *snapshots.Store,
	ref bqtypes.TableReference,
) (*enginepb.TableSchema, []*enginepb.DataRow, error) {
	base, epoch, decorated := snapshots.ParseDecorator(ref.TableID)
	if decorated {
		entry, err := snapStore.ResolveAtEpoch(ref.ProjectID, ref.DatasetID, base, epoch)
		if err != nil {
			return nil, nil, err
		}
		return entry.Schema, entry.Rows, nil
	}
	tableRef := &enginepb.TableRef{
		ProjectId: ref.ProjectID,
		DatasetId: ref.DatasetID,
		TableId:   base,
	}
	desc, err := catalog.DescribeTable(ctx, &enginepb.DescribeTableRequest{Table: tableRef})
	if err != nil {
		return nil, nil, fmt.Errorf("source table %s.%s.%s: %w",
			ref.ProjectID, ref.DatasetID, base, err)
	}
	rows, err := listAllRows(ctx, catalog, tableRef)
	if err != nil {
		return nil, nil, err
	}
	return desc.GetSchema(), rows, nil
}

func listAllRows(ctx context.Context, catalog enginepb.CatalogClient, ref *enginepb.TableRef,
) ([]*enginepb.DataRow, error) {
	var out []*enginepb.DataRow
	start := int64(0)
	const page = 10_000
	for {
		resp, err := catalog.ListRows(ctx, &enginepb.ListRowsRequest{
			Table:      ref,
			StartIndex: start,
			MaxResults: page,
		})
		if err != nil {
			return nil, err
		}
		rows := resp.GetRows()
		if len(rows) == 0 {
			break
		}
		out = append(out, rows...)
		start += int64(len(rows))
		if start >= resp.GetTotalRows() {
			break
		}
	}
	return out, nil
}

func protoRowsToMaps(schema *enginepb.TableSchema, rows []*enginepb.DataRow) []map[string]any {
	out := make([]map[string]any, 0, len(rows))
	fields := schema.GetFields()
	for _, row := range rows {
		m := make(map[string]any, len(fields))
		cells := row.GetCells()
		for i, f := range fields {
			if i < len(cells) {
				m[f.GetName()] = cellToAny(cells[i])
			}
		}
		out = append(out, m)
	}
	return out
}

func cellToAny(c *enginepb.Cell) any {
	if c == nil || c.GetNullValue() {
		return nil
	}
	return c.GetStringValue()
}

func estimateRowBytes(rows []*enginepb.DataRow) int64 {
	var n int64
	for _, row := range rows {
		for _, c := range row.GetCells() {
			if c != nil {
				n += int64(len(c.GetStringValue()))
			}
		}
	}
	return n
}

func ensureDataset(ctx context.Context, catalog enginepb.CatalogClient, projectID, datasetID string) error {
	applier := seed.NewCatalogApplier(catalog)
	_, err := applier.EnsureDataset(ctx, projectID, datasetID, "US")
	return err
}

func applyWriteDisposition(ctx context.Context, catalog enginepb.CatalogClient,
	projectID, datasetID, tableID string, schema *enginepb.TableSchema, wd string,
) error {
	ref := &enginepb.TableRef{ProjectId: projectID, DatasetId: datasetID, TableId: tableID}
	exists := tableExists(ctx, catalog, ref)
	switch wd {
	case writeTruncate:
		if exists {
			if _, err := catalog.DropTable(ctx, &enginepb.DropTableRequest{Table: ref}); err != nil {
				return fmt.Errorf("WRITE_TRUNCATE drop: %w", err)
			}
		}
		_, err := catalog.RegisterTable(ctx, &enginepb.RegisterTableRequest{Table: ref, Schema: schema})
		return err
	case writeEmpty:
		if exists {
			return status.Error(codes.AlreadyExists,
				fmt.Sprintf("Already Exists: Table %s:%s.%s", projectID, datasetID, tableID))
		}
		_, err := catalog.RegisterTable(ctx, &enginepb.RegisterTableRequest{Table: ref, Schema: schema})
		return err
	default:
		if !exists {
			_, err := catalog.RegisterTable(ctx, &enginepb.RegisterTableRequest{Table: ref, Schema: schema})
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

func schemasCompatible(a, b *enginepb.TableSchema) bool {
	if a == nil || b == nil {
		return a == b
	}
	af, bf := a.GetFields(), b.GetFields()
	if len(af) != len(bf) {
		return false
	}
	for i := range af {
		if af[i].GetName() != bf[i].GetName() ||
			af[i].GetType() != bf[i].GetType() ||
			af[i].GetMode() != bf[i].GetMode() {
			return false
		}
	}
	return true
}

func countDestinationRows(ctx context.Context, catalog enginepb.CatalogClient,
	projectID, datasetID, tableID string,
) (Result, error) {
	resp, err := catalog.ListRows(ctx, &enginepb.ListRowsRequest{
		Table: &enginepb.TableRef{
			ProjectId: projectID,
			DatasetId: datasetID,
			TableId:   tableID,
		},
		StartIndex: 0,
		MaxResults: 0,
	})
	if err != nil {
		return Result{}, err
	}
	n := resp.GetTotalRows()
	return Result{CopiedRows: n, CopiedLogicalBytes: n}, nil
}

// FormatStatistics maps Result into jobs.CopyStatistics.
func FormatStatistics(r Result) *jobs.CopyStatistics {
	return &jobs.CopyStatistics{
		CopiedRows:         strconv.FormatInt(r.CopiedRows, 10),
		CopiedLogicalBytes: strconv.FormatInt(r.CopiedLogicalBytes, 10),
	}
}
