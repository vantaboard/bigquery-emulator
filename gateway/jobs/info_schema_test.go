package jobs

import (
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func TestInfoSchemaJobRowsMapsCoreFields(t *testing.T) {
	t.Parallel()
	reg := NewRegistry()
	j := &Job{
		JobReference: bqtypes.JobReference{ProjectID: "p", JobID: "job_1"},
		UserEmail:    "user@example.com",
		Status:       Status{State: JobStateDone},
		Statistics: Statistics{
			CreationTime:        "1700000000000",
			StartTime:           "1700000001000",
			EndTime:             "1700000002000",
			TotalBytesProcessed: "42",
		},
		Configuration: &JobConfiguration{
			JobType: "QUERY",
			Query: &JobConfigurationQuery{
				Query: "SELECT 1",
				DestinationTable: &bqtypes.TableReference{
					ProjectID: "p", DatasetID: "d", TableID: "t",
				},
			},
		},
		Result: &QueryResult{StatementType: "SELECT"},
	}
	reg.Register(j)
	rows := InfoSchemaJobRows(reg, "p")
	if len(rows) != 1 {
		t.Fatalf("rows = %d", len(rows))
	}
	if rows[0]["job_id"] != "job_1" {
		t.Fatalf("job_id = %v", rows[0]["job_id"])
	}
	if rows[0]["statement_type"] != "SELECT" {
		t.Fatalf("statement_type = %v", rows[0]["statement_type"])
	}
	dest, ok := rows[0]["destination_table"].(map[string]any)
	if !ok || dest["table_id"] != "t" {
		t.Fatalf("destination_table = %v", rows[0]["destination_table"])
	}
}
