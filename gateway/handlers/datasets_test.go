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
// expected values without going through the full mux. The project ID
// is the package-level testProjectID; every call site in this package
// uses that constant.
func newDatasetReq(method, datasetID, body string) *http.Request {
	url := "/bigquery/v2/projects/" + testProjectID + "/datasets"
	if datasetID != "" {
		url += "/" + datasetID
	}
	reqBody := strings.NewReader(body)
	req := httptest.NewRequest(method, url, reqBody)
	req.SetPathValue("projectId", testProjectID)
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

	req := newDatasetReq(http.MethodPost, "",
		`{"datasetReference":{"datasetId":"ds1"},"location":"US","friendlyName":"hello"}`)
	rec := httptest.NewRecorder()
	DatasetInsert(deps)(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	if fake.lastRegisterDataset == nil {
		t.Fatal("Catalog.RegisterDataset was not called")
	}
	if got := fake.lastRegisterDataset.GetDataset().GetProjectId(); got != testProjectID {
		t.Errorf("project_id forwarded = %q, want %q", got, testProjectID)
	}
	if got := fake.lastRegisterDataset.GetDataset().GetDatasetId(); got != testDatasetID {
		t.Errorf("dataset_id forwarded = %q, want %q", got, testDatasetID)
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
	if ds.DatasetReference.ProjectID != testProjectID || ds.DatasetReference.DatasetID != testDatasetID {
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
	req := newDatasetReq(http.MethodPost, "", `{"location":"US"}`)
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
	req := newDatasetReq(http.MethodPost, "",
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
// pure gateway-only mode.
func TestDatasetInsertWithoutCatalogFallsBackTo501(t *testing.T) {
	req := newDatasetReq(http.MethodPost, "",
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
	req.SetPathValue("projectId", testProjectID)
	req.SetPathValue("datasetId", testDatasetID)
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
	if got := fake.lastDropDataset.GetDataset().GetDatasetId(); got != testDatasetID {
		t.Errorf("dataset_id = %q, want %q", got, testDatasetID)
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
	req := newDatasetReq(http.MethodDelete, testDatasetID, "")
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

// TestDatasetGetSynthesizesResource pins the catalog stub behavior:
// the engine has no Get RPC yet, so the handler returns a
// synthesized resource derived from the path parameters. Once
// Storage grows a DescribeDataset method this test will need
// updating.
func TestDatasetGetSynthesizesResource(t *testing.T) {
	req := newDatasetReq(http.MethodGet, testDatasetID, "")
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
	if ds.DatasetReference.ProjectID != testProjectID || ds.DatasetReference.DatasetID != testDatasetID {
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
	req := newDatasetReq(http.MethodGet, testDatasetID, "")
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

// TestDatasetGetLabelsIsEmptyObjectNotNull pins the live-IT contract:
// node-bigquery-tests `getDatasetLabels` calls
// `Object.entries(dataset.metadata.labels)` on the deserialized
// response, which raises `TypeError: Cannot convert undefined or null
// to object` when the field is absent or null. Live BigQuery returns
// `labels: {}` for a newly-created dataset; the emulator must do the
// same.
func TestDatasetGetLabelsIsEmptyObjectNotNull(t *testing.T) {
	req := newDatasetReq(http.MethodGet, testDatasetID, "")
	rec := httptest.NewRecorder()
	DatasetGet(Dependencies{})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200", rec.Code)
	}
	var doc map[string]any
	if err := json.NewDecoder(rec.Body).Decode(&doc); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	got, present := doc["labels"]
	if !present {
		t.Fatalf("response missing `labels` field; body=%s", rec.Body.String())
	}
	if got == nil {
		t.Fatalf("`labels` is null; live BigQuery returns {}")
	}
	obj, ok := got.(map[string]any)
	if !ok {
		t.Fatalf("`labels` is %T, want map[string]any", got)
	}
	if len(obj) != 0 {
		t.Errorf("`labels` = %v, want {}", obj)
	}
}

// TestDatasetListReturnsEmptyPage pins the empty-page shape (no
// Catalog client wired) the handler returns when the engine is
// missing. The catalog stub still returns a 200 with the
// datasetList envelope so client iterators don't error.
func TestDatasetListReturnsEmptyPage(t *testing.T) {
	req := httptest.NewRequest(http.MethodGet, "/bigquery/v2/projects/proj/datasets", nil)
	req.SetPathValue("projectId", testProjectID)
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

// TestDatasetListSurfacesEngineEntries asserts the handler forwards
// the project_id, calls the engine, and folds the response into the
// REST envelope. Each entry must carry the required tableList-like
// shape so the node sample's `dataset.metadata.labels` access
// doesn't NPE (mirrors TestDatasetGetLabelsIsEmptyObjectNotNull).
func TestDatasetListSurfacesEngineEntries(t *testing.T) {
	fake := &fakeCatalogClient{
		listDatasetsFn: func(_ context.Context, _ *enginepb.ListDatasetsRequest) (*enginepb.ListDatasetsResponse, error) {
			return &enginepb.ListDatasetsResponse{
				Datasets: []*enginepb.DatasetRef{
					{ProjectId: testProjectID, DatasetId: "ds_alpha"},
					{ProjectId: testProjectID, DatasetId: "ds_bravo"},
				},
			}, nil
		},
	}
	req := newDatasetReq(http.MethodGet, "", "")
	rec := httptest.NewRecorder()
	DatasetList(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	if fake.lastListDatasets == nil {
		t.Fatal("Catalog.ListDatasets was not called")
	}
	if got := fake.lastListDatasets.GetProjectId(); got != testProjectID {
		t.Errorf("project_id forwarded = %q, want %q", got, testProjectID)
	}
	var doc map[string]any
	if err := json.NewDecoder(rec.Body).Decode(&doc); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if doc["kind"] != "bigquery#datasetList" {
		t.Errorf("kind = %v", doc["kind"])
	}
	items, _ := doc["datasets"].([]any)
	if len(items) != 2 {
		t.Fatalf("datasets entries = %d, want 2; body=%s", len(items), rec.Body.String())
	}
	first, _ := items[0].(map[string]any)
	if first["id"] != testProjectID+":ds_alpha" {
		t.Errorf("first entry id = %v, want %q", first["id"], testProjectID+":ds_alpha")
	}
	ref, _ := first["datasetReference"].(map[string]any)
	if ref["datasetId"] != "ds_alpha" || ref["projectId"] != testProjectID {
		t.Errorf("first entry datasetReference = %+v", ref)
	}
	if labels, ok := first["labels"].(map[string]any); !ok || len(labels) != 0 {
		t.Errorf("first entry labels = %v, want {}", first["labels"])
	}
}

// runDatasetMetadataRoundTrip drives the Insert -> Patch -> Get cycle
// a node `dataset.setMetadata` + `dataset.getMetadata` sample
// exercises, asserting the cached REST-only field comes back on the
// wire. Without the MetadataStore the Get handler would return the
// synthesized empty dataset and lose the prior PATCH payload.
func runDatasetMetadataRoundTrip(t *testing.T, patch string, assertion func(*testing.T, bqtypes.Dataset)) {
	t.Helper()
	store := NewMetadataStore()
	fake := &fakeCatalogClient{}
	deps := Dependencies{Catalog: fake, Metadata: store}

	insert := newDatasetReq(http.MethodPost, "",
		`{"datasetReference":{"datasetId":"`+testDatasetID+`"}}`)
	rec := httptest.NewRecorder()
	DatasetInsert(deps)(rec, insert)
	if rec.Code != http.StatusOK {
		t.Fatalf("insert: status=%d body=%s", rec.Code, rec.Body.String())
	}

	patchReq := newDatasetReq(http.MethodPatch, testDatasetID, patch)
	rec = httptest.NewRecorder()
	DatasetPatch(deps)(rec, patchReq)
	if rec.Code != http.StatusOK {
		t.Fatalf("patch: status=%d body=%s", rec.Code, rec.Body.String())
	}

	get := newDatasetReq(http.MethodGet, testDatasetID, "")
	rec = httptest.NewRecorder()
	DatasetGet(deps)(rec, get)
	if rec.Code != http.StatusOK {
		t.Fatalf("get: status=%d body=%s", rec.Code, rec.Body.String())
	}
	var got bqtypes.Dataset
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode get response: %v", err)
	}
	assertion(t, got)
}

// TestDatasetMetadataLabelsRoundTrip mirrors the upstream
// `labelDataset` + `getDatasetLabels` sample sequence.
func TestDatasetMetadataLabelsRoundTrip(t *testing.T) {
	runDatasetMetadataRoundTrip(t, `{"labels":{"color":"green"}}`, func(t *testing.T, got bqtypes.Dataset) {
		if got.Labels["color"] != "green" {
			t.Errorf("labels = %v, want {color:\"green\"}", got.Labels)
		}
	})
}

// TestDatasetMetadataDefaultCollationRoundTrip pins the `und:ci`
// dataset-level collation the `should create a dataset with collation`
// node test asserts on GET.
func TestDatasetMetadataDefaultCollationRoundTrip(t *testing.T) {
	runDatasetMetadataRoundTrip(t, `{"defaultCollation":"und:ci"}`, func(t *testing.T, got bqtypes.Dataset) {
		if got.DefaultCollation != "und:ci" {
			t.Errorf("defaultCollation = %q, want %q", got.DefaultCollation, "und:ci")
		}
	})
}

// TestDatasetDeleteEvictsMetadata asserts DELETE clears the
// MetadataStore so a subsequent Insert against the same ID does not
// surface stale labels. With deleteContents=true the dataset's table
// entries should also be flushed.
func TestDatasetDeleteEvictsMetadata(t *testing.T) {
	store := NewMetadataStore()
	fake := &fakeCatalogClient{}
	deps := Dependencies{Catalog: fake, Metadata: store}

	store.PutDataset(testProjectID, testDatasetID, bqtypes.Dataset{Labels: map[string]string{"k": "v"}})
	store.PutTable(testProjectID, testDatasetID, testTableID, bqtypes.Table{Labels: map[string]string{"k": "v"}})

	req := newDatasetReq(http.MethodDelete, testDatasetID, "")
	q := req.URL.Query()
	q.Set("deleteContents", "true")
	req.URL.RawQuery = q.Encode()
	rec := httptest.NewRecorder()
	DatasetDelete(deps)(rec, req)
	if rec.Code != http.StatusOK {
		t.Fatalf("delete: status=%d body=%s", rec.Code, rec.Body.String())
	}
	if _, ok := store.GetDataset(testProjectID, testDatasetID); ok {
		t.Error("MetadataStore dataset entry survived DatasetDelete")
	}
	if _, ok := store.GetTable(testProjectID, testDatasetID, testTableID); ok {
		t.Error("MetadataStore table entry survived DatasetDelete(deleteContents=true)")
	}
}
