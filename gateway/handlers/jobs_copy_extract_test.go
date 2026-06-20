package handlers

import (
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"github.com/vantaboard/bigquery-emulator/gateway/load"
	"github.com/vantaboard/bigquery-emulator/gateway/snapshots"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

func TestJobInsertCopySingleSource(t *testing.T) {
	t.Parallel()
	schema := &enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: "id", Type: sqlTypeINTEGER},
		{Name: testColumnName, Type: sqlTypeSTRING},
	}}
	srcRows := []*enginepb.DataRow{
		{Cells: []*enginepb.Cell{
			{Value: &enginepb.Cell_StringValue{StringValue: "1"}},
			{Value: &enginepb.Cell_StringValue{StringValue: "alice"}},
		}},
	}
	cat := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, in *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			if in.GetTable().GetTableId() == "dst" {
				return nil, status.Error(codes.NotFound, "table not found")
			}
			return &enginepb.DescribeTableResponse{Schema: schema}, nil
		},
		listRowsFn: func(_ context.Context, in *enginepb.ListRowsRequest) (*enginepb.ListRowsResponse, error) {
			if in.GetTable().GetTableId() == "src" {
				return &enginepb.ListRowsResponse{TotalRows: 1, Rows: srcRows}, nil
			}
			return &enginepb.ListRowsResponse{TotalRows: 0}, nil
		},
	}
	reg := jobs.NewRegistry()
	deps := Dependencies{Catalog: cat, Jobs: reg, Snapshots: snapshots.NewStore()}
	body := `{"configuration":{"copy":{"sourceTable":{"projectId":"dev","datasetId":"ds","tableId":"src"},` +
		`"destinationTable":{"projectId":"dev","datasetId":"ds","tableId":"dst"},"writeDisposition":"WRITE_EMPTY"}}}`

	rec := runJobInsert(t, deps, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, body=%s", rec.Code, rec.Body.String())
	}
	var got jobs.Job
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Status.ErrorResult != nil {
		t.Fatalf("errorResult = %#v", got.Status.ErrorResult)
	}
	if got.Statistics.Copy == nil || got.Statistics.Copy.CopiedRows != "1" {
		t.Fatalf("statistics.copy = %#v", got.Statistics.Copy)
	}
	if cat.lastInsertRows == nil || len(cat.lastInsertRows.GetRows()) != 1 {
		t.Fatalf("InsertRows not called for destination")
	}
}

func TestJobInsertCopyMultipleSources(t *testing.T) {
	t.Parallel()
	schema := &enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: "id", Type: sqlTypeINTEGER},
	}}
	row := func(v string) *enginepb.DataRow {
		return &enginepb.DataRow{Cells: []*enginepb.Cell{
			{Value: &enginepb.Cell_StringValue{StringValue: v}},
		}}
	}
	cat := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, in *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			if in.GetTable().GetTableId() == "dest" {
				return nil, status.Error(codes.NotFound, "table not found")
			}
			return &enginepb.DescribeTableResponse{Schema: schema}, nil
		},
		listRowsFn: func(_ context.Context, in *enginepb.ListRowsRequest) (*enginepb.ListRowsResponse, error) {
			switch in.GetTable().GetTableId() {
			case "src1":
				return &enginepb.ListRowsResponse{TotalRows: 1, Rows: []*enginepb.DataRow{row("1")}}, nil
			case "src2":
				return &enginepb.ListRowsResponse{TotalRows: 1, Rows: []*enginepb.DataRow{row("2")}}, nil
			default:
				return &enginepb.ListRowsResponse{TotalRows: 0}, nil
			}
		},
	}
	reg := jobs.NewRegistry()
	deps := Dependencies{Catalog: cat, Jobs: reg, Snapshots: snapshots.NewStore()}
	body := `{"configuration":{"copy":{"sourceTables":[` +
		`{"projectId":"dev","datasetId":"ds","tableId":"src1"},` +
		`{"projectId":"dev","datasetId":"ds","tableId":"src2"}],` +
		`"destinationTable":{"projectId":"dev","datasetId":"ds","tableId":"dest"}}}}`

	rec := runJobInsert(t, deps, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, body=%s", rec.Code, rec.Body.String())
	}
	var got jobs.Job
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Status.ErrorResult != nil {
		t.Fatalf("errorResult = %#v", got.Status.ErrorResult)
	}
	if got.Statistics.Copy == nil || got.Statistics.Copy.CopiedRows != "2" {
		t.Fatalf("statistics.copy = %#v", got.Statistics.Copy)
	}
}

