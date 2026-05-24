//go:build integration

package e2e

import (
	"encoding/json"
	"net/http"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// skipIfDryRunUnimplemented probes the gateway with a trivial dry-run
// request and skips the test when the engine subprocess responds with
// HTTP 501 notImplemented. The CMake build of `emulator_main` does
// not link GoogleSQL (`BIGQUERY_EMULATOR_HAS_GOOGLESQL` unset), so
// `Query.DryRun` returns `UNIMPLEMENTED` from the C++ handler and the
// whole REST surface for dry-run queries falls back to 501. The
// canonical Bazel build flips the define on; this probe lets us
// auto-promote the test from skip → real check once the Bazel
// `emulator_main` is wired.
func skipIfDryRunUnimplemented(t *testing.T, env *emulatorEnv) {
	t.Helper()
	probeBody := `{"query":"SELECT 1","dryRun":true,"useLegacySql":false}`
	status, body := doJSON(t, http.MethodPost,
		env.URL()+"/bigquery/v2/projects/probe/queries", []byte(probeBody))
	if status == http.StatusNotImplemented {
		t.Skipf("emulator_main was built without GoogleSQL "+
			"(Query.DryRun -> 501 notImplemented); rebuild with "+
			"`bazel build //binaries/emulator_main` to exercise this "+
			"E2E path. Probe body: %s", string(body))
	}
}

// TestDryRunReturnsAnalyzedSchema is the Phase 4c end-to-end story:
// register a dataset and a typed table over REST, run
// `jobs.query?dryRun=true` against `SELECT * FROM ds.t`, and confirm
// the analyzed schema returned by `googlesql::Analyzer` matches the
// table schema we created. Exercises the full
// REST -> gRPC -> Query.DryRun -> GoogleSQL path.
func TestDryRunReturnsAnalyzedSchema(t *testing.T) {
	env := startEmulator(t)
	skipIfDryRunUnimplemented(t, env)

	const (
		projectID = "proj-dryrun"
		datasetID = "ds_dryrun"
		tableID   = "people"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	tableBody := `{
        "tableReference":{"projectId":"` + projectID +
		`","datasetId":"` + datasetID +
		`","tableId":"` + tableID + `"},
        "schema":{"fields":[
            {"name":"id","type":"INT64","mode":"REQUIRED"},
            {"name":"name","type":"STRING","mode":"NULLABLE"},
            {"name":"tags","type":"STRING","mode":"REPEATED"}
        ]}
    }`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables", []byte(tableBody))
	if status != http.StatusOK {
		t.Fatalf("tables.insert -> %d: %s", status, string(body))
	}

	queryBody := `{"query":"SELECT id, name, tags FROM ` + datasetID +
		`.` + tableID + `","dryRun":true,"useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost,
		base+"/queries", []byte(queryBody))
	if status != http.StatusOK {
		t.Fatalf("jobs.query?dryRun=true -> %d: %s", status, string(body))
	}

	var resp bqtypes.QueryResponse
	if err := json.Unmarshal(body, &resp); err != nil {
		t.Fatalf("decode QueryResponse: %v (body=%s)", err, string(body))
	}
	if resp.Kind != "bigquery#queryResponse" {
		t.Errorf("kind = %q, want %q", resp.Kind, "bigquery#queryResponse")
	}
	if !resp.JobComplete {
		t.Error("jobComplete = false, want true for a completed dry run")
	}
	if resp.TotalBytesProcessed == "" {
		t.Error("totalBytesProcessed empty; want a decimal string (zero is fine)")
	}
	if len(resp.Rows) != 0 {
		t.Errorf("rows = %d, want 0 for a dry run", len(resp.Rows))
	}
	if resp.Schema == nil || len(resp.Schema.Fields) != 3 {
		t.Fatalf("schema mismatch (want 3 fields): %+v", resp.Schema)
	}

	wantFields := []bqtypes.TableFieldSchema{
		{Name: "id", Type: "INT64"},
		{Name: "name", Type: "STRING"},
		{Name: "tags", Type: "STRING"},
	}
	for i, want := range wantFields {
		got := resp.Schema.Fields[i]
		if got.Name != want.Name {
			t.Errorf("schema.fields[%d].name = %q, want %q",
				i, got.Name, want.Name)
		}
		if got.Type != want.Type {
			t.Errorf("schema.fields[%d].type = %q, want %q",
				i, got.Type, want.Type)
		}
	}
	// `tags` is REPEATED; mode must round-trip from the analyzer so
	// clients can recognize the array shape without re-running the
	// query for real.
	if got := resp.Schema.Fields[2].Mode; got != "REPEATED" {
		t.Errorf("schema.fields[2].mode = %q, want %q", got, "REPEATED")
	}
}

// TestDryRunSyntaxErrorReturnsInvalidQuery exercises the error path:
// a syntactically-invalid query must surface as HTTP 400 with
// `reason: invalidQuery` so client libraries see the documented
// envelope. Pins the gRPC INVALID_ARGUMENT -> HTTP 400 mapping in
// gateway/handlers/errors.go.
func TestDryRunSyntaxErrorReturnsInvalidQuery(t *testing.T) {
	env := startEmulator(t)
	skipIfDryRunUnimplemented(t, env)

	base := env.URL() + "/bigquery/v2/projects/proj-dryrun-bad/queries"
	status, body := doJSON(t, http.MethodPost, base,
		[]byte(`{"query":"SELECT FROM","dryRun":true,"useLegacySql":false}`))
	if status != http.StatusBadRequest {
		t.Fatalf("jobs.query?dryRun=true -> %d, want 400; body=%s",
			status, string(body))
	}

	var doc map[string]any
	if err := json.Unmarshal(body, &doc); err != nil {
		t.Fatalf("decode error envelope: %v (body=%s)", err, string(body))
	}
	errObj, ok := doc["error"].(map[string]any)
	if !ok {
		t.Fatalf("error envelope missing `error` object: %s", string(body))
	}
	if reason, _ := errObj["status"].(string); reason != "invalidQuery" {
		t.Errorf("error.status = %v, want %q", errObj["status"], "invalidQuery")
	}
	msg, _ := errObj["message"].(string)
	if msg == "" || !strings.Contains(strings.ToLower(msg), "syntax") {
		t.Errorf("error.message = %q, want to mention `syntax` from the analyzer", msg)
	}
}

// TestDryRunUnknownTableReturnsInvalidQuery pins the analyzer's
// name-resolution failure path. GoogleSQL surfaces "Table not found"
// as INVALID_ARGUMENT (not NOT_FOUND), which the gateway maps to
// HTTP 400 + invalidQuery.
func TestDryRunUnknownTableReturnsInvalidQuery(t *testing.T) {
	env := startEmulator(t)
	skipIfDryRunUnimplemented(t, env)

	const (
		projectID = "proj-dryrun-missing"
		datasetID = "ds_dryrun_missing"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	queryBody := `{"query":"SELECT * FROM ` + datasetID +
		`.missing","dryRun":true,"useLegacySql":false}`
	status, body = doJSON(t, http.MethodPost,
		base+"/queries", []byte(queryBody))
	if status != http.StatusBadRequest {
		t.Fatalf("jobs.query?dryRun=true -> %d, want 400; body=%s",
			status, string(body))
	}

	var doc map[string]any
	if err := json.Unmarshal(body, &doc); err != nil {
		t.Fatalf("decode error envelope: %v (body=%s)", err, string(body))
	}
	errObj, _ := doc["error"].(map[string]any)
	if reason, _ := errObj["status"].(string); reason != "invalidQuery" {
		t.Errorf("error.status = %v, want %q", errObj["status"], "invalidQuery")
	}
	if msg, _ := errObj["message"].(string); !strings.Contains(msg, "missing") {
		t.Errorf("error.message = %q, want to mention the missing table name", msg)
	}
}
