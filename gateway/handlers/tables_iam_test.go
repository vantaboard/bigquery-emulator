package handlers

import (
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"testing"
)

func TestTableGetIamPolicyReturnsEmptyStubPolicy(t *testing.T) {
	rec := httptest.NewRecorder()
	req := newTableReq(http.MethodPost, testTableID+":getIamPolicy", "{}")
	TableCustomMethodPOST(Dependencies{})(rec, req)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	var got map[string]any
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if etag, _ := got["etag"].(string); etag == "" {
		t.Errorf("etag = %q, want non-empty", etag)
	}
	bindings, _ := got["bindings"].([]any)
	if len(bindings) != 0 {
		t.Errorf("bindings = %v, want []", bindings)
	}
}

func TestTableSetIamPolicyReturnsPolicyWithEtag(t *testing.T) {
	rec := httptest.NewRecorder()
	body := `{"policy":{"version":1,"bindings":[{"role":"roles/viewer","members":["user:alice@example.com"]}]}}`
	req := newTableReq(http.MethodPost, testTableID+":setIamPolicy", body)
	TableCustomMethodPOST(Dependencies{})(rec, req)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	var got map[string]any
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if etag, _ := got["etag"].(string); etag == "" {
		t.Errorf("etag = %q, want non-empty", etag)
	}
	bindings, _ := got["bindings"].([]any)
	if len(bindings) != 1 {
		t.Fatalf("bindings = %v, want one entry", bindings)
	}
}
