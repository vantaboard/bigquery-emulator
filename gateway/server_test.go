package gateway

import (
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
)

// TestRouteTable smoke-tests that every documented BigQuery v2 REST
// endpoint reaches a handler (not the 404 catch-all). Originally
// every handler returned 501 so the test simply asserted "code != 404",
// but as real handlers land some now legitimately return 404 for
// not-yet-existing resources (e.g. `jobs.getQueryResults` for an
// unknown jobId). To stay precise about what we are testing, the
// route-missing 404 emitted by `handlers.NotFound` is distinguished
// from a handler 404 by its `"No route matches ..."` message.
//
// Cross-reference docs/REST_API.md when adding new routes here.
func TestRouteTable(t *testing.T) {
	srv := NewServer(Options{}, nil)

	// BigQuery v2 endpoints are mounted under both the `/bigquery/v2`
	// prefix and the bare path (see `mountBQv2` in server.go). The
	// table records the bare path; the test exercises both forms.
	bqV2 := []struct {
		name   string
		method string
		path   string
	}{
		// Projects
		{"projects.list", "GET", "/projects"},
		{"projects.getServiceAccount", "GET", "/projects/p/serviceAccount"},

		// Datasets
		{"datasets.list", "GET", "/projects/p/datasets"},
		{"datasets.insert", "POST", "/projects/p/datasets"},
		{"datasets.get", "GET", "/projects/p/datasets/d"},
		{"datasets.update", "PUT", "/projects/p/datasets/d"},
		{"datasets.patch", "PATCH", "/projects/p/datasets/d"},
		{"datasets.delete", "DELETE", "/projects/p/datasets/d"},
		{"datasets.undelete", "POST", "/projects/p/datasets/d:undelete"},

		// Tables
		{"tables.list", "GET", "/projects/p/datasets/d/tables"},
		{"tables.insert", "POST", "/projects/p/datasets/d/tables"},
		{"tables.get", "GET", "/projects/p/datasets/d/tables/t"},
		{"tables.update", "PUT", "/projects/p/datasets/d/tables/t"},
		{"tables.patch", "PATCH", "/projects/p/datasets/d/tables/t"},
		{"tables.delete", "DELETE", "/projects/p/datasets/d/tables/t"},
		{"tables.getIamPolicy", "POST", "/projects/p/datasets/d/tables/t:getIamPolicy"},
		{"tables.setIamPolicy", "POST", "/projects/p/datasets/d/tables/t:setIamPolicy"},
		{"tables.testIamPermissions", "POST", "/projects/p/datasets/d/tables/t:testIamPermissions"},

		// Tabledata
		{"tabledata.list", "GET", "/projects/p/datasets/d/tables/t/data"},
		{"tabledata.insertAll", "POST", "/projects/p/datasets/d/tables/t/insertAll"},

		// Jobs (excludes the media-upload variant, which lives at
		// /upload/bigquery/v2/... only — see `other` below).
		{"jobs.list", "GET", "/projects/p/jobs"},
		{"jobs.insert", "POST", "/projects/p/jobs"},
		{"jobs.get", "GET", "/projects/p/jobs/j"},
		{"jobs.cancel", "POST", "/projects/p/jobs/j/cancel"},
		{"jobs.delete", "DELETE", "/projects/p/jobs/j/delete"},

		// Queries
		{"jobs.query", "POST", "/projects/p/queries"},
		{"jobs.getQueryResults", "GET", "/projects/p/queries/j"},
	}

	// Non-BigQuery-v2 endpoints (or fixed-prefix variants of v2) are
	// registered exactly once, at the literal path documented here.
	other := []struct {
		name   string
		method string
		path   string
	}{
		// Health
		{"health-root", "GET", "/"},
		{"health-z", "GET", "/healthz"},

		// Discovery
		{"discovery", "GET", "/discovery/v1/apis/bigquery/v2/rest"},

		// jobs.insert media-upload variant — the public API hard-codes
		// the /upload prefix, so it is not mirrored at /.../jobs.
		{"jobs.insert-upload", "POST", "/upload/bigquery/v2/projects/p/jobs"},
	}

	check := func(t *testing.T, method, path string) {
		t.Helper()
		req := httptest.NewRequest(method, path, nil)
		rec := httptest.NewRecorder()
		srv.ServeHTTP(rec, req)
		if rec.Code == http.StatusNotFound &&
			strings.Contains(rec.Body.String(), "No route matches") {
			t.Fatalf("%s %s returned 404 from the route catch-all; "+
				"route is missing", method, path)
		}
	}

	for _, tc := range bqV2 {
		// `/bigquery/v2` prefix: the form gcloud, bq, and clients
		// pointed at *.googleapis.com use.
		t.Run("prefixed/"+tc.name, func(t *testing.T) {
			check(t, tc.method, "/bigquery/v2"+tc.path)
		})
		// Bare form: required because the official client libraries
		// (e.g. @google-cloud/bigquery v8) treat BIGQUERY_EMULATOR_HOST
		// as the verbatim baseUrl with no version segment.
		t.Run("bare/"+tc.name, func(t *testing.T) {
			check(t, tc.method, tc.path)
		})
	}

	for _, tc := range other {
		t.Run(tc.name, func(t *testing.T) {
			check(t, tc.method, tc.path)
		})
	}
}