func TestJobInsertCopyFromSnapshotUndelete(t *testing.T) {
	t.Parallel()
	schema := &enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: "word", Type: sqlTypeSTRING},
	}}
	rows := []*enginepb.DataRow{
		{Cells: []*enginepb.Cell{{Value: &enginepb.Cell_StringValue{StringValue: "recoverable"}}}},
	}
	snapStore := snapshots.NewStore()
	snapStore.RecordCreation("dev", "ds", "gone", 1_700_000_000_000)
	liveCat := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, _ *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			return &enginepb.DescribeTableResponse{Schema: schema}, nil
		},
		listRowsFn: func(_ context.Context, _ *enginepb.ListRowsRequest) (*enginepb.ListRowsResponse, error) {
			return &enginepb.ListRowsResponse{TotalRows: 1, Rows: rows}, nil
		},
	}
	if err := snapStore.CaptureBeforeDelete(context.Background(), liveCat, "dev", "ds", "gone"); err != nil {
		t.Fatal(err)
	}

	cat := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, in *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			if in.GetTable().GetTableId() == "recovered" {
				return nil, status.Error(codes.NotFound, "table not found")
			}
			return &enginepb.DescribeTableResponse{Schema: schema}, nil
		},
	}
	reg := jobs.NewRegistry()
	deps := Dependencies{Catalog: cat, Jobs: reg, Snapshots: snapStore}
	body := `{"configuration":{"copy":{"sourceTable":{"projectId":"dev","datasetId":"ds","tableId":"gone@1700000000000"},` +
		`"destinationTable":{"projectId":"dev","datasetId":"ds","tableId":"recovered"}}}}`

	rec := runJobInsert(t, deps, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, body=%s", rec.Code, rec.Body.String())
	}
	var got jobs.Job
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Status.ErrorResult != nil {
		t.Fatalf("errorResult = %#v", got.Status.ErrorResult)
	}
	if got.Statistics.Copy == nil || got.Statistics.Copy.CopiedRows != "1" {
		t.Fatalf("statistics.copy = %#v", got.Statistics.Copy)
	}
}

func TestJobInsertCopySnapshotWithExpiration(t *testing.T) {
	t.Parallel()
	schema := &enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: "id", Type: sqlTypeINTEGER},
	}}
	srcRows := []*enginepb.DataRow{
		{Cells: []*enginepb.Cell{
			{Value: &enginepb.Cell_StringValue{StringValue: "42"}},
		}},
	}
	cat := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, in *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			if in.GetTable().GetTableId() == "snap_dst" {
				return nil, status.Error(codes.NotFound, "table not found")
			}
			return &enginepb.DescribeTableResponse{Schema: schema}, nil
		},
		listRowsFn: func(_ context.Context, in *enginepb.ListRowsRequest) (*enginepb.ListRowsResponse, error) {
			if in.GetTable().GetTableId() == "src" {
				return &enginepb.ListRowsResponse{TotalRows: 1, Rows: srcRows}, nil
			}
			return &enginepb.ListRowsResponse{TotalRows: 0}, nil
		},
	}
	meta := NewMetadataStore()
	reg := jobs.NewRegistry()
	deps := Dependencies{Catalog: cat, Jobs: reg, Snapshots: snapshots.NewStore(), Metadata: meta}
	body := `{"configuration":{"copy":{"sourceTable":{"projectId":"dev","datasetId":"ds","tableId":"src"},` +
		`"destinationTable":{"projectId":"dev","datasetId":"ds","tableId":"snap_dst"},` +
		`"operationType":"SNAPSHOT","destinationExpirationTime":"1990000000000"}}}`

	rec := runJobInsert(t, deps, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, body=%s", rec.Code, rec.Body.String())
	}
	var got jobs.Job
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Status.ErrorResult != nil {
		t.Fatalf("errorResult = %#v", got.Status.ErrorResult)
	}
	overlay, ok := meta.GetTable("dev", "ds", "snap_dst")
	if !ok {
		t.Fatal("metadata overlay missing for snapshot destination")
	}
	if overlay.Type != snapshotTableType {
		t.Errorf("type = %q, want %q", overlay.Type, snapshotTableType)
	}
	if overlay.ExpirationTime.String() != "1990000000000" {
		t.Errorf("expirationTime = %q, want %q", overlay.ExpirationTime, "1990000000000")
	}
}

