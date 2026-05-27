package handlers

import (
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
)

func newDataTransferReq(method, path string, pathVals map[string]string) *http.Request {
	req := httptest.NewRequest(method, path, strings.NewReader(""))
	for k, v := range pathVals {
		req.SetPathValue(k, v)
	}
	return req
}

func TestDataTransferDataSourceListEmptyPage(t *testing.T) {
	rec := httptest.NewRecorder()
	DataTransferDataSourceList(Dependencies{})(rec,
		newDataTransferReq(http.MethodGet, "/v1/projects/p/dataSources",
			map[string]string{"projectId": "p"}))

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	var got struct {
		DataSources []any `json:"dataSources"`
	}
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if len(got.DataSources) != 0 {
		t.Errorf("dataSources = %v, want empty page", got.DataSources)
	}
}

func TestDataTransferDataSourceGetReturns404(t *testing.T) {
	rec := httptest.NewRecorder()
	DataTransferDataSourceGet(Dependencies{})(rec,
		newDataTransferReq(http.MethodGet, "/v1/projects/p/dataSources/ds1",
			map[string]string{"projectId": "p", "dataSourceId": "ds1"}))

	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404; body=%s", rec.Code, rec.Body.String())
	}
	if !strings.Contains(rec.Body.String(), "Not found: DataSource ds1") {
		t.Errorf("body missing id; got %s", rec.Body.String())
	}
}

func TestDataTransferConfigListEmptyPage(t *testing.T) {
	rec := httptest.NewRecorder()
	DataTransferConfigList(Dependencies{})(rec,
		newDataTransferReq(http.MethodGet, "/v1/projects/p/transferConfigs",
			map[string]string{"projectId": "p"}))

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	var got struct {
		TransferConfigs []any `json:"transferConfigs"`
	}
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if len(got.TransferConfigs) != 0 {
		t.Errorf("transferConfigs = %v, want empty page", got.TransferConfigs)
	}
}

func TestDataTransferConfigGetReturns404(t *testing.T) {
	rec := httptest.NewRecorder()
	DataTransferConfigGet(Dependencies{})(rec,
		newDataTransferReq(http.MethodGet, "/v1/projects/p/transferConfigs/c1",
			map[string]string{"projectId": "p", "configId": "c1"}))

	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404; body=%s", rec.Code, rec.Body.String())
	}
}

func TestDataTransferConfigCreateReturns501(t *testing.T) {
	rec := httptest.NewRecorder()
	DataTransferConfigCreate(Dependencies{})(rec,
		newDataTransferReq(http.MethodPost, "/v1/projects/p/transferConfigs",
			map[string]string{"projectId": "p"}))

	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want 501; body=%s", rec.Code, rec.Body.String())
	}
}
