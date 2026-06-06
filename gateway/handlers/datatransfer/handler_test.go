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

package datatransfer_test

import (
	"bytes"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/handlers/datatransfer"
)

const (
	pathP1USConfigs                 = "/v1/projects/p1/locations/us/transferConfigs"
	jsonCreateScheduledQuerySelect1 = `{"displayName":"q","dataSourceId":"scheduled_query","params":{"query":"SELECT 1"}}`
)

// Test-fixture mirrors of the datatransfer package's unexported
// dataSourceID consts. Kept in lockstep with the production values in
// gateway/handlers/datatransfer/handler.go; goconst flagged the
// scheduled-query / amazon-s3 spellings here without a single source
// of truth.
const (
	dataSourceScheduledQuery = "scheduled_query"
	dataSourceAmazonS3       = "amazon_s3"
)

func newTestMux(t *testing.T) *http.ServeMux {
	t.Helper()
	mux := http.NewServeMux()
	datatransfer.NewHandler(nil).Register(mux)
	return mux
}

func postJSON(t *testing.T, mux *http.ServeMux, path, body string) *httptest.ResponseRecorder {
	t.Helper()
	req := httptest.NewRequest(http.MethodPost, path, bytes.NewReader([]byte(body)))
	req.Header.Set("Content-Type", "application/json")
	rec := httptest.NewRecorder()
	mux.ServeHTTP(rec, req)
	return rec
}

func decodeMap(t *testing.T, rec *httptest.ResponseRecorder) map[string]any {
	t.Helper()
	var got map[string]any
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode body: %v; raw=%q", err, rec.Body.String())
	}
	return got
}

// TestDataTransferCreateGetListConfig pins the location-scoped happy
// path: create returns a name + state SUCCEEDED, list returns the
// config, get returns it by id.
func TestDataTransferCreateGetListConfig(t *testing.T) {
	mux := newTestMux(t)

	rec := postJSON(t, mux, pathP1USConfigs, jsonCreateScheduledQuerySelect1)
	if rec.Code != http.StatusOK {
		t.Fatalf("create: status=%d body=%s", rec.Code, rec.Body.String())
	}
	created := decodeMap(t, rec)
	name, _ := created["name"].(string)
	if name == "" {
		t.Fatalf("create: empty name; body=%v", created)
	}
	if got := created["state"]; got != "SUCCEEDED" {
		t.Errorf("create: state=%v, want SUCCEEDED", got)
	}

	req := httptest.NewRequest(http.MethodGet, pathP1USConfigs, nil)
	rec2 := httptest.NewRecorder()
	mux.ServeHTTP(rec2, req)
	if rec2.Code != http.StatusOK {
		t.Fatalf("list: status=%d body=%s", rec2.Code, rec2.Body.String())
	}
	listBody := decodeMap(t, rec2)
	cfgs, _ := listBody["transferConfigs"].([]any)
	if len(cfgs) != 1 {
		t.Errorf("list: %d configs, want 1", len(cfgs))
	}

	getReq := httptest.NewRequest(http.MethodGet, "/v1/"+name, nil)
	rec3 := httptest.NewRecorder()
	mux.ServeHTTP(rec3, getReq)
	if rec3.Code != http.StatusOK {
		t.Fatalf("get: status=%d body=%s", rec3.Code, rec3.Body.String())
	}
}

