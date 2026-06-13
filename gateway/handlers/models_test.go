package handlers

import (
	"context"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/models"
)

const (
	modelTestProjectID = "p"
	modelTestDatasetID = "d"
)

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

func TestModelListReturnsRegisteredModels(t *testing.T) {
	t.Parallel()
	store := models.NewStore()
	store.Upsert(bqtypes.Model{
		ModelReference: bqtypes.ModelReference{
			ProjectID: modelTestProjectID,
			DatasetID: modelTestDatasetID,
			ModelID:   "m1",
		},
		ModelType: "LINEAR_REG",
	})
	rec := httptest.NewRecorder()
	ModelList(Dependencies{Models: store})(rec, newModelReq(http.MethodGet, ""))
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d", rec.Code)
	}
	var got struct {
		Models []bqtypes.Model `json:"models"`
	}
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatal(err)
	}
	if len(got.Models) != 1 || got.Models[0].ModelReference.ModelID != "m1" {
		t.Fatalf("models = %#v", got.Models)
	}
}

func TestModelGetReturnsRegisteredModel(t *testing.T) {
	t.Parallel()
	store := models.NewStore()
	store.Upsert(bqtypes.Model{
		ModelReference: bqtypes.ModelReference{
			ProjectID: modelTestProjectID,
			DatasetID: modelTestDatasetID,
			ModelID:   "m1",
		},
		ModelType: "LINEAR_REG",
	})
	rec := httptest.NewRecorder()
	ModelGet(Dependencies{Models: store})(rec, newModelReq(http.MethodGet, "m1"))
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, body=%s", rec.Code, rec.Body.String())
	}
}

func TestModelDeleteRemovesRegisteredModel(t *testing.T) {
	t.Parallel()
	store := models.NewStore()
	store.Upsert(bqtypes.Model{
		ModelReference: bqtypes.ModelReference{
			ProjectID: modelTestProjectID,
			DatasetID: modelTestDatasetID,
			ModelID:   "m1",
		},
	})
	rec := httptest.NewRecorder()
	ModelDelete(Dependencies{Models: store})(rec, newModelReq(http.MethodDelete, "m1"))
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d", rec.Code)
	}
	if _, ok := store.Get(modelTestProjectID, modelTestDatasetID, "m1"); ok {
		t.Fatal("model still present")
	}
}

func TestPersistModelFromDDL(t *testing.T) {
	t.Parallel()
	deps := Dependencies{Models: models.NewStore()}
	ref := persistModelFromDDL(context.TODO(), &deps, "p", "d",
		"CREATE MODEL `p.d.linear` OPTIONS(model_type='linear_reg') AS SELECT 1")
	if ref == nil || ref.ModelID != "linear" {
		t.Fatalf("ref = %#v", ref)
	}
}

func TestModelGetReturnsNotFoundWhenAbsent(t *testing.T) {
	t.Parallel()
	rec := httptest.NewRecorder()
	ModelGet(Dependencies{})(rec, newModelReq(http.MethodGet, "m1"))
	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d", rec.Code)
	}
	if !strings.Contains(rec.Body.String(), "Not found: Model") {
		t.Fatalf("body = %s", rec.Body.String())
	}
}
