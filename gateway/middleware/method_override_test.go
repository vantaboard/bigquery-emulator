// Copyright 2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package middleware

import (
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
)

// Test-case name labels reused across multiple table-driven subtests.
// Hoisted to package-scope constants so goconst doesn't flag the
// per-table duplicates (the same verb labels recur across header-absent,
// non-POST-rejects, and unsupported-target tables).
const (
	caseNameGET     = "get"
	caseNameHEAD    = "head"
	caseNameOPTIONS = "options"
	caseNamePOST    = "post"
	caseNamePUT     = "put"
	caseNamePATCH   = "patch"
	caseNameDELETE  = "delete"
)

// TestWithMethodOverrideRewritesPOST exercises the documented
// success path: a POST carrying `X-HTTP-Method-Override: <verb>` where
// `<verb>` ∈ {PATCH, PUT, DELETE} (case-insensitive, padding-tolerant)
// is rewritten in place and forwarded to the downstream handler. The
// table covers every supported override target plus three casing
// variants and a header that carries surrounding whitespace, mirroring
// the slack the spec allows for HTTP header values.
func TestWithMethodOverrideRewritesPOST(t *testing.T) {
	cases := []struct {
		name       string
		header     string
		wantMethod string
	}{
		{caseNamePATCH + "-upper", "PATCH", http.MethodPatch},
		{caseNamePATCH + "-lower", "patch", http.MethodPatch},
		{caseNamePATCH + "-mixed", "Patch", http.MethodPatch},
		{caseNamePATCH + "-padded", "  PATCH  ", http.MethodPatch},
		{caseNamePUT + "-upper", "PUT", http.MethodPut},
		{caseNamePUT + "-mixed", "Put", http.MethodPut},
		{caseNameDELETE + "-upper", "DELETE", http.MethodDelete},
		{caseNameDELETE + "-mixed", "Delete", http.MethodDelete},
	}
	for _, c := range cases {
		t.Run(c.name, func(t *testing.T) {
			var observedMethod string
			next := http.HandlerFunc(func(_ http.ResponseWriter, r *http.Request) {
				observedMethod = r.Method
			})
			req := httptest.NewRequest(http.MethodPost,
				"/bigquery/v2/projects/p/datasets/d", nil)
			req.Header.Set(methodOverrideHeader, c.header)
			rec := httptest.NewRecorder()
			WithMethodOverride(next).ServeHTTP(rec, req)

			if rec.Code != http.StatusOK {
				t.Fatalf("status = %d, want 200; body=%s",
					rec.Code, rec.Body.String())
			}
			if observedMethod != c.wantMethod {
				t.Errorf("downstream method = %q, want %q",
					observedMethod, c.wantMethod)
			}
		})
	}
}

// TestWithMethodOverrideHeaderAbsentIsNoOp pins the
// no-header-no-effect contract: a request that doesn't carry the
// override header reaches the downstream handler unchanged. This is
// the dominant traffic shape (every gcloud/CLI request, every browser
// fetch, every loopback healthz probe) and we must not pay header-
// inspection cost beyond a single map lookup on it.
func TestWithMethodOverrideHeaderAbsentIsNoOp(t *testing.T) {
	cases := []struct {
		name   string
		method string
	}{
		{caseNameGET, http.MethodGet},
		{caseNameHEAD, http.MethodHead},
		{caseNameOPTIONS, http.MethodOptions},
		{caseNamePOST, http.MethodPost},
		{caseNamePUT, http.MethodPut},
		{caseNamePATCH, http.MethodPatch},
		{caseNameDELETE, http.MethodDelete},
	}
	for _, c := range cases {
		t.Run(c.name, func(t *testing.T) {
			var observed string
			next := http.HandlerFunc(func(_ http.ResponseWriter, r *http.Request) {
				observed = r.Method
			})
			req := httptest.NewRequest(c.method, "/x", nil)
			rec := httptest.NewRecorder()
			WithMethodOverride(next).ServeHTTP(rec, req)

			if rec.Code != http.StatusOK {
				t.Fatalf("status = %d, want 200; body=%s",
					rec.Code, rec.Body.String())
			}
			if observed != c.method {
				t.Errorf("downstream method = %q, want %q (no-op for header-absent)",
					observed, c.method)
			}
		})
	}
}

