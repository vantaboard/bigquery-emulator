package gateway

import (
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
)

// TestSeedRoutes_GatedByFlag pins the documented opt-in: the
// `/api/emulator/seed*` routes are NOT registered when the operator
// did not pass --enable-seed-api. The default Server should return
// the catch-all 404 envelope on those paths.
func TestSeedRoutes_GatedByFlag(t *testing.T) {
	srv := NewServer(Options{}, nil)
	for _, path := range []string{
		"/api/emulator/seed",
		"/api/emulator/seed/operations/op-x",
	} {
		req := httptest.NewRequest(http.MethodPost, path, nil)
		rec := httptest.NewRecorder()
		srv.ServeHTTP(rec, req)
		if rec.Code != http.StatusNotFound {
			t.Errorf("%s with EnableSeedAPI=false -> %d, want 404", path, rec.Code)
		}
		if !strings.Contains(rec.Body.String(), "No route matches") {
			t.Errorf("%s body=%q; expected route-not-found envelope (proves the seed route was not registered)",
				path, rec.Body.String())
		}
	}
}

// TestSeedRoutes_RegisteredWhenEnabled checks the inverse: with
// --enable-seed-api the POST is reachable. Without a runner (nil
// engine.Client) the handler degrades to 501 per the documented
// contract; that is what we assert here.
func TestSeedRoutes_RegisteredWhenEnabled(t *testing.T) {
	srv := NewServer(Options{EnableSeedAPI: true}, nil)
	req := httptest.NewRequest(http.MethodPost, "/api/emulator/seed",
		strings.NewReader(`{"source":{"project":"p"}}`))
	req.RemoteAddr = "127.0.0.1:1234"
	rec := httptest.NewRecorder()
	srv.ServeHTTP(rec, req)
	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("POST /api/emulator/seed (enabled, no engine) -> %d, want 501; body=%s",
			rec.Code, rec.Body.String())
	}
	if !strings.Contains(rec.Body.String(), "--seed-data-file") {
		t.Errorf("501 body=%q; should point operators at the YAML loader fallback",
			rec.Body.String())
	}
}

// TestSeedRoutes_RemoteCallerDeniedWithoutAllow pins the AccessConfig
// integration at the server level: a non-loopback POST hits 403
// before the handler tries to construct an operation.
func TestSeedRoutes_RemoteCallerDeniedWithoutAllow(t *testing.T) {
	srv := NewServer(Options{EnableSeedAPI: true}, nil)
	req := httptest.NewRequest(http.MethodPost, "/api/emulator/seed",
		strings.NewReader(`{"source":{"project":"p"}}`))
	req.RemoteAddr = "10.0.0.5:1234"
	rec := httptest.NewRecorder()
	srv.ServeHTTP(rec, req)
	if rec.Code != http.StatusForbidden {
		t.Errorf("status=%d, want 403", rec.Code)
	}
}

// TestSeedRoutes_AllowRemoteFlag verifies the opt-in CI knob: with
// SeedAPIAllowRemote=true the same LAN caller goes past the
// loopback gate (and lands on 501 because there's no runner).
func TestSeedRoutes_AllowRemoteFlag(t *testing.T) {
	srv := NewServer(Options{EnableSeedAPI: true, SeedAPIAllowRemote: true}, nil)
	req := httptest.NewRequest(http.MethodPost, "/api/emulator/seed",
		strings.NewReader(`{"source":{"project":"p"}}`))
	req.RemoteAddr = "10.0.0.5:1234"
	rec := httptest.NewRecorder()
	srv.ServeHTTP(rec, req)
	if rec.Code != http.StatusNotImplemented {
		t.Errorf("status=%d, want 501 (route reachable, runner missing)", rec.Code)
	}
}
