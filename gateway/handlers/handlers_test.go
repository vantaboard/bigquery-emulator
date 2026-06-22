package handlers

import (
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"testing"
)

func TestHealth(t *testing.T) {
	rec := httptest.NewRecorder()
	req := httptest.NewRequest(http.MethodGet, "/", nil)
	Health(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want %d", rec.Code, http.StatusOK)
	}
	var got map[string]string
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got["status"] != "ok" {
		t.Fatalf("status field = %q, want %q", got["status"], "ok")
	}
	if got["service"] != "bigquery-emulator" {
		t.Fatalf("service field = %q, want %q", got["service"], "bigquery-emulator")
	}
}

func TestNotImplementedShape(t *testing.T) {
	rec := httptest.NewRecorder()
	req := httptest.NewRequest(http.MethodGet, "/anything", nil)
	NotImplemented(rec, req)

	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want %d", rec.Code, http.StatusNotImplemented)
	}
	var env errorEnvelope
	if err := json.NewDecoder(rec.Body).Decode(&env); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if env.Error.Code != http.StatusNotImplemented {
		t.Fatalf("error.code = %d, want %d", env.Error.Code, http.StatusNotImplemented)
	}
	if len(env.Error.Errors) == 0 {
		t.Fatal("error.errors is empty; BigQuery clients expect at least one detail")
	}
	if env.Error.Errors[0].Reason == "" {
		t.Fatal("error.errors[0].reason is empty")
	}
}

// TestBQStyleMessageRewritesStorageErrors pins the verbatim shape the
// node samples assert on (`expect(err.message).to.include('Not found')`,
// `expect(err.message).to.include('Already Exists')`). Anything that
// does not match a known storage-noun pattern passes through verbatim
// so analysis-time / dml-time errors keep their engine wording.
func TestBQStyleMessageRewritesStorageErrors(t *testing.T) {
	tests := []struct {
		name, in, want string
	}{
		{
			name: "table not found",
			in:   "table not found: dev.foo.bar",
			want: "Not found: Table dev:foo.bar",
		},
		{
			name: "dataset not found",
			in:   "dataset not found: dev.foo",
			want: "Not found: Dataset dev:foo",
		},
		{
			name: "table already exists",
			in:   "table already exists: dev.foo.bar",
			want: "Already Exists: Table dev:foo.bar",
		},
		{
			name: "dataset already exists",
			in:   "dataset already exists: dev.foo",
			want: "Already Exists: Dataset dev:foo",
		},
		{
			name: "passthrough preserves analysis error",
			in:   "Unrecognized name: doesnotexist [at 1:8]",
			want: "Unrecognized name: doesnotexist [at 1:8]",
		},
		{
			name: "passthrough preserves embedded substring",
			in:   "Engine internal: contains table not found: dev.x.y substring",
			want: "Engine internal: contains table not found: dev.x.y substring",
		},
		{
			name: "invalid timestamp parameter",
			in:   "semantic: invalid TIMESTAMP parameter value 'not-a-timestamp'",
			want: "Invalid timestamp string",
		},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			got := bqStyleMessage(tc.in)
			if got != tc.want {
				t.Errorf("bqStyleMessage(%q) = %q, want %q", tc.in, got, tc.want)
			}
		})
	}
}

func TestRequestEmulatorBaseURL(t *testing.T) {
	t.Setenv("BIGQUERY_EMULATOR_HOST", "0.0.0.0:9050")
	if got := requestEmulatorBaseURL(nil); got != "http://0.0.0.0:9050" {
		t.Fatalf("env host = %q, want http://0.0.0.0:9050", got)
	}
	t.Setenv("BIGQUERY_EMULATOR_HOST", "")
	req := httptest.NewRequest(http.MethodPost, "/upload/", nil)
	req.Host = "localhost:9050"
	if got := requestEmulatorBaseURL(req); got != "http://localhost:9050" {
		t.Fatalf("request host = %q, want http://localhost:9050", got)
	}
}
