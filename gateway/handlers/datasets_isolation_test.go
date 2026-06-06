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

// TestDatasetPatchDeleteLabelResponseShape mirrors deleteLabelDataset:
// PATCH with {color:null} must return labels:{} on the wire so
// apiResponse.labels is not undefined in the Node sample.
func TestDatasetPatchDeleteLabelResponseShape(t *testing.T) {
	store := NewMetadataStore()
	deps := Dependencies{Catalog: &fakeCatalogClient{}, Metadata: store}

	insert := newDatasetReq(http.MethodPost, "",
		`{"datasetReference":{"datasetId":"`+testDatasetID+`"}}`)
	rec := httptest.NewRecorder()
	DatasetInsert(deps)(rec, insert)
	if rec.Code != http.StatusOK {
		t.Fatalf("insert: status=%d body=%s", rec.Code, rec.Body.String())
	}

	labelPatch := newDatasetReq(http.MethodPatch, testDatasetID, `{"labels":{"color":"green"}}`)
	rec = httptest.NewRecorder()
	DatasetPatch(deps)(rec, labelPatch)
	if rec.Code != http.StatusOK {
		t.Fatalf("label patch: status=%d body=%s", rec.Code, rec.Body.String())
	}

	deletePatch := newDatasetReq(http.MethodPatch, testDatasetID, `{"labels":{"color":null}}`)
	rec = httptest.NewRecorder()
	DatasetPatch(deps)(rec, deletePatch)
	if rec.Code != http.StatusOK {
		t.Fatalf("delete label patch: status=%d body=%s", rec.Code, rec.Body.String())
	}

	var doc map[string]any
	if err := json.Unmarshal(rec.Body.Bytes(), &doc); err != nil {
		t.Fatalf("decode patch response: %v", err)
	}
	if labels, present := doc["labels"]; present {
		if labels == nil {
			t.Fatal("patch response labels is null")
		}
		obj, ok := labels.(map[string]any)
		if !ok {
			t.Fatalf("labels is %T, want map[string]any", labels)
		}
		if len(obj) != 0 {
			t.Errorf("labels = %v, want empty or omitted", obj)
		}
	}
}

func TestDatasetPatchClearDefaultCollation(t *testing.T) {
	store := NewMetadataStore()
	deps := Dependencies{Catalog: &fakeCatalogClient{}, Metadata: store}

	insert := newDatasetReq(http.MethodPost, "",
		`{"datasetReference":{"datasetId":"`+testDatasetID+`"}}`)
	rec := httptest.NewRecorder()
	DatasetInsert(deps)(rec, insert)
	if rec.Code != http.StatusOK {
		t.Fatalf("insert: status=%d", rec.Code)
	}

	setCollation := newDatasetReq(http.MethodPatch, testDatasetID, `{"defaultCollation":"und:ci"}`)
	rec = httptest.NewRecorder()
	DatasetPatch(deps)(rec, setCollation)
	if rec.Code != http.StatusOK {
		t.Fatalf("set collation: status=%d", rec.Code)
	}

	clearCollation := newDatasetReq(http.MethodPatch, testDatasetID, `{"defaultCollation":""}`)
	rec = httptest.NewRecorder()
	DatasetPatch(deps)(rec, clearCollation)
	if rec.Code != http.StatusOK {
		t.Fatalf("clear collation: status=%d", rec.Code)
	}

	get := newDatasetReq(http.MethodGet, testDatasetID, "")
	rec = httptest.NewRecorder()
	DatasetGet(deps)(rec, get)
	if rec.Code != http.StatusOK {
		t.Fatalf("get: status=%d", rec.Code)
	}
	var got bqtypes.Dataset
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.DefaultCollation != "" {
		t.Errorf("defaultCollation = %q, want empty", got.DefaultCollation)
	}
}

// TestDatasetInsertInvalidRegionBeforeDuplicate asserts invalid location
// is rejected before RegisterDataset when a regional API header is set.
func TestDatasetInsertInvalidRegionBeforeDuplicate(t *testing.T) {
	fake := &fakeCatalogClient{
		registerDatasetFn: func(_ context.Context, _ *enginepb.RegisterDatasetRequest) (*enginepb.RegisterDatasetResponse, error) {
			return nil, status.Error(codes.AlreadyExists, "dataset already exists: proj.ds1")
		},
	}
	req := newDatasetReq(http.MethodPost, "",
		`{"datasetReference":{"datasetId":"ds1"},"location":"us-central1"}`)
	req.Header.Set(headerEmulatorAPIRegion, "us-east4")
	rec := httptest.NewRecorder()
	DatasetInsert(Dependencies{Catalog: fake})(rec, req)

	if rec.Code != http.StatusBadRequest {
		t.Fatalf("status = %d, want 400; body=%s", rec.Code, rec.Body.String())
	}
	if fake.lastRegisterDataset != nil {
		t.Fatal("RegisterDataset must not run when location mismatches regional endpoint")
	}
	var env errorEnvelope
	if err := json.NewDecoder(rec.Body).Decode(&env); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if !strings.Contains(env.Error.Message, "Invalid storage region") {
		t.Errorf("message = %q, want Invalid storage region", env.Error.Message)
	}
}

// TestDatasetDeleteEvictsMetadata asserts DELETE clears the
// MetadataStore so a subsequent Insert against the same ID does not
// surface stale labels. With deleteContents=true the dataset's table
// entries should also be flushed.
func TestDatasetDeleteEvictsMetadata(t *testing.T) {
	store := NewMetadataStore()
	fake := &fakeCatalogClient{}
	deps := Dependencies{Catalog: fake, Metadata: store}

	store.PutDataset(testProjectID, testDatasetID, bqtypes.Dataset{Labels: bqtypes.ResourceLabels{"k": "v"}})
	store.PutTable(testProjectID, testDatasetID, testTableID, bqtypes.Table{Labels: bqtypes.ResourceLabels{"k": "v"}})

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
