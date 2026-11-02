package query_test

import (
	"context"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"github.com/vantaboard/bigquery-emulator/gateway/query"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

const existingDayPartitioned = "existing"

type partitionCatalog struct {
	recordingCatalog
	registered *enginepb.RegisterTableRequest
}

func (c *partitionCatalog) RegisterTable(
	_ context.Context,
	in *enginepb.RegisterTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.RegisterTableResponse, error) {
	c.registered = in
	return &enginepb.RegisterTableResponse{}, nil
}

func (c *partitionCatalog) DescribeTable(
	_ context.Context,
	in *enginepb.DescribeTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.DescribeTableResponse, error) {
	if in.GetTable().GetTableId() != existingDayPartitioned {
		return nil, status.Error(codes.NotFound, "table not found")
	}
	return &enginepb.DescribeTableResponse{Schema: &enginepb.TableSchema{IngestionPartitionType: "DAY"}}, nil
}

func TestAppendResultsRegistersIngestionPartitionType(t *testing.T) {
	t.Parallel()
	cases := []struct {
		name, tableID    string
		timePartitioning *bqtypes.TimePartitioning
		want             string
	}{
		{"new table takes the job's", testDestTableID, &bqtypes.TimePartitioning{Type: "MONTH"}, "MONTH"},
		{"truncate keeps the table's", existingDayPartitioned, nil, "DAY"},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			t.Parallel()
			cat := &partitionCatalog{}
			schema := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{{Name: "label", Type: schemaTypeSTRING}}}
			rows := []bqtypes.Row{{F: []bqtypes.Cell{{V: "alice"}}}}
			cfg := &jobs.JobConfigurationQuery{
				DestinationTable: &bqtypes.TableReference{
					ProjectID: testProjectDev,
					DatasetID: "ds",
					TableID:   tc.tableID,
				},
				TimePartitioning: tc.timePartitioning,
			}
			if err := query.AppendResults(context.Background(), cat, cfg, testProjectDev, schema, rows); err != nil {
				t.Fatalf("AppendResults: %v", err)
			}
			if got := cat.registered.GetSchema().GetIngestionPartitionType(); got != tc.want {
				t.Errorf("ingestion_partition_type = %q, want %q", got, tc.want)
			}
		})
	}
}