// TestDataTransferPatchDisabledRoundTrip pins the
// DisableTransferConfigIT / ReEnableTransferConfigIT fix: the
// `disabled` field on PATCH must round-trip through *bool semantics
// so an explicit `false` re-enables. The empty-mask "implicit"
// disabled inheritance is allowed (gapic clients sometimes send the
// full record on patch).
func TestDataTransferPatchDisabledRoundTrip(t *testing.T) {
	mux := newTestMux(t)

	rec := postJSON(t, mux, pathP1USConfigs, jsonCreateScheduledQuerySelect1)
	if rec.Code != http.StatusOK {
		t.Fatalf("create: status=%d body=%s", rec.Code, rec.Body.String())
	}
	created := decodeMap(t, rec)
	name, _ := created["name"].(string)

	// Disable.
	patch1 := postPatch(t, mux, "/v1/"+name, `{"disabled":true}`)
	if got := patch1["disabled"]; got != true {
		t.Errorf("patch disabled=true round-trip: got %v, want true", got)
	}

	// Re-enable.
	patch2 := postPatch(t, mux, "/v1/"+name, `{"disabled":false}`)
	if got := patch2["disabled"]; got != false && got != nil {
		t.Errorf("patch disabled=false round-trip: got %v, want false (or omitted)",
			got)
	}
}

func postPatch(t *testing.T, mux *http.ServeMux, path, body string) map[string]any {
	t.Helper()
	req := httptest.NewRequest(http.MethodPatch, path, bytes.NewReader([]byte(body)))
	req.Header.Set("Content-Type", "application/json")
	rec := httptest.NewRecorder()
	mux.ServeHTTP(rec, req)
	if rec.Code != http.StatusOK {
		t.Fatalf("patch %s: status=%d body=%s", path, rec.Code, rec.Body.String())
	}
	return decodeMap(t, rec)
}

// TestDataTransferGoSampleListFilterAndSucceededRun mirrors the Go
// sample integration flow: project-scoped create with
// destinationDatasetId + schedule, list with dataSourceIds filter,
// and a seeded SUCCEEDED transfer run for waitTransferRun.
func TestDataTransferGoSampleListFilterAndSucceededRun(t *testing.T) {
	mux := newTestMux(t)
	body := `{"displayName":"Your Scheduled Query Name","dataSourceId":"scheduled_query",` +
		`"destinationDatasetId":"ds1","schedule":"every 24 hours",` +
		`"params":{"destination_table_name_template":"my_destination_table_{run_date}",` +
		`"write_disposition":"WRITE_TRUNCATE","partitioning_field":"","query":"SELECT 1"}}`
	rec := postJSON(t, mux, "/v1/projects/p1/transferConfigs", body)
	if rec.Code != http.StatusOK {
		t.Fatalf("create: status=%d body=%s", rec.Code, rec.Body.String())
	}
	created := decodeMap(t, rec)
	name, _ := created["name"].(string)
	if name == "" {
		t.Fatalf("create: empty name")
	}
	if got, _ := created["destinationDatasetId"].(string); got != "ds1" {
		t.Errorf("destinationDatasetId = %q, want ds1", got)
	}

	listReq := httptest.NewRequest(
		http.MethodGet,
		"/v1/projects/p1/transferConfigs?dataSourceIds=scheduled_query",
		nil,
	)
	listRec := httptest.NewRecorder()
	mux.ServeHTTP(listRec, listReq)
	if listRec.Code != http.StatusOK {
		t.Fatalf("list: status=%d body=%s", listRec.Code, listRec.Body.String())
	}
	listBody := decodeMap(t, listRec)
	cfgs, _ := listBody["transferConfigs"].([]any)
	if len(cfgs) != 1 {
		t.Fatalf("list filtered: %d configs, want 1", len(cfgs))
	}

	otherReq := httptest.NewRequest(
		http.MethodGet,
		"/v1/projects/p1/transferConfigs?dataSourceIds=amazon_s3",
		nil,
	)
	otherRec := httptest.NewRecorder()
	mux.ServeHTTP(otherRec, otherReq)
	otherBody := decodeMap(t, otherRec)
	otherCfgs, _ := otherBody["transferConfigs"].([]any)
	if len(otherCfgs) != 0 {
		t.Errorf("list amazon_s3 filter: %d configs, want 0", len(otherCfgs))
	}

	runsReq := httptest.NewRequest(http.MethodGet, "/v1/"+name+"/runs", nil)
	runsRec := httptest.NewRecorder()
	mux.ServeHTTP(runsRec, runsReq)
	if runsRec.Code != http.StatusOK {
		t.Fatalf("list runs: status=%d body=%s", runsRec.Code, runsRec.Body.String())
	}
	runsBody := decodeMap(t, runsRec)
	runs, _ := runsBody["transferRuns"].([]any)
	if len(runs) != 1 {
		t.Fatalf("transferRuns = %d, want 1", len(runs))
	}
	run, _ := runs[0].(map[string]any)
	if got := run["state"]; got != "SUCCEEDED" {
		t.Errorf("run state = %v, want SUCCEEDED", got)
	}
}

