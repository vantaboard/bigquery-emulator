package query

import (
	"context"
	"errors"
	"fmt"
	"regexp"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"github.com/vantaboard/bigquery-emulator/gateway/seed"
)

var infoSchemaJobsFromRE = regexp.MustCompile(
	"(?i)`(?:[^`]+`\\.)+`[^`]*INFORMATION_SCHEMA\\.(?:JOBS_BY_PROJECT|JOBS)`?")

// ReferencesInfoSchemaJobs reports whether sql reads from JOBS / JOBS_BY_PROJECT.
func ReferencesInfoSchemaJobs(sql string) bool {
	return infoSchemaJobsFromRE.MatchString(sql)
}

// RewriteInfoSchemaJobsSQL replaces INFORMATION_SCHEMA.JOBS* table refs with
// the gateway materialized catalog table for the request project.
func RewriteInfoSchemaJobsSQL(sql, projectID string) string {
	repl := fmt.Sprintf("`%s`.`%s`.`%s`", projectID, jobs.InfoSchemaJobsDataset, jobs.InfoSchemaJobsTable)
	return infoSchemaJobsFromRE.ReplaceAllString(sql, repl)
}

// PrepareInfoSchemaJobsSnapshot registers/refreshes the internal jobs table
// before forwarding a rewritten query to the engine.
func PrepareInfoSchemaJobsSnapshot(
	ctx context.Context,
	catalog enginepb.CatalogClient,
	reg *jobs.Registry,
	projectID string,
) error {
	if catalog == nil {
		return errors.New("info schema jobs: engine catalog required")
	}
	if reg == nil {
		return errors.New("info schema jobs: job registry required")
	}
	applier := seed.NewCatalogApplier(catalog)
	if _, err := applier.EnsureDataset(ctx, projectID, jobs.InfoSchemaJobsDataset, "US"); err != nil {
		return err
	}
	tableRef := seed.TableRef{
		ProjectID: projectID,
		DatasetID: jobs.InfoSchemaJobsDataset,
		TableID:   jobs.InfoSchemaJobsTable,
	}
	schema := jobs.InfoSchemaJobsSchema()
	engineTable := &enginepb.TableRef{
		ProjectId: projectID,
		DatasetId: jobs.InfoSchemaJobsDataset,
		TableId:   jobs.InfoSchemaJobsTable,
	}
	if _, err := catalog.DescribeTable(ctx, &enginepb.DescribeTableRequest{Table: engineTable}); err == nil {
		if _, dropErr := catalog.DropTable(ctx, &enginepb.DropTableRequest{Table: engineTable}); dropErr != nil {
			return fmt.Errorf("info schema jobs drop: %w", dropErr)
		}
	}
	if _, err := applier.EnsureTable(ctx, tableRef, schema); err != nil {
		return err
	}
	rows := jobs.InfoSchemaJobRows(reg, projectID)
	if len(rows) == 0 {
		return nil
	}
	if _, err := applier.InsertRows(ctx, tableRef, schema, rows); err != nil {
		return fmt.Errorf("info schema jobs insert: %w", err)
	}
	return nil
}

// PrepareEngineSQLForJobs rewrites JOBS* queries and refreshes the snapshot table.
func PrepareEngineSQLForJobs(
	ctx context.Context,
	catalog enginepb.CatalogClient,
	reg *jobs.Registry,
	projectID, sql string,
) (string, error) {
	if !ReferencesInfoSchemaJobs(sql) {
		return sql, nil
	}
	if err := PrepareInfoSchemaJobsSnapshot(ctx, catalog, reg, projectID); err != nil {
		return "", err
	}
	return RewriteInfoSchemaJobsSQL(sql, projectID), nil
}
