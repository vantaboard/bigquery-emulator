package handlers

import (
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strconv"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func TestDatasetMetadataDefaultRoundingModeRoundTrip(t *testing.T) {
	runDatasetMetadataRoundTrip(
		t,
		`{"defaultRoundingMode":"ROUND_HALF_EVEN"}`,
		func(t *testing.T, got bqtypes.Dataset) {
			if got.DefaultRoundingMode != "ROUND_HALF_EVEN" {
				t.Errorf("defaultRoundingMode = %q, want ROUND_HALF_EVEN", got.DefaultRoundingMode)
			}
		},
	)
}

func TestDatasetMetadataMaxTimeTravelHoursRoundTrip(t *testing.T) {
	runDatasetMetadataRoundTrip(t, `{"maxTimeTravelHours":"168"}`, func(t *testing.T, got bqtypes.Dataset) {
		if got.MaxTimeTravelHours != "168" {
			t.Errorf("maxTimeTravelHours = %q, want 168", got.MaxTimeTravelHours)
		}
	})
}

func TestDatasetMetadataIsCaseInsensitiveRoundTrip(t *testing.T) {
	runDatasetMetadataRoundTrip(t, `{"isCaseInsensitive":true}`, func(t *testing.T, got bqtypes.Dataset) {
		if got.IsCaseInsensitive == nil || !*got.IsCaseInsensitive {
			t.Errorf("isCaseInsensitive = %v, want true", got.IsCaseInsensitive)
		}
	})
}

func TestDatasetMetadataResourceTagsRoundTrip(t *testing.T) {
	runDatasetMetadataRoundTrip(t,
		`{"resourceTags":{"123456789012/env":"Production"}}`,
		func(t *testing.T, got bqtypes.Dataset) {
			if got.ResourceTags["123456789012/env"] != "Production" {
				t.Errorf("resourceTags = %v", got.ResourceTags)
			}
		},
	)
}

func TestDatasetMetadataReplicasRoundTrip(t *testing.T) {
	runDatasetMetadataRoundTrip(t,
		`{"replicas":[{"projectId":"p","datasetId":"d","tableId":"t"}]}`,
		func(t *testing.T, got bqtypes.Dataset) {
			if len(got.Replicas) != 1 {
				t.Fatalf("replicas = %+v, want one entry", got.Replicas)
			}
			if got.Replicas[0].TableID != "t" {
				t.Errorf("replicas[0].tableId = %q, want t", got.Replicas[0].TableID)
			}
		},
	)
}

func TestDatasetMetadataDefaultTableExpirationRoundTrip(t *testing.T) {
	const ms = "3600000"
	runDatasetMetadataRoundTrip(t, `{"defaultTableExpirationMs":"`+ms+`"}`, func(t *testing.T, got bqtypes.Dataset) {
		if got.DefaultTableExpirationMs != ms {
			t.Errorf("defaultTableExpirationMs = %q, want %q", got.DefaultTableExpirationMs, ms)
		}
	})
}

func TestDatasetTimestampsPersistAcrossGet(t *testing.T) {
	store := NewMetadataStore()
	deps := Dependencies{Catalog: &fakeCatalogClient{}, Metadata: store}

	insert := newDatasetReq(http.MethodPost, "",
		`{"datasetReference":{"datasetId":"`+testDatasetID+`"}}`)
	rec := httptest.NewRecorder()
	DatasetInsert(deps)(rec, insert)
	if rec.Code != http.StatusOK {
		t.Fatalf("insert: status=%d body=%s", rec.Code, rec.Body.String())
	}
	var inserted bqtypes.Dataset
	if err := json.NewDecoder(rec.Body).Decode(&inserted); err != nil {
		t.Fatalf("decode insert: %v", err)
	}
	if inserted.CreationTime == "" {
		t.Fatal("insert creationTime empty")
	}

	get := newDatasetReq(http.MethodGet, testDatasetID, "")
	rec = httptest.NewRecorder()
	DatasetGet(deps)(rec, get)
	if rec.Code != http.StatusOK {
		t.Fatalf("get: status=%d body=%s", rec.Code, rec.Body.String())
	}
	var got bqtypes.Dataset
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode get: %v", err)
	}
	if got.CreationTime != inserted.CreationTime {
		t.Errorf("creationTime = %q, want stable %q", got.CreationTime, inserted.CreationTime)
	}
	if got.LastModifiedTime != inserted.LastModifiedTime {
		t.Errorf("lastModifiedTime = %q, want stable %q on read", got.LastModifiedTime, inserted.LastModifiedTime)
	}
}

func TestDatasetLastModifiedTimeBumpsOnPatch(t *testing.T) {
	store := NewMetadataStore()
	deps := Dependencies{Catalog: &fakeCatalogClient{}, Metadata: store}

	insert := newDatasetReq(http.MethodPost, "",
		`{"datasetReference":{"datasetId":"`+testDatasetID+`"}}`)
	rec := httptest.NewRecorder()
	DatasetInsert(deps)(rec, insert)
	var inserted bqtypes.Dataset
	_ = json.NewDecoder(rec.Body).Decode(&inserted)

	patchReq := newDatasetReq(http.MethodPatch, testDatasetID, `{"friendlyName":"renamed"}`)
	rec = httptest.NewRecorder()
	DatasetPatch(deps)(rec, patchReq)
	if rec.Code != http.StatusOK {
		t.Fatalf("patch: status=%d body=%s", rec.Code, rec.Body.String())
	}
	var patched bqtypes.Dataset
	if err := json.NewDecoder(rec.Body).Decode(&patched); err != nil {
		t.Fatalf("decode patch: %v", err)
	}
	if patched.CreationTime != inserted.CreationTime {
		t.Errorf("creationTime = %q, want %q", patched.CreationTime, inserted.CreationTime)
	}
	insertedMod, _ := strconv.ParseInt(inserted.LastModifiedTime, 10, 64)
	patchedMod, _ := strconv.ParseInt(patched.LastModifiedTime, 10, 64)
	if patchedMod < insertedMod {
		t.Errorf("lastModifiedTime did not advance: before=%q after=%q",
			inserted.LastModifiedTime, patched.LastModifiedTime)
	}
}
