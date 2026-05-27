package handlers

import (
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
)

func newRowAccessPoliciesReq(method, projectID, datasetID, tableID string) *http.Request {
	url := "/bigquery/v2/projects/" + projectID + "/datasets/" + datasetID +
		"/tables/" + tableID + "/rowAccessPolicies"
	req := httptest.NewRequest(method, url, strings.NewReader(""))
	req.SetPathValue("projectId", projectID)
	req.SetPathValue("datasetId", datasetID)
	req.SetPathValue("tableId", tableID)
	return req
}

func TestRowAccessPolicyListReturnsEmptyPage(t *testing.T) {
	rec := httptest.NewRecorder()
	RowAccessPolicyList(Dependencies{})(rec, newRowAccessPoliciesReq(http.MethodGet, "p", "d", "t"))

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	var got struct {
		Kind              string `json:"kind"`
		RowAccessPolicies []any  `json:"rowAccessPolicies"`
	}
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if got.Kind != rowAccessPolicyListKind {
		t.Errorf("kind = %q, want %q", got.Kind, rowAccessPolicyListKind)
	}
	if len(got.RowAccessPolicies) != 0 {
		t.Errorf("rowAccessPolicies = %v, want empty page", got.RowAccessPolicies)
	}
}

func TestRowAccessPolicyIamReturnsNotImplemented(t *testing.T) {
	rec := httptest.NewRecorder()
	RowAccessPolicyIamPolicy(Dependencies{})(rec,
		newRowAccessPoliciesReq(http.MethodPost, "p", "d", "t"))

	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want 501; body=%s", rec.Code, rec.Body.String())
	}
}
