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

type recordingCatalog struct {
	enginepb.CatalogClient
	inserted int
}

func (c *recordingCatalog) RegisterDataset(
	_ context.Context,
	_ *enginepb.RegisterDatasetRequest,
	_ ...grpc.CallOption,
) (*enginepb.RegisterDatasetResponse, error) {
	return &enginepb.RegisterDatasetResponse{}, nil
}

func (c *recordingCatalog) RegisterTable(
	_ context.Context,
	_ *enginepb.RegisterTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.RegisterTableResponse, error) {
	return &enginepb.RegisterTableResponse{}, nil
}

func (c *recordingCatalog) DescribeTable(
	_ context.Context,
	in *enginepb.DescribeTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.DescribeTableResponse, error) {
	if in.GetTable().GetTableId() == "dest" {
		return nil, status.Error(codes.NotFound, "table not found")
	}
	return &enginepb.DescribeTableResponse{}, nil
}

func (c *recordingCatalog) DropTable(
	_ context.Context,
	_ *enginepb.DropTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.DropTableResponse, error) {
	return &enginepb.DropTableResponse{}, nil
}

func (c *recordingCatalog) InsertRows(
	_ context.Context,
	in *enginepb.InsertRowsRequest,
	_ ...grpc.CallOption,
) (*enginepb.InsertRowsResponse, error) {
	c.inserted += len(in.GetRows())
	return &enginepb.InsertRowsResponse{}, nil
}

func TestAppendResultsDefaultWriteTruncate(t *testing.T) {
	t.Parallel()
	cat := &recordingCatalog{}
	schema := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "name", Type: "STRING"},
	}}
	rows := []bqtypes.Row{{F: []bqtypes.Cell{{V: "alice"}}}}
	cfg := &jobs.JobConfigurationQuery{
		DestinationTable: &bqtypes.TableReference{
			ProjectID: "dev",
			DatasetID: "ds",
			TableID:   "dest",
		},
	}
	if err := query.AppendResults(context.Background(), cat, cfg, "dev", schema, rows); err != nil {
		t.Fatalf("AppendResults: %v", err)
	}
	if cat.inserted != 1 {
		t.Fatalf("inserted = %d, want 1", cat.inserted)
	}
}

func TestAppendResultsWriteTruncateRecreates(t *testing.T) {
	t.Parallel()
	cat := &recordingCatalog{}
	schema := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "total_people", Type: "INTEGER"},
	}}
	rows := []bqtypes.Row{{F: []bqtypes.Cell{{V: float64(42)}}}}
	cfg := &jobs.JobConfigurationQuery{
		DestinationTable: &bqtypes.TableReference{ProjectID: "dev", DatasetID: "ds", TableID: "browse"},
		WriteDisposition: "WRITE_TRUNCATE",
	}
	if err := query.AppendResults(context.Background(), cat, cfg, "dev", schema, rows); err != nil {
		t.Fatalf("AppendResults: %v", err)
	}
	if cat.inserted != 1 {
		t.Fatalf("inserted = %d, want 1", cat.inserted)
	}
}