// TestDataTransferProjectScopedCreateRoutesToUS pins the gapic
// project-scoped path: POST /v1/projects/{p}/transferConfigs (no
// /locations/) lands the config at /locations/us/.
func TestDataTransferProjectScopedCreateRoutesToUS(t *testing.T) {
	mux := newTestMux(t)
	rec := postJSON(
		t,
		mux,
		"/v1/projects/p1/transferConfigs",
		`{"displayName":"q","dataSourceId":"scheduled_query","destinationDatasetId":"ds1","params":{"query":"SELECT 1"}}`,
	)
	if rec.Code != http.StatusOK {
		t.Fatalf("create: status=%d body=%s", rec.Code, rec.Body.String())
	}
	created := decodeMap(t, rec)
	name, _ := created["name"].(string)
	if !strings.Contains(name, "/locations/us/transferConfigs/") {
		t.Errorf("name = %q, want it to be under /locations/us/", name)
	}
}

// TestDataTransferAmazonS3Create pins the happy path for the
// CreateAmazonS3TransferIT failing-IT row 13. Once env vars are
// stubbed and the gRPC
// transport is HTTP/JSON-shaped, this is the route the IT lands on.
// We accept both project-scoped (no location segment, gapic Go REST)
// and location-scoped (gapic Java) shapes.
func TestDataTransferAmazonS3Create(t *testing.T) {
	mux := newTestMux(t)
	body := `{"displayName":"s3","dataSourceId":"amazon_s3","destinationDatasetId":"ds1",` +
		`"schedule":"every 24 hours","params":{"data_path":"s3://bucket/*","file_format":"CSV"}}`
	rec := postJSON(t, mux, pathP1USConfigs, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("create: status=%d body=%s", rec.Code, rec.Body.String())
	}
	created := decodeMap(t, rec)
	name, _ := created["name"].(string)
	if name == "" {
		t.Fatalf("create: empty name")
	}
	if ds := created["dataSourceId"]; ds != dataSourceAmazonS3 {
		t.Errorf("dataSourceId = %v, want amazon_s3", ds)
	}
}

// TestDataTransferDeleteConfig pins delete-then-404. Drives the
// CreateAmazonS3TransferIT @After teardown path.
func TestDataTransferDeleteConfig(t *testing.T) {
	mux := newTestMux(t)
	rec := postJSON(t, mux, pathP1USConfigs, jsonCreateScheduledQuerySelect1)
	if rec.Code != http.StatusOK {
		t.Fatalf("create: status=%d body=%s", rec.Code, rec.Body.String())
	}
	created := decodeMap(t, rec)
	name, _ := created["name"].(string)

	delReq := httptest.NewRequest(http.MethodDelete, "/v1/"+name, nil)
	delRec := httptest.NewRecorder()
	mux.ServeHTTP(delRec, delReq)
	if delRec.Code != http.StatusOK {
		t.Fatalf("delete: status=%d body=%s", delRec.Code, delRec.Body.String())
	}

	getReq := httptest.NewRequest(http.MethodGet, "/v1/"+name, nil)
	getRec := httptest.NewRecorder()
	mux.ServeHTTP(getRec, getReq)
	if getRec.Code != http.StatusNotFound {
		t.Errorf("get after delete: status=%d, want 404", getRec.Code)
	}
}

