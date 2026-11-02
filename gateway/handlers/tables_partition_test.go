package handlers

import (
	"context"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"github.com/vantaboard/bigquery-emulator/gateway/load"
)

const hourPartition = "HOUR"

func TestTableInsertForwardsIngestionPartitionType(t *testing.T) {
	cases := []struct {
		name, timePartitioning, want string
	}{
		{"not partitioned", ``, ""},
		{"ingestion time, typed", `,"timePartitioning":{"type":"HOUR"}`, hourPartition},
		{"ingestion time, untyped", `,"timePartitioning":{}`, "DAY"},
		{"column", `,"timePartitioning":{"type":"DAY","field":"ts"}`, ""},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			fake := &fakeCatalogClient{}
			body := `{"tableReference":{"tableId":"t1"},` +
				`"schema":{"fields":[{"name":"ts","type":"TIMESTAMP"}]}` + tc.timePartitioning + `}`
			rec := httptest.NewRecorder()
			TableInsert(Dependencies{Catalog: fake})(rec, newTableReq(http.MethodPost, "", body))

			if rec.Code != http.StatusOK {
				t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
			}
			if got := fake.lastRegisterTable.GetSchema().GetIngestionPartitionType(); got != tc.want {
				t.Errorf("ingestion_partition_type = %q, want %q", got, tc.want)
			}
		})
	}
}

func TestJobInsertLoadRegistersIngestionPartitionType(t *testing.T) {
	csvPath := filepath.Join(t.TempDir(), "rows.csv")
	if err := os.WriteFile(csvPath, []byte("a\n"), 0o644); err != nil {
		t.Fatal(err)
	}
	cat := &fakeCatalogClient{}
	body := `{"configuration":{"load":{"sourceUris":["file://` + csvPath + `"],` +
		`"destinationTable":{"projectId":"dev","datasetId":"ds","tableId":"t"},` +
		`"sourceFormat":"CSV","writeDisposition":"WRITE_TRUNCATE","timePartitioning":{"type":"HOUR"},` +
		`"schema":{"fields":[{"name":"name","type":"` + sqlTypeSTRING + `"}]}}}}`

	rec := runJobInsert(t, Dependencies{Catalog: cat, Jobs: jobs.NewRegistry()}, body)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, body=%s", rec.Code, rec.Body.String())
	}
	if got := cat.lastRegisterTable.GetSchema().GetIngestionPartitionType(); got != hourPartition {
		t.Errorf("ingestion_partition_type = %q, want HOUR", got)
	}
}

func describeAs(partitionType string) func(context.Context, *enginepb.DescribeTableRequest) (
	*enginepb.DescribeTableResponse, error,
) {
	return func(context.Context, *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
		return &enginepb.DescribeTableResponse{Schema: &enginepb.TableSchema{
			Fields:                 []*enginepb.FieldSchema{{Name: "name", Type: sqlTypeSTRING}},
			IngestionPartitionType: partitionType,
		}}, nil
	}
}

