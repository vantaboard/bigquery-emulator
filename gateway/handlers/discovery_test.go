package handlers

import (
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"testing"
)

// TestDiscoveryShape asserts the discovery handler returns 200 and a
// minimally-valid Google API restDescription document. The
// `jq .kind` verification command in the plan depends on `kind` being
// exactly "discovery#restDescription".
func TestDiscoveryShape(t *testing.T) {
	rec := httptest.NewRecorder()
	req := httptest.NewRequest(http.MethodGet, "/discovery/v1/apis/bigquery/v2/rest", nil)
	Discovery(Dependencies{})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200", rec.Code)
	}
	if ct := rec.Header().Get("Content-Type"); ct != "application/json; charset=utf-8" {
		t.Fatalf("Content-Type = %q, want application/json", ct)
	}

	var doc map[string]any
	if err := json.NewDecoder(rec.Body).Decode(&doc); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if got := doc["kind"]; got != "discovery#restDescription" {
		t.Fatalf("kind = %v, want %q", got, "discovery#restDescription")
	}
	if got := doc["name"]; got != "bigquery" {
		t.Fatalf("name = %v, want %q", got, "bigquery")
	}
	if got := doc["version"]; got != "v2" {
		t.Fatalf("version = %v, want %q", got, "v2")
	}
	if _, ok := doc["resources"]; !ok {
		t.Fatal("resources field is missing; clients need it to enumerate methods")
	}
}

// TestDiscoveryLists routed methods walks the static document and
// checks every BigQuery resource we wire in gateway/server.go has at
// least one method entry. This is what guards docs/REST_API.md from
// drifting out of sync with the discovery document.
func TestDiscoveryListsRoutedMethods(t *testing.T) {
	doc := buildDiscoveryDocument()

	wantResources := []string{"projects", "datasets", "tables", "tabledata", "jobs"}
	for _, name := range wantResources {
		res, ok := doc.Resources[name]
		if !ok {
			t.Errorf("resource %q missing from discovery doc", name)
			continue
		}
		if len(res.Methods) == 0 {
			t.Errorf("resource %q has zero methods; discovery doc must list each routed method", name)
		}
	}

	// Spot-check a few representative method ids so a refactor that
	// drops a method without updating this file gets caught.
	wantMethodIDs := []string{
		"bigquery.projects.list",
		"bigquery.projects.getServiceAccount",
		"bigquery.datasets.undelete",
		"bigquery.tables.getIamPolicy",
		"bigquery.tabledata.insertAll",
		"bigquery.jobs.query",
		"bigquery.jobs.getQueryResults",
	}
	have := map[string]bool{}
	for _, res := range doc.Resources {
		for _, m := range res.Methods {
			have[m.ID] = true
		}
	}
	for _, id := range wantMethodIDs {
		if !have[id] {
			t.Errorf("discovery doc is missing method id %q", id)
		}
	}
}
