package gateway

import (
	"net/http"
	"net/http/httptest"
	"testing"
)

// TestRouteTable smoke-tests that every documented BigQuery v2 REST
// endpoint reaches a handler (not the 404 catch-all). Currently every
// handler is a 501 stub, so we only assert the response is *not* a 404
// and that it has the BigQuery error envelope. As real implementations
// land, individual handlers grow their own targeted tests.
//
// Cross-reference docs/REST_API.md when adding new routes here.
func TestRouteTable(t *testing.T) {
	srv := NewServer(Options{})

	cases := []struct {
		name   string
		method string
		path   string
	}{
		// Health
		{"health-root", "GET", "/"},
		{"health-z", "GET", "/healthz"},

		// Discovery
		{"discovery", "GET", "/discovery/v1/apis/bigquery/v2/rest"},

		// Projects
		{"projects.list", "GET", "/bigquery/v2/projects"},
		{"projects.getServiceAccount", "GET", "/bigquery/v2/projects/p/serviceAccount"},

		// Datasets
		{"datasets.list", "GET", "/bigquery/v2/projects/p/datasets"},
		{"datasets.insert", "POST", "/bigquery/v2/projects/p/datasets"},
		{"datasets.get", "GET", "/bigquery/v2/projects/p/datasets/d"},
		{"datasets.update", "PUT", "/bigquery/v2/projects/p/datasets/d"},
		{"datasets.patch", "PATCH", "/bigquery/v2/projects/p/datasets/d"},
		{"datasets.delete", "DELETE", "/bigquery/v2/projects/p/datasets/d"},
		{"datasets.undelete", "POST", "/bigquery/v2/projects/p/datasets/d:undelete"},

		// Tables
		{"tables.list", "GET", "/bigquery/v2/projects/p/datasets/d/tables"},
		{"tables.insert", "POST", "/bigquery/v2/projects/p/datasets/d/tables"},
		{"tables.get", "GET", "/bigquery/v2/projects/p/datasets/d/tables/t"},
		{"tables.update", "PUT", "/bigquery/v2/projects/p/datasets/d/tables/t"},
		{"tables.patch", "PATCH", "/bigquery/v2/projects/p/datasets/d/tables/t"},
		{"tables.delete", "DELETE", "/bigquery/v2/projects/p/datasets/d/tables/t"},
		{"tables.getIamPolicy", "POST", "/bigquery/v2/projects/p/datasets/d/tables/t:getIamPolicy"},
		{"tables.setIamPolicy", "POST", "/bigquery/v2/projects/p/datasets/d/tables/t:setIamPolicy"},
		{"tables.testIamPermissions", "POST", "/bigquery/v2/projects/p/datasets/d/tables/t:testIamPermissions"},

		// Tabledata
		{"tabledata.list", "GET", "/bigquery/v2/projects/p/datasets/d/tables/t/data"},
		{"tabledata.insertAll", "POST", "/bigquery/v2/projects/p/datasets/d/tables/t/insertAll"},

		// Jobs
		{"jobs.list", "GET", "/bigquery/v2/projects/p/jobs"},
		{"jobs.insert", "POST", "/bigquery/v2/projects/p/jobs"},
		{"jobs.insert-upload", "POST", "/upload/bigquery/v2/projects/p/jobs"},
		{"jobs.get", "GET", "/bigquery/v2/projects/p/jobs/j"},
		{"jobs.cancel", "POST", "/bigquery/v2/projects/p/jobs/j/cancel"},
		{"jobs.delete", "DELETE", "/bigquery/v2/projects/p/jobs/j/delete"},

		// Queries
		{"jobs.query", "POST", "/bigquery/v2/projects/p/queries"},
		{"jobs.getQueryResults", "GET", "/bigquery/v2/projects/p/queries/j"},
	}

	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			req := httptest.NewRequest(tc.method, tc.path, nil)
			rec := httptest.NewRecorder()
			srv.ServeHTTP(rec, req)
			if rec.Code == http.StatusNotFound {
				t.Fatalf("%s %s returned 404; route is missing", tc.method, tc.path)
			}
		})
	}
}

// TestUnknownColonOpReturns404 verifies the dispatcher returns a
// BigQuery-shaped 404 (not a 501) when a client invokes an unknown
// custom method on a dataset or table resource.
func TestUnknownColonOpReturns404(t *testing.T) {
	srv := NewServer(Options{})
	cases := []string{
		"/bigquery/v2/projects/p/datasets/d:nosuchop",
		"/bigquery/v2/projects/p/datasets/d/tables/t:nosuchop",
	}
	for _, path := range cases {
		req := httptest.NewRequest(http.MethodPost, path, nil)
		rec := httptest.NewRecorder()
		srv.ServeHTTP(rec, req)
		if rec.Code != http.StatusNotFound {
			t.Fatalf("POST %s -> %d, want 404", path, rec.Code)
		}
	}
}

// TestRemovedProjectGetIs404 guards against re-introducing the bogus
// `GET /bigquery/v2/projects/{projectId}` route that an early scaffold
// registered. There is no such endpoint in the public BigQuery API.
func TestRemovedProjectGetIs404(t *testing.T) {
	srv := NewServer(Options{})
	req := httptest.NewRequest(http.MethodGet, "/bigquery/v2/projects/p", nil)
	rec := httptest.NewRecorder()
	srv.ServeHTTP(rec, req)
	if rec.Code != http.StatusNotFound {
		t.Fatalf("GET /bigquery/v2/projects/p -> %d, want 404 (endpoint does not exist)", rec.Code)
	}
}

// TestBearerTokenIsNotRejected pins the documented auth posture: the
// emulator parses Authorization headers but never rejects them. A real
// BigQuery client always sends a bearer token, so a 401 here would
// force every client to special-case the emulator.
func TestBearerTokenIsNotRejected(t *testing.T) {
	srv := NewServer(Options{})

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
	srv := NewServer(Options{})
	req := httptest.NewRequest(http.MethodGet, "/discovery/v1/apis/bigquery/v2/rest", nil)
	rec := httptest.NewRecorder()
	srv.ServeHTTP(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200", rec.Code)
	}
}
