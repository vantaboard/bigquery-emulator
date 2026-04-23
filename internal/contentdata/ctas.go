package contentdata

import (
	"context"
	"database/sql"
	"fmt"
	"os"
	"strings"
	"sync"
	"time"

	"github.com/vantaboard/go-googlesql"
	"github.com/vantaboard/go-googlesql-engine"
	"github.com/vantaboard/go-googlesql/ast"
	bigqueryv2 "google.golang.org/api/bigquery/v2"

	"github.com/vantaboard/bigquery-emulator/internal/connection"
	"github.com/vantaboard/bigquery-emulator/internal/logger"
	internaltypes "github.com/vantaboard/bigquery-emulator/internal/types"
	"github.com/vantaboard/bigquery-emulator/types"
	"log/slog"
)

// ctasQueryParserOptions returns parser options for lightweight classification of SQL text.
// A single default instance is enough for [IsCTASQuery] (read-only use by [googlesql.ParseStatement]).
var ctasQueryParserOptions = sync.OnceValue(func() *googlesql.ParserOptions {
	return googlesql.NewParserOptions()
})

// IsCTASQuery returns true when the statement is parsed as a CREATE (OR REPLACE) TABLE
// with an AS <query> clause (CTAS), using the GoogleSQL parser from go-googlesql. On parse
// failure or other statement kinds, it returns false.
func IsCTASQuery(q string) bool {
	s := strings.TrimSpace(q)
	if s == "" {
		return false
	}
	stmt, err := googlesql.ParseStatement(s, ctasQueryParserOptions())
	if err != nil {
		return false
	}
	ct, ok := stmt.(*ast.CreateTableStatementNode)
	if !ok {
		return false
	}
	// Table DDL without AS <query> is still CreateTableStatementNode but Query() is nil.
	return ct.Query() != nil
}

func namePathEqual(a, b []string) bool {
	if len(a) != len(b) {
		return false
	}
	for i := range a {
		if a[i] != b[i] {
			return false
		}
	}
	return true
}

func findTableSpecInChangedCatalog(
	cc *googlesqlengine.ChangedCatalog, projectID, datasetID, tableID string,
) *googlesqlengine.TableSpec {
	want := []string{projectID, datasetID, tableID}
	if cc == nil || cc.Table == nil {
		return nil
	}
	for _, t := range cc.Table.Added {
		if t != nil && namePathEqual(t.NamePath, want) {
			return t
		}
	}
	for _, t := range cc.Table.Updated {
		if t != nil && namePathEqual(t.NamePath, want) {
			return t
		}
	}
	return nil
}

func tableSpecToBqSchema(spec *googlesqlengine.TableSpec) (*bigqueryv2.TableSchema, error) {
	if spec == nil {
		return nil, fmt.Errorf("table spec is nil")
	}
	fields := make([]*bigqueryv2.TableFieldSchema, 0, len(spec.Columns))
	for _, col := range spec.Columns {
		googlesqlType, err := col.Type.ToGoogleSQLType()
		if err != nil {
			return nil, fmt.Errorf("column %s: %w", col.Name, err)
		}
		fields = append(fields, types.TableFieldSchemaFromType(col.Name, googlesqlType))
	}
	return &bigqueryv2.TableSchema{Fields: fields}, nil
}

