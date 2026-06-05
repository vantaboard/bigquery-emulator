package handlers

import (
	"context"
	"encoding/json"
	"net/http"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"google.golang.org/grpc"
)

func TestParseCreateTableDestinationSessionTable(t *testing.T) {
	t.Parallel()
	dest := parseCreateTableDestination("dev",
		"CREATE TEMP TABLE `_SESSION`.bqdf_abc123 (x INT64)")
	if dest == nil {
		t.Fatal("expected destination")
	}
	if dest.ProjectID != "dev" || dest.DatasetID != "_SESSION" ||
		dest.TableID != "bqdf_abc123" {
		t.Fatalf("dest = %+v", dest)
	}
}

func TestStampQueryJobDestinationOnCreateTempTable(t *testing.T) {
	t.Parallel()
	job := &jobs.Job{
		JobReference: bqtypes.JobReference{ProjectID: "dev"},
		Configuration: &jobs.JobConfiguration{
			Query: &jobs.JobConfigurationQuery{
				Query: "CREATE TEMP TABLE `_SESSION`.bqdf_xyz (a STRING)",
			},
		},
	}
	stampQueryJobDestination("dev", job, "CREATE_TABLE")
	if job.Configuration.Query.DestinationTable == nil {
		t.Fatal("expected destinationTable on job configuration")
	}
	if job.Configuration.Query.DestinationTable.TableID != "bqdf_xyz" {
		t.Fatalf("tableId = %q", job.Configuration.Query.DestinationTable.TableID)
	}
}

func TestJobInsertCreateTempTableSetsDestination(t *testing.T) {
	t.Parallel()
	fake := &fakeQueryClient{
		executeQueryFn: func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return &fakeQueryResultStream{
				msgs: []*enginepb.QueryResultRow{
					{StatementType: "CREATE_TABLE"},
				},
			}, nil
		},
	}
	deps := Dependencies{Jobs: jobs.NewRegistry(), Query: fake}
	body := "{\"configuration\":{\"query\":{\"query\":\"CREATE TEMP TABLE `_SESSION`.bqdf_test (x INT64)\",\"useLegacySql\":false}}}"
	rec := runJobInsert(t, deps, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("JobInsert -> %d: %s", rec.Code, rec.Body.String())
	}
	var job jobs.Job
	if err := json.Unmarshal(rec.Body.Bytes(), &job); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if job.Configuration == nil || job.Configuration.Query == nil ||
		job.Configuration.Query.DestinationTable == nil {
		t.Fatalf("missing destinationTable: %+v", job.Configuration)
	}
	if job.Configuration.Query.DestinationTable.DatasetID != "_SESSION" {
		t.Fatalf("datasetId = %q", job.Configuration.Query.DestinationTable.DatasetID)
	}
}
