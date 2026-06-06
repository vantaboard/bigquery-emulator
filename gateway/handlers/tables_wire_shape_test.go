package handlers

import (
	"context"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

// TestTableInsertExpirationTimeNumeric mirrors tables.createTable when the
// Node client posts expirationTime as a JSON number.
func TestTableInsertExpirationTimeNumeric(t *testing.T) {
	store := NewMetadataStore()
	fake := &fakeCatalogClient{}
	deps := Dependencies{Catalog: fake, Metadata: store}

	insert := newTableReq(http.MethodPost, "",
		`{"tableReference":{"tableId":"`+testTableID+`"},"expirationTime":1234567890}`)
	rec := httptest.NewRecorder()
	TableInsert(deps)(rec, insert)
	if rec.Code != http.StatusOK {
		t.Fatalf("insert: status=%d body=%s", rec.Code, rec.Body.String())
	}
	var got bqtypes.Table
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.ExpirationTime.String() != "1234567890" {
		t.Errorf("expirationTime = %q, want %q", got.ExpirationTime, "1234567890")
	}
}

// TestTablePatchDeleteLabelResponseShape mirrors deleteLabelTable.
func TestTablePatchDeleteLabelResponseShape(t *testing.T) {
	store := NewMetadataStore()
	fake := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, _ *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			return &enginepb.DescribeTableResponse{}, nil
		},
	}
	deps := Dependencies{Catalog: fake, Metadata: store}

	insert := newTableReq(http.MethodPost, "", `{"tableReference":{"tableId":"`+testTableID+`"}}`)
	rec := httptest.NewRecorder()
	TableInsert(deps)(rec, insert)
	if rec.Code != http.StatusOK {
		t.Fatalf("insert: status=%d body=%s", rec.Code, rec.Body.String())
	}

	labelPatch := newTableReq(http.MethodPatch, testTableID, `{"labels":{"color":"green"}}`)
	rec = httptest.NewRecorder()
	TablePatch(deps)(rec, labelPatch)
	if rec.Code != http.StatusOK {
		t.Fatalf("label patch: status=%d body=%s", rec.Code, rec.Body.String())
	}

	deletePatch := newTableReq(http.MethodPatch, testTableID, `{"labels":{"color":null}}`)
	rec = httptest.NewRecorder()
	TablePatch(deps)(rec, deletePatch)
	if rec.Code != http.StatusOK {
		t.Fatalf("delete label patch: status=%d body=%s", rec.Code, rec.Body.String())
	}

	var doc map[string]any
	if err := json.Unmarshal(rec.Body.Bytes(), &doc); err != nil {
		t.Fatalf("decode patch response: %v", err)
	}
	if labels, present := doc["labels"]; present {
		obj, ok := labels.(map[string]any)
		if !ok {
			t.Fatalf("labels is %T, want map[string]any", labels)
		}
		if len(obj) != 0 {
			t.Errorf("labels = %v, want empty or omitted", obj)
		}
	}
}
