package handlers

import (
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
)

func newMigrationReq(method, ver, projectID, location, workflowID string) *http.Request {
	url := "/" + ver + "/projects/" + projectID + "/locations/" + location + "/workflows"
	if workflowID != "" {
		url += "/" + workflowID
	}
	req := httptest.NewRequest(method, url, strings.NewReader(""))
	req.SetPathValue("projectId", projectID)
	req.SetPathValue("location", location)
	if workflowID != "" {
		req.SetPathValue("workflowId", workflowID)
	}
	return req
}

func TestMigrationWorkflowListEmptyPage(t *testing.T) {
	rec := httptest.NewRecorder()
	MigrationWorkflowList(Dependencies{})(rec,
		newMigrationReq(http.MethodGet, "v2alpha", "p", "us", ""))

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	var got struct {
		MigrationWorkflows []any `json:"migrationWorkflows"`
	}
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if len(got.MigrationWorkflows) != 0 {
		t.Errorf("migrationWorkflows = %v, want empty page", got.MigrationWorkflows)
	}
}

func TestMigrationWorkflowGetReturns404(t *testing.T) {
	rec := httptest.NewRecorder()
	MigrationWorkflowGet(Dependencies{})(rec,
		newMigrationReq(http.MethodGet, "v2alpha", "p", "us", "wf1"))

	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404; body=%s", rec.Code, rec.Body.String())
	}
	if !strings.Contains(rec.Body.String(), "projects/p/locations/us/workflows/wf1") {
		t.Errorf("body missing canonical name; got %s", rec.Body.String())
	}
}

func TestMigrationWorkflowCreateReturns501(t *testing.T) {
	rec := httptest.NewRecorder()
	MigrationWorkflowCreate(Dependencies{})(rec,
		newMigrationReq(http.MethodPost, "v2alpha", "p", "us", ""))

	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want 501; body=%s", rec.Code, rec.Body.String())
	}
}

func TestMigrationWorkflowCustomMethodPOSTStart(t *testing.T) {
	rec := httptest.NewRecorder()
	req := newMigrationReq(http.MethodPost, "v2alpha", "p", "us", "wf1:start")
	MigrationWorkflowCustomMethodPOST(Dependencies{})(rec, req)

	if rec.Code != http.StatusNotImplemented {
		t.Fatalf(":start status = %d, want 501; body=%s", rec.Code, rec.Body.String())
	}
}

func TestMigrationWorkflowCustomMethodPOSTUnknownOpReturns404(t *testing.T) {
	rec := httptest.NewRecorder()
	req := newMigrationReq(http.MethodPost, "v2alpha", "p", "us", "wf1:bogus")
	MigrationWorkflowCustomMethodPOST(Dependencies{})(rec, req)

	if rec.Code != http.StatusNotFound {
		t.Fatalf(":bogus status = %d, want 404; body=%s", rec.Code, rec.Body.String())
	}
}
