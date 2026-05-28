package handlers

import (
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
)

// modelTestProjectID and modelTestDatasetID are the only project /
// dataset values these tests target; promoted to consts so newModelReq
// can drop the otherwise-constant `projectID` parameter (unparam).
const (
	modelTestProjectID = "p"
	modelTestDatasetID = "d"
)

// newModelReq builds an *http.Request with the path-value wildcards
// populated the way Go's mux populates them at runtime. Mirrors
// newDatasetReq in datasets_test.go. Both the project and dataset IDs
// are constant across every caller, so they live in package consts
// rather than per-call parameters.
func newModelReq(method, modelID string) *http.Request {
	url := "/bigquery/v2/projects/" + modelTestProjectID + "/datasets/" + modelTestDatasetID + "/models"
	if modelID != "" {
		url += "/" + modelID
	}
	req := httptest.NewRequest(method, url, strings.NewReader(""))
	req.SetPathValue("projectId", modelTestProjectID)
	req.SetPathValue("datasetId", modelTestDatasetID)
	if modelID != "" {
		req.SetPathValue("modelId", modelID)
	}
	return req
}

// TestModelListReturnsEmptyPage verifies the list endpoint returns the
// BigQuery-shaped envelope so client libraries that probe at startup
// don't fall through to the catch-all NotFound handler.
func TestModelListReturnsEmptyPage(t *testing.T) {
	rec := httptest.NewRecorder()
	ModelList(Dependencies{})(rec, newModelReq(http.MethodGet, ""))

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	var got struct {
		Kind   string `json:"kind"`
		Models []any  `json:"models"`
	}
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if got.Kind != modelListKind {
		t.Errorf("kind = %q, want %q", got.Kind, modelListKind)
	}
	if len(got.Models) != 0 {
		t.Errorf("models = %v, want empty page", got.Models)
	}
}

// TestModelGetReturnsNotFound verifies that any specific modelId
// yields the BigQuery 404 envelope (not 501) so list-get-delete sample
// loops can rely on the "absent" semantics.
func TestModelGetReturnsNotFound(t *testing.T) {
	rec := httptest.NewRecorder()
	ModelGet(Dependencies{})(rec, newModelReq(http.MethodGet, "m1"))

	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404; body=%s", rec.Code, rec.Body.String())
	}
	if !strings.Contains(rec.Body.String(), "Not found: Model p:d.m1") {
		t.Errorf("body missing fqn; got %s", rec.Body.String())
	}
}

// TestModelDeleteReturnsNotFound mirrors the Get behavior so a
// list-get-delete loop sees consistent "absent" semantics.
func TestModelDeleteReturnsNotFound(t *testing.T) {
	rec := httptest.NewRecorder()
	ModelDelete(Dependencies{})(rec, newModelReq(http.MethodDelete, "m1"))

	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404; body=%s", rec.Code, rec.Body.String())
	}
}

// TestModelPatchReturnsNotImplemented documents that a mutating
// operation against the stub returns 501 (no BQML store yet).
func TestModelPatchReturnsNotImplemented(t *testing.T) {
	rec := httptest.NewRecorder()
	ModelPatch(Dependencies{})(rec, newModelReq(http.MethodPatch, "m1"))

	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want 501; body=%s", rec.Code, rec.Body.String())
	}
}
