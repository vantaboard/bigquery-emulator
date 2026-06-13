package jobs

import (
	"strconv"
	"strings"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

// InfoSchemaJobsDataset is the internal dataset used when rewriting
// INFORMATION_SCHEMA.JOBS* queries to a catalog table the engine can scan.
const InfoSchemaJobsDataset = "_bqemu_jobs"

// InfoSchemaJobsTable is the table id holding materialized job rows.
const InfoSchemaJobsTable = "JOBS"

// InfoSchemaJobsSchema is the column layout for the materialized JOBS view.
func InfoSchemaJobsSchema() *enginepb.TableSchema {
	return &enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: "job_id", Type: "STRING"},
		{Name: "creation_time", Type: "TIMESTAMP"},
		{Name: "start_time", Type: "TIMESTAMP"},
		{Name: "end_time", Type: "TIMESTAMP"},
		{Name: "state", Type: "STRING"},
		{Name: "job_type", Type: "STRING"},
		{Name: "project_id", Type: "STRING"},
		{Name: "query", Type: "STRING"},
		{Name: "statement_type", Type: "STRING"},
		{Name: "user_email", Type: "STRING"},
		{Name: "parent_job_id", Type: "STRING"},
		{Name: "total_bytes_processed", Type: "INT64"},
		{Name: "cache_hit", Type: "BOOL"},
		{
			Name: "destination_table", Type: "STRUCT", Fields: []*enginepb.FieldSchema{
				{Name: "project_id", Type: "STRING"},
				{Name: "dataset_id", Type: "STRING"},
				{Name: "table_id", Type: "STRING"},
			},
		},
		{
			Name: "error_result", Type: "STRUCT", Fields: []*enginepb.FieldSchema{
				{Name: "reason", Type: "STRING"},
				{Name: "message", Type: "STRING"},
			},
		},
		{
			Name: "dml_statistics", Type: "STRUCT", Fields: []*enginepb.FieldSchema{
				{Name: "inserted_row_count", Type: "INT64"},
				{Name: "deleted_row_count", Type: "INT64"},
				{Name: "updated_row_count", Type: "INT64"},
			},
		},
	}}
}

// InfoSchemaJobRows materializes registry jobs for projectID into map rows
// matching InfoSchemaJobsSchema.
func InfoSchemaJobRows(reg *Registry, projectID string) []map[string]any {
	if reg == nil {
		return nil
	}
	all, _ := reg.ListByProject(projectID, ListOptions{})
	out := make([]map[string]any, 0, len(all))
	for _, j := range all {
		out = append(out, infoSchemaRowFromJob(j))
	}
	return out
}

func infoSchemaRowFromJob(j *Job) map[string]any {
	if j == nil {
		return map[string]any{}
	}
	row := map[string]any{
		"job_id":                j.JobReference.JobID,
		"creation_time":         millisToTimestamp(j.Statistics.CreationTime),
		"start_time":            millisToTimestamp(j.Statistics.StartTime),
		"end_time":              millisToTimestamp(j.Statistics.EndTime),
		"state":                 j.Status.State,
		"job_type":              jobTypeFromConfiguration(j.Configuration),
		"project_id":            j.JobReference.ProjectID,
		"query":                 queryTextFromConfiguration(j.Configuration),
		"statement_type":        statementTypeFromJob(j),
		"user_email":            j.UserEmail,
		"parent_job_id":         parentJobID(j),
		"total_bytes_processed": parseInt64Default(j.Statistics.TotalBytesProcessed, 0),
		"cache_hit":             false,
	}
	if dest := destinationTableFromConfiguration(j.Configuration); dest != nil {
		row["destination_table"] = dest
	}
	if j.Status.ErrorResult != nil {
		row["error_result"] = map[string]any{
			"reason":  j.Status.ErrorResult.Reason,
			"message": j.Status.ErrorResult.Message,
		}
	}
	if dml := dmlStatsFromJob(j); dml != nil {
		row["dml_statistics"] = dml
	}
	return row
}

func parentJobID(j *Job) string {
	if j.ParentJobID != "" {
		return j.ParentJobID
	}
	return j.Statistics.ParentJobID
}

func jobTypeFromConfiguration(cfg *JobConfiguration) string {
	if cfg == nil {
		return ""
	}
	if cfg.JobType != "" {
		return strings.ToUpper(cfg.JobType)
	}
	switch {
	case cfg.Query != nil:
		return "QUERY"
	case cfg.Load != nil:
		return "LOAD"
	case cfg.Copy != nil:
		return "COPY"
	case cfg.Extract != nil:
		return "EXTRACT"
	default:
		return ""
	}
}

func queryTextFromConfiguration(cfg *JobConfiguration) string {
	if cfg == nil || cfg.Query == nil {
		return ""
	}
	return cfg.Query.Query
}

func statementTypeFromJob(j *Job) string {
	if j.Result != nil && j.Result.StatementType != "" {
		return j.Result.StatementType
	}
	if j.Statistics.Query != nil && j.Statistics.Query.StatementType != "" {
		return j.Statistics.Query.StatementType
	}
	return ""
}

func destinationTableFromConfiguration(cfg *JobConfiguration) map[string]any {
	if cfg == nil {
		return nil
	}
	var ref *bqtypes.TableReference
	switch {
	case cfg.Query != nil && cfg.Query.DestinationTable != nil:
		ref = cfg.Query.DestinationTable
	case cfg.Load != nil && cfg.Load.DestinationTable != nil:
		ref = cfg.Load.DestinationTable
	case cfg.Copy != nil && cfg.Copy.DestinationTable != nil:
		ref = cfg.Copy.DestinationTable
	}
	if ref == nil {
		return nil
	}
	return map[string]any{
		"project_id": ref.ProjectID,
		"dataset_id": ref.DatasetID,
		"table_id":   ref.TableID,
	}
}

func dmlStatsFromJob(j *Job) map[string]any {
	var stats *bqtypes.DmlStats
	if j.Result != nil && j.Result.DmlStats != nil {
		stats = j.Result.DmlStats
	}
	if stats == nil {
		return nil
	}
	return map[string]any{
		"inserted_row_count": parseInt64Default(stats.InsertedRowCount, 0),
		"deleted_row_count":  parseInt64Default(stats.DeletedRowCount, 0),
		"updated_row_count":  parseInt64Default(stats.UpdatedRowCount, 0),
	}
}

func millisToTimestamp(ms string) any {
	if strings.TrimSpace(ms) == "" {
		return nil
	}
	n, err := strconv.ParseInt(ms, 10, 64)
	if err != nil {
		return nil
	}
	return time.UnixMilli(n).UTC().Format("2006-01-02 15:04:05.999999 UTC")
}

func parseInt64Default(s string, def int64) int64 {
	if strings.TrimSpace(s) == "" {
		return def
	}
	n, err := strconv.ParseInt(s, 10, 64)
	if err != nil {
		return def
	}
	return n
}
