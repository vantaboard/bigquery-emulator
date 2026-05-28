//go:build integration

// Copyright 2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package e2e

import (
	"encoding/json"
	"net/http"
	"strings"
	"testing"
)

// TestDataTransferCRUDOverGateway exercises the full datatransfer
// REST surface end-to-end through the live emulator gateway: create
// a transferConfig, list it, get it by name, patch its `disabled`
// field both directions (the
// DisableTransferConfigIT/ReEnableTransferConfigIT path), then
// delete it and confirm the GET returns 404. Mirrors the catalog
// shape of `gateway/e2e/storage_read_test.go` so the lane stays
// uniform.
func TestDataTransferCRUDOverGateway(t *testing.T) {
	env := startEmulator(t)
	const project = "proj-dts-e2e"

	create := `{"displayName":"q","dataSourceId":"scheduled_query","destinationDatasetId":"ds1","params":{"query":"SELECT 1"}}`
	createURL := env.URL() + "/v1/projects/" + project + "/locations/us/transferConfigs"
	statusCode, body := doJSON(t, http.MethodPost, createURL, []byte(create))
	if statusCode != http.StatusOK {
		t.Fatalf("create: %d body=%s", statusCode, string(body))
	}
	var created map[string]any
	if err := json.Unmarshal(body, &created); err != nil {
		t.Fatalf("decode create body: %v", err)
	}
	name, _ := created["name"].(string)
	if name == "" {
		t.Fatalf("create: empty name; body=%v", created)
	}
	if !strings.HasPrefix(name, "projects/"+project+"/locations/us/transferConfigs/") {
		t.Errorf("name shape = %q, want it under projects/%s/locations/us/transferConfigs/",
			name, project)
	}

	listURL := createURL
	statusCode, body = doJSON(t, http.MethodGet, listURL, nil)
	if statusCode != http.StatusOK {
		t.Fatalf("list: %d body=%s", statusCode, string(body))
	}

	getURL := env.URL() + "/v1/" + name
	statusCode, body = doJSON(t, http.MethodGet, getURL, nil)
	if statusCode != http.StatusOK {
		t.Fatalf("get: %d body=%s", statusCode, string(body))
	}

	statusCode, body = doJSON(t, http.MethodPatch, getURL, []byte(`{"disabled":true}`))
	if statusCode != http.StatusOK {
		t.Fatalf("patch disabled=true: %d body=%s", statusCode, string(body))
	}
	var patched map[string]any
	if err := json.Unmarshal(body, &patched); err != nil {
		t.Fatalf("decode patch body: %v", err)
	}
	if got := patched["disabled"]; got != true {
		t.Errorf("patch disabled=true: response disabled=%v, want true", got)
	}

	statusCode, body = doJSON(t, http.MethodPatch, getURL, []byte(`{"disabled":false}`))
	if statusCode != http.StatusOK {
		t.Fatalf("patch disabled=false: %d body=%s", statusCode, string(body))
	}

	statusCode, _ = doJSON(t, http.MethodDelete, getURL, nil)
	if statusCode != http.StatusOK {
		t.Errorf("delete: %d", statusCode)
	}

	statusCode, _ = doJSON(t, http.MethodGet, getURL, nil)
	if statusCode != http.StatusNotFound {
		t.Errorf("get after delete: %d, want 404", statusCode)
	}
}

// TestDataTransferDataSourceCatalogOverGateway pins the seed
// dataSources catalog (scheduled_query + amazon_s3) over the live
// gateway. Phase A row 13 (CreateAmazonS3TransferIT) needs the
// catalog probe to surface the amazon_s3 entry before the IT
// proceeds to POST /transferConfigs.
func TestDataTransferDataSourceCatalogOverGateway(t *testing.T) {
	env := startEmulator(t)
	url := env.URL() + "/v1/projects/proj-dts-cat/locations/us/dataSources"
	statusCode, body := doJSON(t, http.MethodGet, url, nil)
	if statusCode != http.StatusOK {
		t.Fatalf("list dataSources: %d body=%s", statusCode, string(body))
	}
	var got map[string]any
	if err := json.Unmarshal(body, &got); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	ds, _ := got["dataSources"].([]any)
	if len(ds) < 2 {
		t.Fatalf("dataSources = %d entries, want >=2; body=%s", len(ds), string(body))
	}
	ids := map[string]struct{}{}
	for _, row := range ds {
		m, _ := row.(map[string]any)
		if id, _ := m["dataSourceId"].(string); id != "" {
			ids[id] = struct{}{}
		}
	}
	for _, want := range []string{"scheduled_query", "amazon_s3"} {
		if _, ok := ids[want]; !ok {
			t.Errorf("dataSources missing %q (got %v)", want, ids)
		}
	}
}
