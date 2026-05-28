package gateway

import (
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
)

// Shared synthetic REST paths used by the route-table smoke test.
const (
	datasetsPath = "/projects/p/datasets"
	datasetPath  = "/projects/p/datasets/d"
	tablePath    = "/projects/p/datasets/d/tables/t"
)

// routeCase captures one row of the BigQuery v2 / non-v2 route tables
// the smoke test below walks. Hoisted out of the test body so the
// table literals do not count toward TestRouteTable's funlen budget.
type routeCase struct {
	name   string
	method string
	path   string
}

// bqV2RouteCases lists the bare paths (no `/bigquery/v2` prefix) for
// every documented BigQuery v2 REST endpoint. The test exercises each
// case under both the prefixed and the bare form (see
// `mountBigQueryV2` in server.go for why both exist).
var bqV2RouteCases = []routeCase{
	{"projects.list", http.MethodGet, "/projects"},
	{"projects.getServiceAccount", http.MethodGet, "/projects/p/serviceAccount"},

	{"datasets.list", http.MethodGet, datasetsPath},
	{"datasets.insert", http.MethodPost, datasetsPath},
	{"datasets.get", http.MethodGet, datasetPath},
	{"datasets.update", http.MethodPut, datasetPath},
	{"datasets.patch", http.MethodPatch, datasetPath},
	{"datasets.delete", http.MethodDelete, datasetPath},
	{"datasets.undelete", http.MethodPost, datasetPath + ":undelete"},

	{"tables.list", http.MethodGet, datasetPath + "/tables"},
	{"tables.insert", http.MethodPost, datasetPath + "/tables"},
	{"tables.get", http.MethodGet, tablePath},
	{"tables.update", http.MethodPut, tablePath},
	{"tables.patch", http.MethodPatch, tablePath},
	{"tables.delete", http.MethodDelete, tablePath},
	{"tables.getIamPolicy", http.MethodPost, tablePath + ":getIamPolicy"},
	{"tables.setIamPolicy", http.MethodPost, tablePath + ":setIamPolicy"},
	{"tables.testIamPermissions", http.MethodPost, tablePath + ":testIamPermissions"},

	{"tabledata.list", http.MethodGet, tablePath + "/data"},
	{"tabledata.insertAll", http.MethodPost, tablePath + "/insertAll"},

	// Jobs (excludes the media-upload variant, which lives at
	// /upload/bigquery/v2/... only - see otherRouteCases below).
	{"jobs.list", http.MethodGet, "/projects/p/jobs"},
	{"jobs.insert", http.MethodPost, "/projects/p/jobs"},
	{"jobs.get", http.MethodGet, "/projects/p/jobs/j"},
	{"jobs.cancel", http.MethodPost, "/projects/p/jobs/j/cancel"},
	{"jobs.delete", http.MethodDelete, "/projects/p/jobs/j/delete"},

	{"jobs.query", http.MethodPost, "/projects/p/queries"},
	{"jobs.getQueryResults", http.MethodGet, "/projects/p/queries/j"},
}

// otherRouteCases lists endpoints registered at exactly one literal
// path (not mirrored at the bare form).
var otherRouteCases = []routeCase{
	{"health-root", http.MethodGet, "/"},
	{"health-z", http.MethodGet, "/healthz"},

	{"discovery", http.MethodGet, "/discovery/v1/apis/bigquery/v2/rest"},

	// jobs.insert media-upload variant - the public API hard-codes
	// the /upload prefix, so it is not mirrored at /.../jobs.
	{"jobs.insert-upload", http.MethodPost, "/upload/bigquery/v2/projects/p/jobs"},
}

// assertRouteReachable issues `method path` against srv and fails the
// test if the response is the catch-all 404 emitted by
// `handlers.NotFound` (distinguishable by its "No route matches"
// message). A handler 404 is allowed because some routes legitimately
// return 404 for not-yet-existing resources (e.g.
// `jobs.getQueryResults` for an unknown jobId).
func assertRouteReachable(t *testing.T, srv http.Handler, method, path string) {
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

// TestRouteTable smoke-tests that every documented BigQuery v2 REST
// endpoint reaches a handler (not the 404 catch-all). Cross-reference
// docs/REST_API.md when adding new routes to bqV2RouteCases or
// otherRouteCases.
func TestRouteTable(t *testing.T) {
	srv := NewServer(Options{}, nil)

	for _, tc := range bqV2RouteCases {
		// `/bigquery/v2` prefix: the form gcloud, bq, and clients
		// pointed at *.googleapis.com use.
		t.Run("prefixed/"+tc.name, func(t *testing.T) {
			assertRouteReachable(t, srv, tc.method, "/bigquery/v2"+tc.path)
		})
		// Bare form: required because the official client libraries
		// (e.g. @google-cloud/bigquery v8) treat BIGQUERY_EMULATOR_HOST
		// as the verbatim baseUrl with no version segment.
		t.Run("bare/"+tc.name, func(t *testing.T) {
			assertRouteReachable(t, srv, tc.method, tc.path)
		})
	}

	for _, tc := range otherRouteCases {
		t.Run(tc.name, func(t *testing.T) {
			assertRouteReachable(t, srv, tc.method, tc.path)
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