func TestJobInsertCopyRestoreFromSnapshot(t *testing.T) {
	t.Parallel()
	schema := &enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: "word", Type: sqlTypeSTRING},
	}}
	rows := []*enginepb.DataRow{
		{Cells: []*enginepb.Cell{{Value: &enginepb.Cell_StringValue{StringValue: "snap_data"}}}},
	}
	meta := NewMetadataStore()
	meta.MergeTable("dev", "ds", "snap_src", bqtypes.Table{Type: snapshotTableType})
	cat := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, in *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			if in.GetTable().GetTableId() == "restored" {
				return nil, status.Error(codes.NotFound, "table not found")
			}
			return &enginepb.DescribeTableResponse{Schema: schema}, nil
		},
		listRowsFn: func(_ context.Context, in *enginepb.ListRowsRequest) (*enginepb.ListRowsResponse, error) {
			if in.GetTable().GetTableId() == "snap_src" {
				return &enginepb.ListRowsResponse{TotalRows: 1, Rows: rows}, nil
			}
			return &enginepb.ListRowsResponse{TotalRows: 0}, nil
		},
	}
	reg := jobs.NewRegistry()
	deps := Dependencies{Catalog: cat, Jobs: reg, Snapshots: snapshots.NewStore(), Metadata: meta}
	body := `{"configuration":{"copy":{"sourceTable":{"projectId":"dev","datasetId":"ds","tableId":"snap_src"},` +
		`"destinationTable":{"projectId":"dev","datasetId":"ds","tableId":"restored"},` +
		`"operationType":"RESTORE","writeDisposition":"WRITE_EMPTY"}}}`

	rec := runJobInsert(t, deps, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, body=%s", rec.Code, rec.Body.String())
	}
	var got jobs.Job
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Status.ErrorResult != nil {
		t.Fatalf("errorResult = %#v", got.Status.ErrorResult)
	}
	if got.Statistics.Copy == nil || got.Statistics.Copy.CopiedRows != "1" {
		t.Fatalf("statistics.copy = %#v", got.Statistics.Copy)
	}
	overlay, ok := meta.GetTable("dev", "ds", "restored")
	if !ok {
		t.Fatal("metadata overlay missing for restore destination")
	}
	if overlay.Type != defaultTableType {
		t.Errorf("type = %q, want %q", overlay.Type, defaultTableType)
	}
}

func TestJobInsertCopyLiveTableTimeTravelUsesSQL(t *testing.T) {
	t.Parallel()
	schema := &enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: "id", Type: sqlTypeINTEGER},
	}}
	epoch := int64(1_700_000_000_000)
	cat := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, in *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			if in.GetTable().GetTableId() == "time_dst" {
				return nil, status.Error(codes.NotFound, "table not found")
			}
			return &enginepb.DescribeTableResponse{Schema: schema}, nil
		},
		listRowsFn: func(_ context.Context, in *enginepb.ListRowsRequest) (*enginepb.ListRowsResponse, error) {
			if in.GetTable().GetTableId() == "time_dst" {
				return &enginepb.ListRowsResponse{TotalRows: 3}, nil
			}
			return &enginepb.ListRowsResponse{TotalRows: 0}, nil
		},
	}
	fakeQuery := &fakeQueryClient{
		executeQueryFn: func(_ context.Context, in *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			want := fmt.Sprintf("FOR SYSTEM_TIME AS OF TIMESTAMP_MILLIS(%d)", epoch)
			if !strings.Contains(in.GetSql(), want) {
				return nil, fmt.Errorf("sql missing time travel: %q", in.GetSql())
			}
			return &fakeQueryResultStream{}, nil
		},
	}
	reg := jobs.NewRegistry()
	deps := Dependencies{
		Catalog:   cat,
		Query:     fakeQuery,
		Jobs:      reg,
		Snapshots: snapshots.NewStore(),
	}
	body := fmt.Sprintf(
		`{"configuration":{"copy":{"sourceTable":{"projectId":"dev","datasetId":"ds","tableId":"live@%d"},`+
			`"destinationTable":{"projectId":"dev","datasetId":"ds","tableId":"time_dst"},`+
			`"operationType":"SNAPSHOT","writeDisposition":"WRITE_EMPTY"}}}`,
		epoch,
	)

	rec := runJobInsert(t, deps, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, body=%s", rec.Code, rec.Body.String())
	}
	var got jobs.Job
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Status.ErrorResult != nil {
		t.Fatalf("errorResult = %#v", got.Status.ErrorResult)
	}
	if got.Statistics.Copy == nil || got.Statistics.Copy.CopiedRows != "3" {
		t.Fatalf("statistics.copy = %#v", got.Statistics.Copy)
	}
}

