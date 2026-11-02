package handlers

import (
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
)

func TestTableInsertForwardsIngestionPartitionType(t *testing.T) {
	cases := []struct {
		name, timePartitioning, want string
	}{
		{"not partitioned", ``, ""},
		{"ingestion time, typed", `,"timePartitioning":{"type":"HOUR"}`, "HOUR"},
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
	if got := cat.lastRegisterTable.GetSchema().GetIngestionPartitionType(); got != "HOUR" {
		t.Errorf("ingestion_partition_type = %q, want HOUR", got)
	}
}
