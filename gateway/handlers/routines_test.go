package handlers

import (
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
)

func newRoutineReq(method, projectID, datasetID, routineID string) *http.Request {
	url := "/bigquery/v2/projects/" + projectID + "/datasets/" + datasetID + "/routines"
	if routineID != "" {
		url += "/" + routineID
	}
	req := httptest.NewRequest(method, url, strings.NewReader(""))
	req.SetPathValue("projectId", projectID)
	req.SetPathValue("datasetId", datasetID)
	if routineID != "" {
		req.SetPathValue("routineId", routineID)
	}
	return req
}

func TestRoutineListReturnsEmptyPage(t *testing.T) {
	rec := httptest.NewRecorder()
	RoutineList(Dependencies{})(rec, newRoutineReq(http.MethodGet, "p", "d", ""))

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	var got struct {
		Kind     string `json:"kind"`
		Routines []any  `json:"routines"`
	}
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if got.Kind != routineListKind {
		t.Errorf("kind = %q, want %q", got.Kind, routineListKind)
	}
	if len(got.Routines) != 0 {
		t.Errorf("routines = %v, want empty page", got.Routines)
	}
}

func TestRoutineGetReturnsNotFound(t *testing.T) {
	rec := httptest.NewRecorder()
	RoutineGet(Dependencies{})(rec, newRoutineReq(http.MethodGet, "p", "d", "r1"))

	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404; body=%s", rec.Code, rec.Body.String())
	}
	if !strings.Contains(rec.Body.String(), "Not found: Routine p:d.r1") {
		t.Errorf("body missing fqn; got %s", rec.Body.String())
	}
}

func TestRoutineInsertReturnsNotImplemented(t *testing.T) {
	rec := httptest.NewRecorder()
	RoutineInsert(Dependencies{})(rec, newRoutineReq(http.MethodPost, "p", "d", ""))

	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want 501; body=%s", rec.Code, rec.Body.String())
	}
}

func TestRoutineUpdateReturnsNotImplemented(t *testing.T) {
	rec := httptest.NewRecorder()
	RoutineUpdate(Dependencies{})(rec, newRoutineReq(http.MethodPut, "p", "d", "r1"))

	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want 501; body=%s", rec.Code, rec.Body.String())
	}
}

func TestRoutineDeleteReturnsNotFound(t *testing.T) {
	rec := httptest.NewRecorder()
	RoutineDelete(Dependencies{})(rec, newRoutineReq(http.MethodDelete, "p", "d", "r1"))

	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404; body=%s", rec.Code, rec.Body.String())
	}
}