func TestJobInsertExtractCSVToGCS(t *testing.T) {
	var uploaded []byte
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if r.Method != http.MethodPost {
			http.NotFound(w, r)
			return
		}
		body := make([]byte, r.ContentLength)
		_, _ = r.Body.Read(body)
		uploaded = body
		w.WriteHeader(http.StatusOK)
	}))
	defer srv.Close()
	t.Setenv("STORAGE_EMULATOR_HOST", srv.URL)

	schema := &enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: testColumnName, Type: sqlTypeSTRING},
	}}
	rows := []*enginepb.DataRow{
		{Cells: []*enginepb.Cell{{Value: &enginepb.Cell_StringValue{StringValue: "test"}}}},
	}
	cat := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, _ *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			return &enginepb.DescribeTableResponse{Schema: schema}, nil
		},
		listRowsFn: func(_ context.Context, _ *enginepb.ListRowsRequest) (*enginepb.ListRowsResponse, error) {
			return &enginepb.ListRowsResponse{TotalRows: 1, Rows: rows}, nil
		},
	}
	reg := jobs.NewRegistry()
	deps := Dependencies{Catalog: cat, Jobs: reg}
	body := `{"configuration":{"extract":{"sourceTable":{"projectId":"dev","datasetId":"ds","tableId":"t"},` +
		`"destinationUris":["gs://bucket/out.csv"],"destinationFormat":"CSV"}}}`

	rec := runJobInsert(t, deps, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, body=%s", rec.Code, rec.Body.String())
	}
	var got jobs.Job
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Status.ErrorResult != nil {
		t.Fatalf("errorResult = %#v", got.Status.ErrorResult)
	}
	if got.Statistics.Extract == nil || got.Statistics.Extract.DestinationURIFileCounts[0] != "1" {
		t.Fatalf("statistics.extract = %#v", got.Statistics.Extract)
	}
	if len(uploaded) == 0 {
		t.Fatal("expected GCS upload")
	}
	if string(uploaded) != "name\ntest\n" {
		t.Fatalf("uploaded CSV = %q", string(uploaded))
	}
}

func TestJobInsertExtractWithoutCatalogDeferred(t *testing.T) {
	t.Parallel()
	body := `{"configuration":{"extract":{"sourceTable":{"projectId":"dev","datasetId":"ds","tableId":"t"},` +
		`"destinationUris":["gs://b/o.json"],"destinationFormat":"NEWLINE_DELIMITED_JSON"}}}`
	rec := runJobInsert(t, Dependencies{Jobs: jobs.NewRegistry()}, body)
	var got jobs.Job
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Status.ErrorResult == nil {
		t.Fatal("expected deferred error when Catalog is nil")
	}
}

func TestPutGCSRoundTrip(t *testing.T) {
	var gotURI string
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		gotURI = r.URL.String()
		w.WriteHeader(http.StatusOK)
	}))
	defer srv.Close()
	t.Setenv("STORAGE_EMULATOR_HOST", srv.URL)
	if err := load.PutGCS(context.Background(), "gs://my-bucket/path/file.csv", "text/csv", []byte("a\n")); err != nil {
		t.Fatalf("PutGCS: %v", err)
	}
	for _, part := range []string{"my-bucket", "path%2Ffile.csv", "uploadType=media"} {
		if !strings.Contains(gotURI, part) {
			t.Fatalf("upload URL = %q, missing %q", gotURI, part)
		}
	}
}