// TestDataTransferListDataSources pins the seed dataSources catalog
// (scheduled_query + amazon_s3).
func TestDataTransferListDataSources(t *testing.T) {
	mux := newTestMux(t)
	req := httptest.NewRequest(http.MethodGet, "/v1/projects/p1/locations/us/dataSources", nil)
	rec := httptest.NewRecorder()
	mux.ServeHTTP(rec, req)
	if rec.Code != http.StatusOK {
		t.Fatalf("list dataSources: status=%d body=%s", rec.Code, rec.Body.String())
	}
	body := decodeMap(t, rec)
	ds, _ := body["dataSources"].([]any)
	if len(ds) < 2 {
		t.Errorf("dataSources = %d, want >=2", len(ds))
	}
	ids := map[string]struct{}{}
	for _, row := range ds {
		m, _ := row.(map[string]any)
		if id, _ := m["dataSourceId"].(string); id != "" {
			ids[id] = struct{}{}
		}
	}
	for _, want := range []string{dataSourceScheduledQuery, dataSourceAmazonS3} {
		if _, ok := ids[want]; !ok {
			t.Errorf("dataSources missing %q (got %v)", want, ids)
		}
	}
}

// TestDataTransferListDataSourcesThirdPartyConnectors pins the
// catalog extension: the 8 third-party connector IDs that the
// upstream `Create*Transfer.java` driver classes send on
// CreateTransferConfig must all surface from `dataSources.list` so
// the catalog accurately represents what the emulator accepts. The
// IDs match the `setDataSourceId(...)` literals in each driver (see
// CreateAdManagerTransfer → `dfp_dt`, CreateAdsTransfer → `adwords`,
// CreateTeradataTransfer → `on_premises`).
func TestDataTransferListDataSourcesThirdPartyConnectors(t *testing.T) {
	mux := newTestMux(t)
	req := httptest.NewRequest(http.MethodGet, "/v1/projects/p1/locations/us/dataSources", nil)
	rec := httptest.NewRecorder()
	mux.ServeHTTP(rec, req)
	if rec.Code != http.StatusOK {
		t.Fatalf("list dataSources: status=%d body=%s", rec.Code, rec.Body.String())
	}
	body := decodeMap(t, rec)
	ds, _ := body["dataSources"].([]any)
	ids := map[string]struct{}{}
	for _, row := range ds {
		m, _ := row.(map[string]any)
		if id, _ := m["dataSourceId"].(string); id != "" {
			ids[id] = struct{}{}
		}
	}
	wantIDs := []string{
		// Initial shallow-emulator baseline.
		dataSourceScheduledQuery,
		dataSourceAmazonS3,
		// Third-party connector stubs.
		"dfp_dt",
		"adwords",
		"dcm_dt",
		"play",
		"redshift",
		"on_premises",
		"youtube_channel",
		"youtube_content_owner",
	}
	for _, want := range wantIDs {
		if _, ok := ids[want]; !ok {
			t.Errorf("dataSources missing %q (got %v)", want, ids)
		}
	}
}

// TestDataTransferCreateConfigThirdPartyConnectorsSucceed pins that
// CreateTransferConfig succeeds for each of the 8 third-party
// connector IDs. The emulator does not perform the transfer; it only
// persists the config so the IT's GetTransferConfig probe finds it.
// This is the REST-side surface contract the 8 Create*TransferIT
// ITs depend on (the gapic clients ultimately dial gRPC, which is
// the gRPC-server follow-up; this test fixes the REST shape today).
func TestDataTransferCreateConfigThirdPartyConnectorsSucceed(t *testing.T) {
	mux := newTestMux(t)
	for _, ds := range []string{
		"dfp_dt",
		"adwords",
		"dcm_dt",
		"play",
		"redshift",
		"on_premises",
		"youtube_channel",
		"youtube_content_owner",
	} {
		body := `{"displayName":"x","dataSourceId":"` + ds +
			`","destinationDatasetId":"ds1","params":{"k":"v"}}`
		rec := postJSON(t, mux, pathP1USConfigs, body)
		if rec.Code != http.StatusOK {
			t.Errorf("create %q: status=%d body=%s", ds, rec.Code, rec.Body.String())
			continue
		}
		created := decodeMap(t, rec)
		if got, _ := created["dataSourceId"].(string); got != ds {
			t.Errorf("create %q: dataSourceId=%v, want %q", ds, got, ds)
		}
		name, _ := created["name"].(string)
		if name == "" {
			t.Errorf("create %q: empty name", ds)
		}
	}
}

