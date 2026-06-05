package external

import (
	"context"
	"encoding/json"
	"errors"
	"net/http"
	"net/http/httptest"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

const (
	testExtColType   = "STRING"
	testExtTableName = "us_states"
)

type materializeFakeCatalog struct {
	lastRegisterTable *enginepb.RegisterTableRequest
	lastInsertRows    *enginepb.InsertRowsRequest
}

func (f *materializeFakeCatalog) RegisterDataset(
	_ context.Context,
	_ *enginepb.RegisterDatasetRequest,
	_ ...grpc.CallOption,
) (*enginepb.RegisterDatasetResponse, error) {
	return &enginepb.RegisterDatasetResponse{}, nil
}

func (f *materializeFakeCatalog) DropDataset(
	_ context.Context,
	_ *enginepb.DropDatasetRequest,
	_ ...grpc.CallOption,
) (*enginepb.DropDatasetResponse, error) {
	return &enginepb.DropDatasetResponse{}, nil
}

func (f *materializeFakeCatalog) RegisterTable(
	_ context.Context,
	in *enginepb.RegisterTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.RegisterTableResponse, error) {
	f.lastRegisterTable = in
	return &enginepb.RegisterTableResponse{}, nil
}

func (f *materializeFakeCatalog) DropTable(
	_ context.Context,
	_ *enginepb.DropTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.DropTableResponse, error) {
	return &enginepb.DropTableResponse{}, nil
}

func (f *materializeFakeCatalog) DescribeTable(
	_ context.Context,
	_ *enginepb.DescribeTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.DescribeTableResponse, error) {
	return nil, status.Error(codes.NotFound, "table not found")
}

func (f *materializeFakeCatalog) InsertRows(
	_ context.Context,
	in *enginepb.InsertRowsRequest,
	_ ...grpc.CallOption,
) (*enginepb.InsertRowsResponse, error) {
	f.lastInsertRows = in
	return &enginepb.InsertRowsResponse{}, nil
}

func (f *materializeFakeCatalog) ListRows(
	_ context.Context,
	_ *enginepb.ListRowsRequest,
	_ ...grpc.CallOption,
) (*enginepb.ListRowsResponse, error) {
	return &enginepb.ListRowsResponse{}, nil
}

func (f *materializeFakeCatalog) ListDatasets(
	_ context.Context,
	_ *enginepb.ListDatasetsRequest,
	_ ...grpc.CallOption,
) (*enginepb.ListDatasetsResponse, error) {
	return &enginepb.ListDatasetsResponse{}, nil
}

func (f *materializeFakeCatalog) ListTables(
	_ context.Context,
	_ *enginepb.ListTablesRequest,
	_ ...grpc.CallOption,
) (*enginepb.ListTablesResponse, error) {
	return &enginepb.ListTablesResponse{}, nil
}

func TestExternalMaterializeCSVFromFakeGCS(t *testing.T) {
	const csvBody = "name,post_abbr\nWashington,WA\nWyoming,WY\nWisconsin,WI\nWest Virginia,WV\n"
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, _ *http.Request) {
		w.Header().Set("Content-Type", "text/csv")
		_, _ = w.Write([]byte(csvBody))
	}))
	defer srv.Close()

	t.Setenv("STORAGE_EMULATOR_HOST", srv.Listener.Addr().String())

	fake := &materializeFakeCatalog{}
	ctx := context.Background()
	var csvOpts bqtypes.CsvOptions
	if err := json.Unmarshal([]byte(`{"skipLeadingRows":"1"}`), &csvOpts); err != nil {
		t.Fatal(err)
	}
	cfg := &bqtypes.ExternalDataConfiguration{
		SourceFormat: "CSV",
		SourceURIs:   []string{"gs://cloud-samples-data/bigquery/us-states/us-states.csv"},
		CsvOptions:   &csvOpts,
		Schema: &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
			{Name: "name", Type: testExtColType},
			{Name: "post_abbr", Type: testExtColType},
		}},
	}
	if err := Materialize(ctx, fake, Target{
		ProjectID: "dev",
		DatasetID: "ds",
		TableID:   testExtTableName,
	}, cfg); err != nil {
		t.Fatalf("Materialize: %v", err)
	}
	if fake.lastRegisterTable == nil {
		t.Fatal("RegisterTable not called")
	}
	if got := fake.lastRegisterTable.GetTable().GetTableId(); got != testExtTableName {
		t.Fatalf("table id = %q, want %s", got, testExtTableName)
	}
	if fake.lastInsertRows == nil || len(fake.lastInsertRows.GetRows()) != 4 {
		t.Fatalf("insert rows = %d, want 4", len(fake.lastInsertRows.GetRows()))
	}
}

func TestExternalGoogleSheetsUnsupported(t *testing.T) {
	fake := &materializeFakeCatalog{}
	cfg := &bqtypes.ExternalDataConfiguration{
		SourceFormat: "GOOGLE_SHEETS",
		SourceURIs:   []string{"https://docs.google.com/spreadsheets/d/abc/edit"},
	}
	err := Materialize(context.Background(), fake, Target{
		ProjectID: "dev",
		DatasetID: "ds",
		TableID:   "sheet",
	}, cfg)
	if !errors.Is(err, ErrGoogleSheetsUnsupported) {
		t.Fatalf("err = %v, want ErrGoogleSheetsUnsupported", err)
	}
}

func TestExternalPrepareTableDefinitionsSetsTempDataset(t *testing.T) {
	const csvBody = "name,post_abbr\nWashington,WA\n"
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, _ *http.Request) {
		_, _ = w.Write([]byte(csvBody))
	}))
	defer srv.Close()
	t.Setenv("STORAGE_EMULATOR_HOST", srv.Listener.Addr().String())

	fake := &materializeFakeCatalog{}
	var skipOpts bqtypes.CsvOptions
	if err := json.Unmarshal([]byte(`{"skipLeadingRows":"1"}`), &skipOpts); err != nil {
		t.Fatal(err)
	}
	defs := map[string]bqtypes.ExternalDataConfiguration{
		testExtTableName: {
			SourceFormat: "CSV",
			SourceURIs:   []string{"gs://bkt/obj.csv"},
			Schema: &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
				{Name: "name", Type: testExtColType},
				{Name: "post_abbr", Type: testExtColType},
			}},
			CsvOptions: &skipOpts,
		},
	}
	ds, err := PrepareTableDefinitions(context.Background(), fake, "dev", defs, "")
	if err != nil {
		t.Fatalf("PrepareTableDefinitions: %v", err)
	}
	if ds != TempDatasetID {
		t.Fatalf("default dataset = %q, want %q", ds, TempDatasetID)
	}
	if fake.lastRegisterTable.GetTable().GetDatasetId() != TempDatasetID {
		t.Fatalf("registered dataset = %q", fake.lastRegisterTable.GetTable().GetDatasetId())
	}
}
