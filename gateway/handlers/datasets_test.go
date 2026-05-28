package handlers

import (
	"context"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

// newDatasetReq builds an *http.Request with the path-value wildcards
// populated the way Go's mux populates them at runtime, so handlers
// that read PathValue("projectId") / PathValue("datasetId") see the
// expected values without going through the full mux.
func newDatasetReq(method, projectID, datasetID, body string) *http.Request {
	url := "/bigquery/v2/projects/" + projectID + "/datasets"
	if datasetID != "" {
		url += "/" + datasetID
	}
	var reqBody = strings.NewReader(body)
	req := httptest.NewRequest(method, url, reqBody)
	req.SetPathValue("projectId", projectID)
	if datasetID != "" {
		req.SetPathValue("datasetId", datasetID)
	}
	if body != "" {
		req.Header.Set("Content-Type", "application/json")
	}
	return req
}

// TestDatasetInsertForwardsToCatalog verifies the request body is
// decoded, the projectId is taken from the URL, and the engine RPC
// is dispatched with the captured values. The response carries the
// stamped Dataset resource fields (kind, id, reference, timestamps).
func TestDatasetInsertForwardsToCatalog(t *testing.T) {
	fake := &fakeCatalogClient{}
	deps := Dependencies{Catalog: fake}

	req := newDatasetReq(http.MethodPost, "proj", "",
		`{"datasetReference":{"datasetId":"ds1"},"location":"US","friendlyName":"hello"}`)
	rec := httptest.NewRecorder()
	DatasetInsert(deps)(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	if fake.lastRegisterDataset == nil {
		t.Fatal("Catalog.RegisterDataset was not called")
	}
	if got := fake.lastRegisterDataset.GetDataset().GetProjectId(); got != "proj" {
		t.Errorf("project_id forwarded = %q, want %q", got, "proj")
	}
	if got := fake.lastRegisterDataset.GetDataset().GetDatasetId(); got != "ds1" {
		t.Errorf("dataset_id forwarded = %q, want %q", got, "ds1")
	}
	if got := fake.lastRegisterDataset.GetLocation(); got != "US" {
		t.Errorf("location forwarded = %q, want %q", got, "US")
	}

	var ds bqtypes.Dataset
	if err := json.NewDecoder(rec.Body).Decode(&ds); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if ds.Kind != "bigquery#dataset" {
		t.Errorf("kind = %q, want %q", ds.Kind, "bigquery#dataset")
	}
	if ds.ID != "proj:ds1" {
		t.Errorf("id = %q, want %q", ds.ID, "proj:ds1")
	}
	if ds.DatasetReference.ProjectID != "proj" || ds.DatasetReference.DatasetID != "ds1" {
		t.Errorf("datasetReference = %+v, want {proj, ds1}", ds.DatasetReference)
	}
	if ds.FriendlyName != "hello" {
		t.Errorf("friendlyName = %q, want %q (caller-supplied metadata must round-trip)",
			ds.FriendlyName, "hello")
	}
	if ds.CreationTime == "" || ds.LastModifiedTime == "" {
		t.Errorf("timestamps not stamped: creation=%q lastModified=%q",
			ds.CreationTime, ds.LastModifiedTime)
	}
}

// TestDatasetInsertRequiresDatasetID asserts a missing
// datasetReference.datasetId is rejected with 400 invalid before any
// RPC is issued.
func TestDatasetInsertRequiresDatasetID(t *testing.T) {
	fake := &fakeCatalogClient{}
	req := newDatasetReq(http.MethodPost, "proj", "", `{"location":"US"}`)
	rec := httptest.NewRecorder()
	DatasetInsert(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusBadRequest {
		t.Fatalf("status = %d, want 400; body=%s", rec.Code, rec.Body.String())
	}
	if fake.lastRegisterDataset != nil {
		t.Fatal("Catalog.RegisterDataset must not be called when datasetId is missing")
	}
}

// TestDatasetInsertEnginePropagatesAlreadyExists pins the gRPC→HTTP
// mapping: an ALREADY_EXISTS from the engine surfaces as 409
// duplicate so client libraries can dedupe correctly.
func TestDatasetInsertEnginePropagatesAlreadyExists(t *testing.T) {
	fake := &fakeCatalogClient{
		registerDatasetFn: func(_ context.Context, _ *enginepb.RegisterDatasetRequest) (*enginepb.RegisterDatasetResponse, error) {
			return nil, status.Error(codes.AlreadyExists, "dataset already exists")
		},
	}
	req := newDatasetReq(http.MethodPost, "proj", "",
		`{"datasetReference":{"datasetId":"ds1"}}`)
	rec := httptest.NewRecorder()
	DatasetInsert(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusConflict {
		t.Fatalf("status = %d, want %d", rec.Code, http.StatusConflict)
	}
	var env errorEnvelope
	if err := json.NewDecoder(rec.Body).Decode(&env); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if env.Error.Status != "duplicate" {
		t.Errorf("error.status = %q, want %q", env.Error.Status, "duplicate")
	}
}

// TestDatasetInsertWithoutCatalogFallsBackTo501 documents the
// nil-Catalog behaviour so the route table tests keep working in
// pure-Phase-1 mode.
func TestDatasetInsertWithoutCatalogFallsBackTo501(t *testing.T) {
	req := newDatasetReq(http.MethodPost, "proj", "",
		`{"datasetReference":{"datasetId":"ds1"}}`)
	rec := httptest.NewRecorder()
	DatasetInsert(Dependencies{})(rec, req)

	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want %d (deps.Catalog==nil must degrade gracefully)",
			rec.Code, http.StatusNotImplemented)
	}
}

// TestDatasetDeleteForwardsDeleteContents asserts the documented
// `deleteContents=true` query parameter reaches the engine. Without
// it the engine refuses non-empty datasets and surfaces FAILED_PRECONDITION.
func TestDatasetDeleteForwardsDeleteContents(t *testing.T) {
	fake := &fakeCatalogClient{}
	req := httptest.NewRequest(http.MethodDelete,
		"/bigquery/v2/projects/proj/datasets/ds1?deleteContents=true", nil)
	req.SetPathValue("projectId", "proj")
	req.SetPathValue("datasetId", "ds1")
	rec := httptest.NewRecorder()
	DatasetDelete(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	if fake.lastDropDataset == nil {
		t.Fatal("Catalog.DropDataset was not called")
	}
	if !fake.lastDropDataset.GetDeleteContents() {
		t.Error("delete_contents was not forwarded as true")
	}
	if got := fake.lastDropDataset.GetDataset().GetDatasetId(); got != "ds1" {
		t.Errorf("dataset_id = %q, want %q", got, "ds1")
	}
}

// TestDatasetDeleteNotFound asserts a missing dataset propagates as a
// BigQuery-shaped 404, not the default 500.
func TestDatasetDeleteNotFound(t *testing.T) {
	fake := &fakeCatalogClient{
		dropDatasetFn: func(_ context.Context, _ *enginepb.DropDatasetRequest) (*enginepb.DropDatasetResponse, error) {
			return nil, status.Error(codes.NotFound, "dataset not found")
		},
	}
	req := newDatasetReq(http.MethodDelete, "proj", "ds1", "")
	rec := httptest.NewRecorder()
	DatasetDelete(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404", rec.Code)
	}
	var env errorEnvelope
	if err := json.NewDecoder(rec.Body).Decode(&env); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if env.Error.Status != "notFound" {
		t.Errorf("error.status = %q, want %q", env.Error.Status, "notFound")
	}
}

// TestDatasetGetSynthesizesResource pins the Phase-3 behavior: the
// engine has no Get RPC yet, so the handler returns a synthesized
// resource derived from the path parameters. Once Storage grows a
// DescribeDataset method this test will need updating.
func TestDatasetGetSynthesizesResource(t *testing.T) {
	req := newDatasetReq(http.MethodGet, "proj", "ds1", "")
	rec := httptest.NewRecorder()
	DatasetGet(Dependencies{})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200", rec.Code)
	}
	var ds bqtypes.Dataset
	if err := json.NewDecoder(rec.Body).Decode(&ds); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if ds.Kind != "bigquery#dataset" {
		t.Errorf("kind = %q, want %q", ds.Kind, "bigquery#dataset")
	}
	if ds.DatasetReference.ProjectID != "proj" || ds.DatasetReference.DatasetID != "ds1" {
		t.Errorf("datasetReference = %+v, want {proj, ds1}", ds.DatasetReference)
	}
}

// TestDatasetGetAccessIsEmptyArrayNotNull pins the live-IT contract:
// Java BigQuery's AuthorizeDatasetIT (and any other ACL-mutation
// flow) calls `new ArrayList<>(dataset.getAcl())`, which NPEs when
// the API response has `access: null` or omits the field entirely.
// Live BigQuery returns `access: []` for newly-created datasets; the
// emulator must do the same.
func TestDatasetGetAccessIsEmptyArrayNotNull(t *testing.T) {
	req := newDatasetReq(http.MethodGet, "proj", "ds1", "")
	rec := httptest.NewRecorder()
	DatasetGet(Dependencies{})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200", rec.Code)
	}
	var doc map[string]any
	if err := json.NewDecoder(rec.Body).Decode(&doc); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	got, present := doc["access"]
	if !present {
		t.Fatalf("response missing `access` field; body=%s", rec.Body.String())
	}
	if got == nil {
		t.Fatalf("`access` is null; live BigQuery returns []")
	}
	arr, ok := got.([]any)
	if !ok {
		t.Fatalf("`access` is %T, want []any", got)
	}
	if len(arr) != 0 {
		t.Errorf("`access` = %v, want []", arr)
	}
}

// TestDatasetListReturnsEmptyPage pins the empty-page shape the handler
// returns until Storage grows a list RPC.
func TestDatasetListReturnsEmptyPage(t *testing.T) {
	req := httptest.NewRequest(http.MethodGet, "/bigquery/v2/projects/proj/datasets", nil)
	req.SetPathValue("projectId", "proj")
	rec := httptest.NewRecorder()
	DatasetList(Dependencies{})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200", rec.Code)
	}
	var doc map[string]any
	if err := json.NewDecoder(rec.Body).Decode(&doc); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if doc["kind"] != "bigquery#datasetList" {
		t.Errorf("kind = %v, want %q", doc["kind"], "bigquery#datasetList")
	}
	if _, ok := doc["datasets"]; !ok {
		t.Error("response missing `datasets` field")
	}
}