// TestDataTransferGetDataSource_AmazonS3AuthorizationPlaceholder pins
// the inert .invalid placeholder URL on the amazon_s3 catalog row.
func TestDataTransferGetDataSourceAmazonS3AuthorizationPlaceholder(t *testing.T) {
	mux := newTestMux(t)
	req := httptest.NewRequest(http.MethodGet, "/v1/projects/p1/locations/us/dataSources/amazon_s3", nil)
	rec := httptest.NewRecorder()
	mux.ServeHTTP(rec, req)
	if rec.Code != http.StatusOK {
		t.Fatalf("get dataSource: status=%d body=%s", rec.Code, rec.Body.String())
	}
	body := decodeMap(t, rec)
	if got, _ := body["dataSourceId"].(string); got != dataSourceAmazonS3 {
		t.Errorf("dataSourceId = %q, want amazon_s3", got)
	}
	if got, _ := body["authorizationUrl"].(string); !strings.Contains(got, "oauth-emulator.invalid") {
		t.Errorf("authorizationUrl = %q, want it to contain oauth-emulator.invalid", got)
	}
}

// TestDataTransferGetDataSourceNotFound pins 404 for unknown
// dataSource ids.
func TestDataTransferGetDataSourceNotFound(t *testing.T) {
	mux := newTestMux(t)
	req := httptest.NewRequest(http.MethodGet, "/v1/projects/p1/locations/us/dataSources/no_such_connector", nil)
	rec := httptest.NewRecorder()
	mux.ServeHTTP(rec, req)
	if rec.Code != http.StatusNotFound {
		t.Fatalf("status=%d, want 404; body=%s", rec.Code, rec.Body.String())
	}
}

// TestDataTransferCheckValidCredsNoLiveOAuth pins the deterministic
// `hasValidCreds: false` response for the AIP-136 :checkValidCreds
// custom method. The emulator never performs real OAuth.
func TestDataTransferCheckValidCredsNoLiveOAuth(t *testing.T) {
	mux := newTestMux(t)
	rec := postJSON(t, mux, pathP1USConfigs, jsonCreateScheduledQuerySelect1)
	if rec.Code != http.StatusOK {
		t.Fatalf("create: status=%d body=%s", rec.Code, rec.Body.String())
	}
	created := decodeMap(t, rec)
	name, _ := created["name"].(string)

	rec2 := postJSON(t, mux, "/v1/"+name+":checkValidCreds", "{}")
	if rec2.Code != http.StatusOK {
		t.Fatalf(":checkValidCreds: status=%d body=%s", rec2.Code, rec2.Body.String())
	}
	creds := decodeMap(t, rec2)
	if got := creds["hasValidCreds"]; got != false {
		t.Errorf("hasValidCreds = %v, want false", got)
	}
}

// TestDataTransferStartManualRunsUnsupportedWithoutRunner pins the
// 501 for :startManualRuns when no Runner is wired (the shallow
// emulator keeps the SQL execution path off; the SQL runner wires it).
func TestDataTransferStartManualRunsUnsupportedWithoutRunner(t *testing.T) {
	mux := newTestMux(t)
	rec := postJSON(t, mux, pathP1USConfigs, jsonCreateScheduledQuerySelect1)
	if rec.Code != http.StatusOK {
		t.Fatalf("create: status=%d body=%s", rec.Code, rec.Body.String())
	}
	created := decodeMap(t, rec)
	name, _ := created["name"].(string)

	rec2 := postJSON(t, mux, "/v1/"+name+":startManualRuns", "{}")
	if rec2.Code != http.StatusNotImplemented {
		t.Fatalf(":startManualRuns: status=%d, want 501; body=%s", rec2.Code, rec2.Body.String())
	}
}

