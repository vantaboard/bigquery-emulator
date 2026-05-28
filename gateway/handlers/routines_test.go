package handlers

import (
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
)

// routineTestProjectID and routineTestDatasetID are the only project /
// dataset values these tests target; promoted to consts so
// newRoutineReq can drop the otherwise-constant `projectID` parameter
// (unparam).
const (
	routineTestProjectID = "p"
	routineTestDatasetID = "d"
)

func newRoutineReq(method, routineID string) *http.Request {
	url := "/bigquery/v2/projects/" + routineTestProjectID + "/datasets/" + routineTestDatasetID + "/routines"
	if routineID != "" {
		url += "/" + routineID
	}
	req := httptest.NewRequest(method, url, strings.NewReader(""))
	req.SetPathValue("projectId", routineTestProjectID)
	req.SetPathValue("datasetId", routineTestDatasetID)
	if routineID != "" {
		req.SetPathValue("routineId", routineID)
	}
	return req
}

func TestRoutineListReturnsEmptyPage(t *testing.T) {
	rec := httptest.NewRecorder()
	RoutineList(Dependencies{})(rec, newRoutineReq(http.MethodGet, ""))

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
	RoutineGet(Dependencies{})(rec, newRoutineReq(http.MethodGet, "r1"))

	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404; body=%s", rec.Code, rec.Body.String())
	}
	if !strings.Contains(rec.Body.String(), "Not found: Routine p:d.r1") {
		t.Errorf("body missing fqn; got %s", rec.Body.String())
	}
}

func TestRoutineInsertReturnsNotImplemented(t *testing.T) {
	rec := httptest.NewRecorder()
	RoutineInsert(Dependencies{})(rec, newRoutineReq(http.MethodPost, ""))

	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want 501; body=%s", rec.Code, rec.Body.String())
	}
}

func TestRoutineUpdateReturnsNotImplemented(t *testing.T) {
	rec := httptest.NewRecorder()
	RoutineUpdate(Dependencies{})(rec, newRoutineReq(http.MethodPut, "r1"))

	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want 501; body=%s", rec.Code, rec.Body.String())
	}
}

func TestRoutineDeleteReturnsNotFound(t *testing.T) {
	rec := httptest.NewRecorder()
	RoutineDelete(Dependencies{})(rec, newRoutineReq(http.MethodDelete, "r1"))

	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404; body=%s", rec.Code, rec.Body.String())
	}
}
