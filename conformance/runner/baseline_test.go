package runner

import (
	"encoding/json"
	"strings"
	"testing"
)

func TestMarshalJobsQueryBodyDefaultDataset(t *testing.T) {
	body, err := marshalJobsQueryBody("SELECT 1", "ds_default")
	if err != nil {
		t.Fatalf("marshalJobsQueryBody: %v", err)
	}
	var parsed map[string]any
	if err := json.Unmarshal(body, &parsed); err != nil {
		t.Fatalf("unmarshal: %v", err)
	}
	dd, ok := parsed["defaultDataset"].(map[string]any)
	if !ok {
		t.Fatalf("defaultDataset missing: %v", parsed)
	}
	if dd["datasetId"] != "ds_default" {
		t.Fatalf("datasetId=%v, want ds_default", dd["datasetId"])
	}
}

func TestMarshalJobsQueryBodyOmitsDefaultDatasetWhenEmpty(t *testing.T) {
	body, err := marshalJobsQueryBody("SELECT 1", "")
	if err != nil {
		t.Fatalf("marshalJobsQueryBody: %v", err)
	}
	if strings.Contains(string(body), "defaultDataset") {
		t.Fatalf("unexpected defaultDataset in %s", body)
	}
}

func TestLoadDefaultDatasetField(t *testing.T) {
	body := []byte(`name: test
default_dataset: ds_main
query: SELECT 1 FROM t
expected:
  rows:
    - {n: "1"}
`)
	f, err := loadBytes(body, "test.yaml")
	if err != nil {
		t.Fatalf("loadBytes: %v", err)
	}
	if f.DefaultDataset != "ds_main" {
		t.Fatalf("DefaultDataset=%q, want ds_main", f.DefaultDataset)
	}
}

func TestBaselineUpdateForbiddenCoreUsagePath(t *testing.T) {
	fx := &Fixture{Path: "conformance/fixtures/core_usage/views/x.yaml"}
	if !fx.BaselineUpdateForbidden() {
		t.Fatal("core_usage path should be protected")
	}
}

func TestBaselineUpdateForbiddenVerifiedProduction(t *testing.T) {
	fx := &Fixture{Path: "conformance/fixtures/fastpath/x.yaml", VerifiedProduction: true}
	if !fx.BaselineUpdateForbidden() {
		t.Fatal("verified_production should be protected")
	}
}

func TestRewriteFixtureRowsRefusesProtected(t *testing.T) {
	fx := &Fixture{
		Path:               "conformance/fixtures/core_usage/views/x.yaml",
		VerifiedProduction: true,
		Name:               "x",
		Query:              "SELECT 1 AS n",
	}
	err := rewriteFixtureRows(
		fx,
		[]byte(`{"schema":{"fields":[{"name":"n","type":"INT64"}]},"rows":[{"f":[{"v":"1"}]}]}`),
	)
	if err == nil || !strings.Contains(err.Error(), "refused") {
		t.Fatalf("want refused error, got %v", err)
	}
}
