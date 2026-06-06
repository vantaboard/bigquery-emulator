package handlers

import (
	"bytes"
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

func resetMigrationWorkflowStoreForTest(t *testing.T) {
	t.Helper()
	migrationWorkflowStore.Range(func(key, _ any) bool {
		migrationWorkflowStore.Delete(key)
		return true
	})
}

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
	resetMigrationWorkflowStoreForTest(t)
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

func TestMigrationWorkflowCreateGetListDelete(t *testing.T) {
	resetMigrationWorkflowStoreForTest(t)

	createRec := httptest.NewRecorder()
	createReq := httptest.NewRequest(http.MethodPost,
		"/"+migrationAPIVersion+"/projects/"+migrationProjectID+
			"/locations/"+migrationLocation+"/workflows",
		bytes.NewReader([]byte(`{"displayName":"wf-demo"}`)))
	createReq.SetPathValue("projectId", migrationProjectID)
	createReq.SetPathValue("location", migrationLocation)
	MigrationWorkflowCreate(Dependencies{})(createRec, createReq)
	if createRec.Code != http.StatusOK {
		t.Fatalf("create status = %d, want 200; body=%s", createRec.Code, createRec.Body.String())
	}
	var created migrationWorkflowResource
	if err := json.NewDecoder(createRec.Body).Decode(&created); err != nil {
		t.Fatalf("decode create: %v", err)
	}
	if created.Name == "" {
		t.Fatal("create: empty name")
	}
	if created.State != migrationWorkflowStateDraft {
		t.Errorf("state = %q, want %q", created.State, migrationWorkflowStateDraft)
	}

	listRec := httptest.NewRecorder()
	MigrationWorkflowList(Dependencies{})(listRec, newMigrationReq(http.MethodGet, ""))
	if listRec.Code != http.StatusOK {
		t.Fatalf("list status = %d, want 200", listRec.Code)
	}
	var listed struct {
		MigrationWorkflows []migrationWorkflowResource `json:"migrationWorkflows"`
	}
	if err := json.NewDecoder(listRec.Body).Decode(&listed); err != nil {
		t.Fatalf("decode list: %v", err)
	}
	if len(listed.MigrationWorkflows) != 1 {
		t.Fatalf("listed = %d, want 1", len(listed.MigrationWorkflows))
	}

	wid := strings.TrimPrefix(created.Name,
		"projects/"+migrationProjectID+"/locations/"+migrationLocation+"/workflows/")
	getRec := httptest.NewRecorder()
	MigrationWorkflowGet(Dependencies{})(getRec, newMigrationReq(http.MethodGet, wid))
	if getRec.Code != http.StatusOK {
		t.Fatalf("get status = %d, want 200; body=%s", getRec.Code, getRec.Body.String())
	}

	delRec := httptest.NewRecorder()
	MigrationWorkflowDelete(Dependencies{})(delRec, newMigrationReq(http.MethodDelete, wid))
	if delRec.Code != http.StatusOK {
		t.Fatalf("delete status = %d, want 200", delRec.Code)
	}

	missRec := httptest.NewRecorder()
	MigrationWorkflowGet(Dependencies{})(missRec, newMigrationReq(http.MethodGet, wid))
	if missRec.Code != http.StatusNotFound {
		t.Fatalf("get after delete status = %d, want 404", missRec.Code)
	}
}

func TestMigrationWorkflowGetReturns404(t *testing.T) {
	resetMigrationWorkflowStoreForTest(t)
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

func TestMigrationWorkflowCustomMethodPOSTStart(t *testing.T) {
	resetMigrationWorkflowStoreForTest(t)

	createRec := httptest.NewRecorder()
	createReq := httptest.NewRequest(http.MethodPost,
		"/"+migrationAPIVersion+"/projects/"+migrationProjectID+
			"/locations/"+migrationLocation+"/workflows",
		bytes.NewReader([]byte("{}")))
	createReq.SetPathValue("projectId", migrationProjectID)
	createReq.SetPathValue("location", migrationLocation)
	MigrationWorkflowCreate(Dependencies{})(createRec, createReq)
	if createRec.Code != http.StatusOK {
		t.Fatalf("create status = %d, want 200", createRec.Code)
	}
	var created migrationWorkflowResource
	if err := json.NewDecoder(createRec.Body).Decode(&created); err != nil {
		t.Fatalf("decode create: %v", err)
	}
	wid := strings.TrimPrefix(created.Name,
		"projects/"+migrationProjectID+"/locations/"+migrationLocation+"/workflows/")

	startRec := httptest.NewRecorder()
	startReq := newMigrationReq(http.MethodPost, wid+":start")
	MigrationWorkflowCustomMethodPOST(Dependencies{})(startRec, startReq)
	if startRec.Code != http.StatusOK {
		t.Fatalf(":start status = %d, want 200; body=%s", startRec.Code, startRec.Body.String())
	}

	getRec := httptest.NewRecorder()
	MigrationWorkflowGet(Dependencies{})(getRec, newMigrationReq(http.MethodGet, wid))
	var got migrationWorkflowResource
	if err := json.NewDecoder(getRec.Body).Decode(&got); err != nil {
		t.Fatalf("decode get: %v", err)
	}
	if got.State != migrationWorkflowStateRunning {
		t.Errorf("state = %q, want %q", got.State, migrationWorkflowStateRunning)
	}
}

func TestMigrationWorkflowCustomMethodPOSTUnknownOpReturns404(t *testing.T) {
	resetMigrationWorkflowStoreForTest(t)
	rec := httptest.NewRecorder()
	req := newMigrationReq(http.MethodPost, "wf1:bogus")
	MigrationWorkflowCustomMethodPOST(Dependencies{})(rec, req)

	if rec.Code != http.StatusNotFound {
		t.Fatalf(":bogus status = %d, want 404; body=%s", rec.Code, rec.Body.String())
	}
}