// TestDataTransferScheduleRunsUnsupported pins :scheduleRuns as 501
// (no backfill / cron).
func TestDataTransferScheduleRunsUnsupported(t *testing.T) {
	mux := newTestMux(t)
	rec := postJSON(t, mux, pathP1USConfigs, `{"displayName":"q","dataSourceId":"scheduled_query"}`)
	if rec.Code != http.StatusOK {
		t.Fatalf("create: status=%d body=%s", rec.Code, rec.Body.String())
	}
	created := decodeMap(t, rec)
	name, _ := created["name"].(string)

	rec2 := postJSON(t, mux, "/v1/"+name+":scheduleRuns", "{}")
	if rec2.Code != http.StatusNotImplemented {
		t.Fatalf(":scheduleRuns: status=%d, want 501; body=%s", rec2.Code, rec2.Body.String())
	}
}

// TestDataTransferCreateRunUnsupportedNonScheduledDataSource pins
// the documented-but-not-implemented behavior for amazon_s3 manual
// runs: the metadata catalog accepts the config but POST /runs is
// 501.
func TestDataTransferCreateRunUnsupportedNonScheduledDataSource(t *testing.T) {
	mux := newTestMux(t)
	rec := postJSON(t, mux, pathP1USConfigs,
		`{"displayName":"s3","dataSourceId":"amazon_s3","params":{"data_path":"s3://b/*"}}`)
	if rec.Code != http.StatusOK {
		t.Fatalf("create: status=%d body=%s", rec.Code, rec.Body.String())
	}
	created := decodeMap(t, rec)
	name, _ := created["name"].(string)

	req := httptest.NewRequest(http.MethodPost, "/v1/"+name+"/runs", nil)
	rec2 := httptest.NewRecorder()
	mux.ServeHTTP(rec2, req)
	if rec2.Code != http.StatusNotImplemented {
		t.Fatalf(".../runs: status=%d, want 501; body=%s", rec2.Code, rec2.Body.String())
	}
}

// TestDataTransferDataSourceCatalogExtras pins the merge semantics:
// caller-provided extras + builtin entries surface together in the
// dataSources list.
func TestDataTransferDataSourceCatalogExtras(t *testing.T) {
	mux := http.NewServeMux()
	h := datatransfer.NewHandler(nil)
	h.DataSourceCatalogExtras = []datatransfer.DataSourceCatalogEntry{
		{DataSourceID: "custom_stub", DisplayName: "Custom stub", Description: "from extras"},
	}
	h.Register(mux)
	req := httptest.NewRequest(http.MethodGet, "/v1/projects/p1/locations/eu/dataSources", nil)
	rec := httptest.NewRecorder()
	mux.ServeHTTP(rec, req)
	if rec.Code != http.StatusOK {
		t.Fatalf("status=%d body=%s", rec.Code, rec.Body.String())
	}
	body := decodeMap(t, rec)
	list, _ := body["dataSources"].([]any)
	ids := map[string]struct{}{}
	for _, row := range list {
		m, _ := row.(map[string]any)
		if id, _ := m["dataSourceId"].(string); id != "" {
			ids[id] = struct{}{}
		}
	}
	if _, ok := ids["custom_stub"]; !ok {
		t.Errorf("missing extras id `custom_stub` (got %v)", ids)
	}
	if _, ok := ids["scheduled_query"]; !ok {
		t.Errorf("missing builtin id `scheduled_query` (got %v)", ids)
	}
}
