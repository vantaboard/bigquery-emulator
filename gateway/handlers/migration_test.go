package handlers

import (
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
)

// migrationAPIVersion is the only API version any caller in this
// package targets; migrationProjectID and migrationLocation are
// similarly the only project ID and location the migration tests
// exercise. Promoted to consts so newMigrationReq does not need to
// accept any of them as parameters.
const (
	migrationAPIVersion = "v2alpha"
	migrationProjectID  = "p"
	migrationLocation   = "us"
)

func newMigrationReq(method, workflowID string) *http.Request {
	url := "/" + migrationAPIVersion + "/projects/" + migrationProjectID +
		"/locations/" + migrationLocation + "/workflows"
	if workflowID != "" {
		url += "/" + workflowID
	}
	req := httptest.NewRequest(method, url, strings.NewReader(""))
	req.SetPathValue("projectId", migrationProjectID)
	req.SetPathValue("location", migrationLocation)
	if workflowID != "" {
		req.SetPathValue("workflowId", workflowID)
	}
	return req
}

func TestMigrationWorkflowListEmptyPage(t *testing.T) {
	rec := httptest.NewRecorder()
	MigrationWorkflowList(Dependencies{})(rec,
		newMigrationReq(http.MethodGet, ""))

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
		newMigrationReq(http.MethodGet, "wf1"))

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
		newMigrationReq(http.MethodPost, ""))

	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want 501; body=%s", rec.Code, rec.Body.String())
	}
}

func TestMigrationWorkflowCustomMethodPOSTStart(t *testing.T) {
	rec := httptest.NewRecorder()
	req := newMigrationReq(http.MethodPost, "wf1:start")
	MigrationWorkflowCustomMethodPOST(Dependencies{})(rec, req)

	if rec.Code != http.StatusNotImplemented {
		t.Fatalf(":start status = %d, want 501; body=%s", rec.Code, rec.Body.String())
	}
}

func TestMigrationWorkflowCustomMethodPOSTUnknownOpReturns404(t *testing.T) {
	rec := httptest.NewRecorder()
	req := newMigrationReq(http.MethodPost, "wf1:bogus")
	MigrationWorkflowCustomMethodPOST(Dependencies{})(rec, req)

	if rec.Code != http.StatusNotFound {
		t.Fatalf(":bogus status = %d, want 404; body=%s", rec.Code, rec.Body.String())
	}
}