// TestWithMethodOverrideNonPOSTReturns400 pins the
// "override is meaningful only on POST" rule. A caller that sets the
// header on (e.g.) a GET would be attempting to silently mutate state
// through what looks like a safe verb in the access log; we refuse
// that with a structured 400 so the failure is obvious instead of
// silently bypassing dispatch.
func TestWithMethodOverrideNonPOSTReturns400(t *testing.T) {
	cases := []struct {
		name   string
		method string
	}{
		{caseNameGET, http.MethodGet},
		{caseNameHEAD, http.MethodHead},
		{caseNameOPTIONS, http.MethodOptions},
		{caseNamePUT, http.MethodPut},
		{caseNamePATCH, http.MethodPatch},
		{caseNameDELETE, http.MethodDelete},
	}
	for _, c := range cases {
		t.Run(c.name, func(t *testing.T) {
			called := false
			next := http.HandlerFunc(func(_ http.ResponseWriter, _ *http.Request) {
				called = true
			})
			req := httptest.NewRequest(c.method, "/x", nil)
			req.Header.Set(methodOverrideHeader, http.MethodPatch)
			rec := httptest.NewRecorder()
			WithMethodOverride(next).ServeHTTP(rec, req)

			if called {
				t.Errorf("downstream handler must not run when override is set on %s", c.method)
			}
			if rec.Code != http.StatusBadRequest {
				t.Fatalf("status = %d, want 400; body=%s",
					rec.Code, rec.Body.String())
			}
			if got := rec.Header().Get("Content-Type"); !strings.HasPrefix(got, "application/json") {
				t.Errorf("Content-Type = %q, want application/json prefix", got)
			}
			if !strings.Contains(rec.Body.String(), "only valid on POST") {
				t.Errorf("body = %q, want it to explain the POST-only rule",
					rec.Body.String())
			}
		})
	}
}

// TestWithMethodOverrideUnsupportedTargetReturns400 covers the
// unknown-verb branch: a POST with `X-HTTP-Method-Override: OPTIONS`
// (or `BANANA` or empty-after-trim) refuses to rewrite to a verb the
// downstream mux's Go 1.22 method-aware patterns aren't expecting,
// which would otherwise fall through to the `/` catch-all and produce
// a misleading 404.
func TestWithMethodOverrideUnsupportedTargetReturns400(t *testing.T) {
	cases := []struct {
		name   string
		header string
	}{
		{caseNameOPTIONS, http.MethodOptions},
		{caseNameHEAD, http.MethodHead},
		{caseNameGET, http.MethodGet},
		{caseNamePOST, http.MethodPost},
		{"banana", "BANANA"},
		{"empty-after-trim", "   "},
	}
	for _, c := range cases {
		t.Run(c.name, func(t *testing.T) {
			called := false
			next := http.HandlerFunc(func(_ http.ResponseWriter, _ *http.Request) {
				called = true
			})
			req := httptest.NewRequest(http.MethodPost, "/x", nil)
			req.Header.Set(methodOverrideHeader, c.header)
			rec := httptest.NewRecorder()
			WithMethodOverride(next).ServeHTTP(rec, req)

			if called {
				t.Errorf("downstream handler must not run for unsupported override %q", c.header)
			}
			if rec.Code != http.StatusBadRequest {
				t.Fatalf("status = %d, want 400; body=%s",
					rec.Code, rec.Body.String())
			}
			if !strings.Contains(rec.Body.String(), "must be one of PATCH") {
				t.Errorf("body = %q, want it to list the supported override verbs",
					rec.Body.String())
			}
		})
	}
}

// TestWithMethodOverrideEmptyHeaderIsNoOp pins the
// distinction between "header missing" and "header set to the empty
// string". `Header.Get` returns "" for both, but a client that
// explicitly sends `X-HTTP-Method-Override: ` (trailing space stripped
// by net/http) should be indistinguishable from one that sends nothing
// — we want a no-op pass-through, not a spurious 400. The casing
// dance in `TestWithMethodOverrideUnsupportedTargetReturns400` already
// covers the "trim-to-empty" case (`"   "`); this test pins the
// canonical bare-empty no-op so a future refactor that switches to
// `Header.Values` (which would surface the empty entry) is forced to
// keep the same behavior.
func TestWithMethodOverrideEmptyHeaderIsNoOp(t *testing.T) {
	var observed string
	next := http.HandlerFunc(func(_ http.ResponseWriter, r *http.Request) {
		observed = r.Method
	})
	req := httptest.NewRequest(http.MethodPost, "/x", nil)
	req.Header.Set(methodOverrideHeader, "")
	rec := httptest.NewRecorder()
	WithMethodOverride(next).ServeHTTP(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s",
			rec.Code, rec.Body.String())
	}
	if observed != http.MethodPost {
		t.Errorf("downstream method = %q, want POST", observed)
	}
}