// QueryCTASInPlace runs a CREATE (OR REPLACE) TABLE ... AS ... statement with Exec, avoiding
// row-by-row materialization into Go. The destination table is filled by the engine; callers
// must not re-insert. Returns CTASInPlace on the response.
func (r *Repository) QueryCTASInPlace(
	ctx context.Context,
	tx *connection.Tx,
	projectID, datasetID string,
	dest *bigqueryv2.TableReference,
	query string,
	params []*bigqueryv2.QueryParameter,
) (*internaltypes.QueryResponse, error) {
	if dest == nil {
		return nil, fmt.Errorf("destination table is nil")
	}
	if !IsCTASQuery(query) {
		return nil, fmt.Errorf("not a CTAS query")
	}

	tx.SetProjectAndDataset(projectID, datasetID)
	if err := tx.ContentRepoMode(); err != nil {
		return nil, err
	}
	defer func() {
		_ = tx.MetadataRepoMode()
	}()

	ctx = googlesqlengine.WithLogger(ctx, logger.Logger(ctx))

	values := []interface{}{}
	for _, param := range params {
		value, err := r.queryParameterValueToGoValue(param.ParameterValue)
		if err != nil {
			return nil, err
		}
		if param.Name != "" {
			values = append(values, sql.Named(param.Name, value))
		} else {
			values = append(values, value)
		}
	}

	logger.Logger(ctx).LogAttrs(
		ctx,
		slog.LevelDebug,
		"content query CTAS in-place (Exec)",
		slog.String("query", truncateQueryForLog(query, 2048)),
	)

	if err := tx.Conn().Raw(func(c interface{}) error {
		googlesqlengineConn, ok := c.(*googlesqlengine.GoogleSQLEngineConn)
		if !ok {
			return fmt.Errorf("failed to get GoogleSQLEngineConn from %T", c)
		}
		googlesqlengineConn.SetQueryParameters(params)
		return nil
	}); err != nil {
		return nil, fmt.Errorf("failed to setup connection: %w", err)
	}

	execStart := time.Now()
	result, err := tx.Tx().ExecContext(ctx, query, values...)
	execMS := time.Since(execStart).Milliseconds()
	if err != nil {
		return nil, err
	}

	rowsAff, rowsAffErr := result.RowsAffected()

	buildStart := time.Now()
	changedCatalog, err := googlesqlengine.ChangedCatalogFromResult(result)
	if err != nil {
		return nil, fmt.Errorf("failed to get changed catalog: %w", err)
	}
	spec := findTableSpecInChangedCatalog(
		changedCatalog,
		dest.ProjectId,
		dest.DatasetId,
		dest.TableId,
	)
	if spec == nil {
		return nil, fmt.Errorf(
			"CTAS completed but destination table spec not in changed catalog: %s.%s.%s",
			dest.ProjectId, dest.DatasetId, dest.TableId,
		)
	}

	schema, err := tableSpecToBqSchema(spec)
	if err != nil {
		return nil, err
	}
	schemaMS := time.Since(buildStart).Milliseconds()

	forceCount := os.Getenv("BQ_EMULATOR_CTAS_INPLACE_FORCE_COUNT") == "1"
	tpath := r.tablePath(dest.ProjectId, dest.DatasetId, dest.TableId)
	var totalRows int64
	var countMS int64
	rowCountSource := "count_query"
	if !forceCount && rowsAffErr == nil && rowsAff > 0 {
		totalRows = rowsAff
		rowCountSource = "rows_affected"
	} else {
		countStart := time.Now()
		countQuery := fmt.Sprintf("SELECT COUNT(1) AS c FROM `%s`", tpath)
		if err := tx.Tx().QueryRowContext(ctx, countQuery).Scan(&totalRows); err != nil {
			return nil, fmt.Errorf("row count for CTAS table: %w", err)
		}
		countMS = time.Since(countStart).Milliseconds()
	}
	if totalRows < 0 {
		totalRows = 0
	}

	if logger.Logger(ctx) != nil {
		attrs := []slog.Attr{
			slog.String("destination", fmt.Sprintf("%s.%s.%s", dest.ProjectId, dest.DatasetId, dest.TableId)),
			slog.Int64("exec_ms", execMS),
			slog.Int64("schema_extract_ms", schemaMS),
			slog.String("row_count_source", rowCountSource),
			slog.Int64("row_count_ms", countMS),
		}
		attrs = append(attrs, slog.Int64("rows_affected", rowsAff), slog.Int64("total_rows", totalRows))
		if rowsAffErr != nil {
			attrs = append(attrs, slog.String("rows_affected_err", rowsAffErr.Error()))
		}
		logger.Logger(ctx).LogAttrs(ctx, slog.LevelInfo, "CTAS in-place phase timings", attrs...)
	}

	return &internaltypes.QueryResponse{
		Schema:         schema,
		TotalRows:      uint64(totalRows),
		JobComplete:    true,
		Rows:           nil,
		TotalBytes:     0,
		ChangedCatalog: changedCatalog,
		CTASInPlace:    true,
	}, nil
}
