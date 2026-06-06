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

type multiSourceCatalog struct {
	enginepb.CatalogClient
	tables map[string]*enginepb.TableSchema
	rows   map[string][]*enginepb.DataRow
}

func (c *multiSourceCatalog) RegisterDataset(
	_ context.Context,
	_ *enginepb.RegisterDatasetRequest,
	_ ...grpc.CallOption,
) (*enginepb.RegisterDatasetResponse, error) {
	return &enginepb.RegisterDatasetResponse{}, nil
}

func (c *multiSourceCatalog) RegisterTable(
	_ context.Context,
	in *enginepb.RegisterTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.RegisterTableResponse, error) {
	key := in.GetTable().GetTableId()
	c.tables[key] = in.GetSchema()
	return &enginepb.RegisterTableResponse{}, nil
}

func (c *multiSourceCatalog) DescribeTable(
	_ context.Context,
	in *enginepb.DescribeTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.DescribeTableResponse, error) {
	key := in.GetTable().GetTableId()
	schema, ok := c.tables[key]
	if !ok {
		return nil, status.Error(codes.NotFound, "table not found")
	}
	return &enginepb.DescribeTableResponse{Schema: schema}, nil
}

func (c *multiSourceCatalog) DropTable(
	_ context.Context,
	in *enginepb.DropTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.DropTableResponse, error) {
	delete(c.tables, in.GetTable().GetTableId())
	return &enginepb.DropTableResponse{}, nil
}

func (c *multiSourceCatalog) ListRows(
	_ context.Context,
	in *enginepb.ListRowsRequest,
	_ ...grpc.CallOption,
) (*enginepb.ListRowsResponse, error) {
	key := in.GetTable().GetTableId()
	rows := c.rows[key]
	return &enginepb.ListRowsResponse{
		TotalRows: int64(len(rows)),
		Rows:      rows,
	}, nil
}

func (c *multiSourceCatalog) InsertRows(
	_ context.Context,
	in *enginepb.InsertRowsRequest,
	_ ...grpc.CallOption,
) (*enginepb.InsertRowsResponse, error) {
	key := in.GetTable().GetTableId()
	c.rows[key] = append(c.rows[key], in.GetRows()...)
	return &enginepb.InsertRowsResponse{}, nil
}

func TestExecuteMultiSourceCopy(t *testing.T) {
	t.Parallel()
	schema := &enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: "id", Type: "INTEGER"},
	}}
	cat := &multiSourceCatalog{
		tables: map[string]*enginepb.TableSchema{
			"src1": schema,
			"src2": schema,
		},
		rows: map[string][]*enginepb.DataRow{
			"src1": {{Cells: []*enginepb.Cell{{Value: &enginepb.Cell_StringValue{StringValue: "1"}}}}},
			"src2": {{Cells: []*enginepb.Cell{{Value: &enginepb.Cell_StringValue{StringValue: "2"}}}}},
		},
	}
	cfg := &jobs.JobConfigurationCopy{
		SourceTables: []bqtypes.TableReference{
			{ProjectID: "dev", DatasetID: "ds", TableID: "src1"},
			{ProjectID: "dev", DatasetID: "ds", TableID: "src2"},
		},
		DestinationTable: &bqtypes.TableReference{ProjectID: "dev", DatasetID: "ds", TableID: "dest"},
		WriteDisposition: "WRITE_EMPTY",
	}
	result, err := copy.Execute(context.Background(), cat, nil, nil, cfg, "dev")
	if err != nil {
		t.Fatalf("Execute: %v", err)
	}
	if result.CopiedRows != 2 {
		t.Fatalf("CopiedRows = %d, want 2", result.CopiedRows)
	}
	if len(cat.rows["dest"]) != 2 {
		t.Fatalf("dest rows = %d, want 2", len(cat.rows["dest"]))
	}
}

func TestExecuteCopyCreateNeverRequiresDestination(t *testing.T) {
	t.Parallel()
	cat := &multiSourceCatalog{tables: map[string]*enginepb.TableSchema{}, rows: map[string][]*enginepb.DataRow{}}
	cfg := &jobs.JobConfigurationCopy{
		SourceTables: []bqtypes.TableReference{
			{ProjectID: "dev", DatasetID: "ds", TableID: "src1"},
		},
		DestinationTable:  &bqtypes.TableReference{ProjectID: "dev", DatasetID: "ds", TableID: "dest"},
		CreateDisposition: "CREATE_NEVER",
		WriteDisposition:  "WRITE_TRUNCATE",
	}
	_, err := copy.Execute(context.Background(), cat, nil, nil, cfg, "dev")
	if err == nil {
		t.Fatal("expected error when destination missing with CREATE_NEVER")
	}
}
