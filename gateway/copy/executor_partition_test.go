package copy_test

import (
	"context"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/copy"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

type recordingQuery struct {
	enginepb.QueryClient
	called bool
}

func (q *recordingQuery) ExecuteQuery(
	context.Context, *enginepb.QueryRequest, ...grpc.CallOption,
) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
	q.called = true
	return nil, status.Error(codes.Unimplemented, "no SQL in this test")
}

func TestExecuteCopyKeepsIngestionPartitions(t *testing.T) {
	t.Parallel()
	const partition = "2024-03-04 00:00:00+00"
	cat := &multiSourceCatalog{
		tables: map[string]*enginepb.TableSchema{testTableSrc1: {
			Fields:                 []*enginepb.FieldSchema{{Name: "id", Type: schemaTypeINTEGER}},
			IngestionPartitionType: "DAY",
		}},
		rows: map[string][]*enginepb.DataRow{testTableSrc1: {{
			Cells:         []*enginepb.Cell{{Value: &enginepb.Cell_StringValue{StringValue: "1"}}},
			PartitionTime: partition,
		}}},
	}
	cfg := &jobs.JobConfigurationCopy{
		SourceTables: []bqtypes.TableReference{
			{ProjectID: testProjectDev, DatasetID: "ds", TableID: testTableSrc1},
		},
		DestinationTable: &bqtypes.TableReference{ProjectID: testProjectDev, DatasetID: "ds", TableID: testDestTableID},
		WriteDisposition: "WRITE_EMPTY",
	}
	query := &recordingQuery{}
	if _, err := copy.Execute(context.Background(), cat, query, nil, cfg, testProjectDev); err != nil {
		t.Fatalf("Execute: %v", err)
	}
	if query.called {
		t.Error("copied through SQL, which drops the partitions")
	}
	if got := cat.tables[testDestTableID].GetIngestionPartitionType(); got != "DAY" {
		t.Errorf("destination partition type = %q, want DAY", got)
	}
	if got := cat.rows[testDestTableID][0].GetPartitionTime(); got != partition {
		t.Errorf("destination partition time = %q, want %q", got, partition)
	}
}
