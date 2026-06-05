package handlers

import (
	"context"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
)

func TestJobInsertLoadFromLocalCSV(t *testing.T) {
	t.Parallel()
	dir := t.TempDir()
	csvPath := filepath.Join(dir, "states.csv")
	csvBody := "name,post_abbr\nAlabama,AL\nAlaska,AK\n"
	if err := os.WriteFile(csvPath, []byte(csvBody), 0o644); err != nil {
		t.Fatal(err)
	}

	cat := &fakeCatalogClient{}
	reg := jobs.NewRegistry()
	deps := Dependencies{Catalog: cat, Jobs: reg}
	body := `{"configuration":{"load":{"sourceUris":["file://` + csvPath + `"],` +
		`"destinationTable":{"projectId":"dev","datasetId":"ds","tableId":"states"},` +
		`"sourceFormat":"CSV","skipLeadingRows":"1",` +
		`"schema":{"fields":[{"name":"name","type":"STRING"},{"name":"post_abbr","type":"STRING"}]}}}}`

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
	if got.Statistics.Load == nil || got.Statistics.Load.OutputRows != "2" {
		t.Fatalf("statistics.load = %#v", got.Statistics.Load)
	}
	if cat.lastInsertRows == nil || len(cat.lastInsertRows.Rows) != 2 {
		t.Fatalf("InsertRows rows = %d, want 2", len(cat.lastInsertRows.GetRows()))
	}
}

func TestJobConfigurationLoadSkipLeadingRowsString(t *testing.T) {
	t.Parallel()
	var cfg jobs.JobConfiguration
	if err := json.Unmarshal([]byte(`{"load":{"skipLeadingRows":"1"}}`), &cfg); err != nil {
		t.Fatalf("unmarshal: %v", err)
	}
	if cfg.Load.SkipLeadingRows() != 1 {
		t.Fatalf("skipLeadingRows = %d, want 1", cfg.Load.SkipLeadingRows())
	}
}

func TestJobInsertLoadWithoutCatalogStillDeferred(t *testing.T) {
	t.Parallel()
	reg := jobs.NewRegistry()
	body := `{"configuration":{"load":{"sourceUris":["gs://b/f.csv"],"destinationTable":{"projectId":"dev","datasetId":"ds","tableId":"t"}}}}`
	rec := runJobInsert(t, Dependencies{Jobs: reg}, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d", rec.Code)
	}
	var got jobs.Job
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Status.ErrorResult == nil {
		t.Fatal("expected deferred errorResult when Catalog is nil")
	}
}

func TestJobInsertUploadMultipartCSV(t *testing.T) {
	t.Parallel()
	cat := &fakeCatalogClient{}
	reg := jobs.NewRegistry()
	deps := Dependencies{Catalog: cat, Jobs: reg}
	meta := `{"configuration":{"load":{"destinationTable":{"projectId":"dev","datasetId":"ds","tableId":"people"},` +
		`"sourceFormat":"CSV","skipLeadingRows":"1","autodetect":true}}}`
	body := strings.Join([]string{
		"--BOUNDARY",
		"Content-Type: application/json; charset=UTF-8",
		"",
		meta,
		"--BOUNDARY",
		"Content-Type: */*",
		"",
		"full_name,age\nPhred Phlyntstone,32\n",
		"--BOUNDARY--",
		"",
	}, "\r\n")

	rec := httptest.NewRecorder()
	req := httptest.NewRequest(http.MethodPost,
		"/upload/bigquery/v2/projects/"+testProjectID+"/jobs?uploadType=multipart",
		strings.NewReader(body))
	req.Header.Set("Content-Type", `multipart/related; boundary="BOUNDARY"`)
	req.SetPathValue("projectId", testProjectID)
	JobInsertUpload(deps)(rec, req)

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
	if got.Statistics.Load == nil || got.Statistics.Load.OutputRows != "1" {
		t.Fatalf("statistics.load = %#v", got.Statistics.Load)
	}
}

func TestJobInsertUploadResumableCSV(t *testing.T) {
	t.Parallel()
	cat := &fakeCatalogClient{}
	reg := jobs.NewRegistry()
	deps := Dependencies{Catalog: cat, Jobs: reg}
	meta := `{"configuration":{"load":{"destinationTable":{"projectId":"dev","datasetId":"ds","tableId":"people"},` +
		`"sourceFormat":"CSV","skipLeadingRows":"1","autodetect":true}}}`

	initRec := httptest.NewRecorder()
	initReq := httptest.NewRequest(http.MethodPost,
		"/upload/bigquery/v2/projects/"+testProjectID+"/jobs?uploadType=resumable",
		strings.NewReader(meta))
	initReq.Header.Set("Content-Type", "application/json; charset=UTF-8")
	initReq.Header.Set("X-Upload-Content-Length", "40")
	initReq.SetPathValue("projectId", testProjectID)
	JobInsertUpload(deps)(initRec, initReq)
	if initRec.Code != http.StatusOK {
		t.Fatalf("init status = %d, body=%s", initRec.Code, initRec.Body.String())
	}
	loc := initRec.Header().Get("Location")
	if loc == "" || !strings.Contains(loc, "upload_id=") {
		t.Fatalf("Location = %q", loc)
	}

	uploadRec := httptest.NewRecorder()
	uploadReq := httptest.NewRequest(http.MethodPut, loc,
		strings.NewReader("full_name,age\nWylma Phlyntstone,29\n"))
	uploadReq.SetPathValue("projectId", testProjectID)
	JobInsertUpload(deps)(uploadRec, uploadReq)
	if uploadRec.Code != http.StatusOK {
		t.Fatalf("upload status = %d, body=%s", uploadRec.Code, uploadRec.Body.String())
	}
	var got jobs.Job
	if err := json.NewDecoder(uploadRec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Status.ErrorResult != nil {
		t.Fatalf("errorResult = %#v", got.Status.ErrorResult)
	}
}

func TestJobInsertLoadSchemaUpdateAllowFieldAddition(t *testing.T) {
	t.Parallel()
	existingSchema := &enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: "Name", Type: "STRING"},
		{Name: "Age", Type: "INTEGER"},
	}}
	cat := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, _ *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			return &enginepb.DescribeTableResponse{Schema: existingSchema}, nil
		},
		listRowsFn: func(_ context.Context, _ *enginepb.ListRowsRequest) (*enginepb.ListRowsResponse, error) {
			return &enginepb.ListRowsResponse{TotalRows: 0}, nil
		},
	}
	reg := jobs.NewRegistry()
	deps := Dependencies{Catalog: cat, Jobs: reg}
	dir := t.TempDir()
	csvPath := filepath.Join(dir, "append.csv")
	csv := "Name,Age,IsMagic\nBob,30,true\n"
	if err := os.WriteFile(csvPath, []byte(csv), 0o644); err != nil {
		t.Fatal(err)
	}
	body := `{"configuration":{"load":{"sourceUris":["file://` + csvPath + `"],` +
		`"destinationTable":{"projectId":"dev","datasetId":"ds","tableId":"t"},` +
		`"sourceFormat":"CSV","skipLeadingRows":"1","writeDisposition":"WRITE_APPEND",` +
		`"schemaUpdateOptions":["ALLOW_FIELD_ADDITION"],` +
		`"schema":{"fields":[{"name":"Name","type":"STRING"},{"name":"Age","type":"INTEGER"},{"name":"IsMagic","type":"BOOLEAN"}]}}}}`

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
	if cat.lastRegisterTable == nil || len(cat.lastRegisterTable.GetSchema().GetFields()) != 3 {
		t.Fatalf("register schema = %#v", cat.lastRegisterTable)
	}
}