// TestUnknownColonOpReturns404 verifies the dispatcher returns a
// BigQuery-shaped 404 (not a 501) when a client invokes an unknown
// custom method on a dataset or table resource. Both the `/bigquery/v2`
// prefix and the bare form are covered because clients pointed at
// BIGQUERY_EMULATOR_HOST hit the bare form.
func TestUnknownColonOpReturns404(t *testing.T) {
	srv := NewServer(Options{}, nil)
	bareCases := []string{
		"/projects/p/datasets/d:nosuchop",
		"/projects/p/datasets/d/tables/t:nosuchop",
	}
	for _, path := range bareCases {
		for _, full := range []string{"/bigquery/v2" + path, path} {
			req := httptest.NewRequest(http.MethodPost, full, nil)
			rec := httptest.NewRecorder()
			srv.ServeHTTP(rec, req)
			if rec.Code != http.StatusNotFound {
				t.Fatalf("POST %s -> %d, want 404", full, rec.Code)
			}
		}
	}
}

// TestRemovedProjectGetIs404 guards against re-introducing the bogus
// `GET /bigquery/v2/projects/{projectId}` route that an early scaffold
// registered. There is no such endpoint in the public BigQuery API,
// in either the prefixed or the bare form.
func TestRemovedProjectGetIs404(t *testing.T) {
	srv := NewServer(Options{}, nil)
	for _, path := range []string{"/bigquery/v2/projects/p", "/projects/p"} {
		req := httptest.NewRequest(http.MethodGet, path, nil)
		rec := httptest.NewRecorder()
		srv.ServeHTTP(rec, req)
		if rec.Code != http.StatusNotFound {
			t.Fatalf("GET %s -> %d, want 404 (endpoint does not exist)", path, rec.Code)
		}
	}
}

// TestBearerTokenIsNotRejected pins the documented auth posture: the
// emulator parses Authorization headers but never rejects them. A real
// BigQuery client always sends a bearer token, so a 401 here would
// force every client to special-case the emulator.
func TestBearerTokenIsNotRejected(t *testing.T) {
	srv := NewServer(Options{}, nil)

	cases := []struct {
		name  string
		token string
	}{
		{"valid-looking-bearer", "Bearer ya29.real-looking-token"},
		{"lowercase-scheme", "bearer ya29.lowercase"},
		{"empty-token", "Bearer "},
		{"malformed-no-scheme", "definitely-not-a-token"},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			req := httptest.NewRequest(http.MethodGet, "/healthz", nil)
			req.Header.Set("Authorization", tc.token)
			rec := httptest.NewRecorder()
			srv.ServeHTTP(rec, req)
			if rec.Code == http.StatusUnauthorized {
				t.Fatalf("Authorization=%q -> 401; emulator must never reject bearer tokens", tc.token)
			}
			if rec.Code != http.StatusOK {
				t.Fatalf("Authorization=%q -> %d, want 200", tc.token, rec.Code)
			}
		})
	}
}

// TestDiscoveryReturnsOK verifies the discovery route now returns a
// real document (kind=discovery#restDescription) rather than the 501
// stub it used to. This is the route library clients hit at startup.
func TestDiscoveryReturnsOK(t *testing.T) {
	srv := NewServer(Options{}, nil)
	req := httptest.NewRequest(http.MethodGet, "/discovery/v1/apis/bigquery/v2/rest", nil)
	rec := httptest.NewRecorder()
	srv.ServeHTTP(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200", rec.Code)
	}
}
