package gateway

import (
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
)

func TestSQLToolsRoutes_GatedByFlag(t *testing.T) {
	srv := NewServer(Options{}, handlers.BuildDependencies(nil), nil)
	for _, path := range []string{
		"/api/emulator/sql/capabilities",
		"/api/emulator/sql/format",
		"/api/emulator/sql/analyze",
	} {
		method := http.MethodPost
		if path == "/api/emulator/sql/capabilities" {
			method = http.MethodGet
		}
		req := httptest.NewRequest(method, path, nil)
		rec := httptest.NewRecorder()
		srv.ServeHTTP(rec, req)
		if rec.Code != http.StatusNotFound {
			t.Errorf("%s with EnableSQLToolsAPI=false -> %d, want 404", path, rec.Code)
		}
		if !strings.Contains(rec.Body.String(), "No route matches") {
			t.Errorf("%s body=%q; expected route-not-found envelope", path, rec.Body.String())
		}
	}
}

func TestSQLToolsCapabilities_RegisteredWhenEnabled(t *testing.T) {
	srv := NewServer(Options{EnableSQLToolsAPI: true}, handlers.BuildDependencies(nil), nil)
	req := httptest.NewRequest(http.MethodGet, "/api/emulator/sql/capabilities", nil)
	req.RemoteAddr = "127.0.0.1:1234"
	rec := httptest.NewRecorder()
	srv.ServeHTTP(rec, req)
	if rec.Code != http.StatusOK {
		t.Fatalf("GET /api/emulator/sql/capabilities -> %d, want 200; body=%s",
			rec.Code, rec.Body.String())
	}
	if !strings.Contains(rec.Body.String(), `"sqlTools":true`) {
		t.Fatalf("body=%q; expected sqlTools capability", rec.Body.String())
	}
}

func TestSQLToolsRoutes_RemoteCallerDeniedWithoutAllow(t *testing.T) {
	srv := NewServer(Options{EnableSQLToolsAPI: true}, handlers.BuildDependencies(nil), nil)
	req := httptest.NewRequest(http.MethodGet, "/api/emulator/sql/capabilities", nil)
	req.RemoteAddr = "10.0.0.5:1234"
	rec := httptest.NewRecorder()
	srv.ServeHTTP(rec, req)
	if rec.Code != http.StatusForbidden {
		t.Errorf("status=%d, want 403", rec.Code)
	}
}