func TestTableDataInsertAllWritesIntoTheDecoratedPartition(t *testing.T) {
	fake := &fakeCatalogClient{describeTableFn: describeAs("DAY")}
	req := newTabledataReq(http.MethodPost, testTableID+"$20240304", "insertAll", `{"rows":[{"json":{"name":"a"}}]}`)
	rec := httptest.NewRecorder()
	TableDataInsertAll(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	if got := fake.lastDescribeTable.GetTable().GetTableId(); got != testTableID {
		t.Errorf("described table %q, want %q", got, testTableID)
	}
	if got := fake.lastInsertRows.GetTable().GetTableId(); got != testTableID {
		t.Errorf("inserted into table %q, want %q", got, testTableID)
	}
	if got := fake.lastInsertRows.GetRows()[0].GetPartitionTime(); got != "2024-03-04 00:00:00+00" {
		t.Errorf("partition time = %q, want the 2024-03-04 partition", got)
	}
}

func TestTableDataInsertAllRejectsAMismatchedDecorator(t *testing.T) {
	for _, partitionType := range []string{"", hourPartition} {
		fake := &fakeCatalogClient{describeTableFn: describeAs(partitionType)}
		req := newTabledataReq(
			http.MethodPost,
			testTableID+"$20240304",
			"insertAll",
			`{"rows":[{"json":{"name":"a"}}]}`,
		)
		rec := httptest.NewRecorder()
		TableDataInsertAll(Dependencies{Catalog: fake})(rec, req)

		if rec.Code != http.StatusBadRequest || fake.lastInsertRows != nil {
			t.Errorf("%q table: status = %d, inserted = %v; want 400 and no insert",
				partitionType, rec.Code, fake.lastInsertRows != nil)
		}
	}
}

func TestJobInsertLoadWritesIntoTheDecoratedPartition(t *testing.T) {
	csvPath := filepath.Join(t.TempDir(), "rows.csv")
	if err := os.WriteFile(csvPath, []byte("a\n"), 0o644); err != nil {
		t.Fatal(err)
	}
	load := func(writeDisposition string) (*fakeCatalogClient, jobs.Job) {
		cat := &fakeCatalogClient{describeTableFn: describeAs("DAY")}
		body := `{"configuration":{"load":{"sourceUris":["file://` + csvPath + `"],` +
			`"destinationTable":{"projectId":"dev","datasetId":"ds","tableId":"t$20240304"},` +
			`"sourceFormat":"CSV","writeDisposition":"` + writeDisposition + `",` +
			`"schema":{"fields":[{"name":"name","type":"` + sqlTypeSTRING + `"}]}}}}`
		rec := runJobInsert(t, Dependencies{Catalog: cat, Jobs: jobs.NewRegistry()}, body)
		var job jobs.Job
		if err := json.NewDecoder(rec.Body).Decode(&job); err != nil {
			t.Fatalf("decode: %v", err)
		}
		return cat, job
	}

	cat, job := load("WRITE_APPEND")
	if job.Status.ErrorResult != nil {
		t.Fatalf("errorResult = %#v", job.Status.ErrorResult)
	}
	if got := cat.lastInsertRows.GetTable().GetTableId(); got != "t" {
		t.Errorf("inserted into table %q, want t", got)
	}
	if got := cat.lastInsertRows.GetRows()[0].GetPartitionTime(); got != "2024-03-04 00:00:00+00" {
		t.Errorf("partition time = %q, want the 2024-03-04 partition", got)
	}

	cat, job = load("WRITE_TRUNCATE")
	if job.Status.ErrorResult == nil || cat.lastDropTable != nil {
		t.Errorf("WRITE_TRUNCATE into a partition: errorResult = %v, dropped = %v; want an error and no drop",
			job.Status.ErrorResult, cat.lastDropTable != nil)
	}
}

func TestSchemaUpdateKeepsIngestionPartitions(t *testing.T) {
	const partition = "2024-03-04 00:00:00+00"
	cat := &fakeCatalogClient{
		describeTableFn: describeAs("DAY"),
		listRowsFn: func(context.Context, *enginepb.ListRowsRequest) (*enginepb.ListRowsResponse, error) {
			return &enginepb.ListRowsResponse{TotalRows: 1, Rows: []*enginepb.DataRow{{
				Cells:         []*enginepb.Cell{{Value: &enginepb.Cell_StringValue{StringValue: "a"}}},
				PartitionTime: partition,
			}}}, nil
		},
	}
	added := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "name", Type: sqlTypeSTRING},
		{Name: "extra", Type: sqlTypeSTRING},
	}}
	ref := &enginepb.TableRef{ProjectId: testProjectID, DatasetId: testDatasetID, TableId: testTableID}

	if _, err := load.ApplySchemaUpdate(
		context.Background(),
		cat,
		ref,
		added,
		load.TablePatchSchemaOptions,
	); err != nil {
		t.Fatalf("ApplySchemaUpdate: %v", err)
	}
	if got := cat.lastRegisterTable.GetSchema().GetIngestionPartitionType(); got != "DAY" {
		t.Errorf("re-registered partition type = %q, want DAY", got)
	}
	if got := cat.lastInsertRows.GetRows()[0].GetPartitionTime(); got != partition {
		t.Errorf("re-inserted partition time = %q, want %q", got, partition)
	}
}
