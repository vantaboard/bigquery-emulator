package middleware

import (
	"context"
	"net/http"
	"net/http/httptest"
	"testing"
)

// captureHandler is a tiny http.Handler that records the request
// context for later inspection by the test.
type captureHandler struct {
	ctx context.Context
}

func (c *captureHandler) ServeHTTP(_ http.ResponseWriter, r *http.Request) {
	c.ctx = r.Context()
}

func TestWithAuthNoHeaderIsAnonymous(t *testing.T) {
	cap := &captureHandler{}
	srv := WithAuth(cap)

	req := httptest.NewRequest(http.MethodGet, "/anything", nil)
	rec := httptest.NewRecorder()
	srv.ServeHTTP(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200 (middleware must never short-circuit)", rec.Code)
	}
	p, ok := PrincipalFromContext(cap.ctx)
	if !ok {
		t.Fatal("PrincipalFromContext returned ok=false; middleware did not attach a principal")
	}
	if !p.Anonymous {
		t.Fatalf("Anonymous = false, want true (no Authorization header)")
	}
	if p.Bearer != "" {
		t.Fatalf("Bearer = %q, want empty", p.Bearer)
	}
	if p.Email == "" {
		t.Fatal("Email is empty; the emulator must attribute a synthetic email to every request")
	}
}

func TestWithAuthBearerTokenIsParsed(t *testing.T) {
	cap := &captureHandler{}
	srv := WithAuth(cap)

	req := httptest.NewRequest(http.MethodGet, "/anything", nil)
	req.Header.Set("Authorization", "Bearer ya29.fake-token")
	rec := httptest.NewRecorder()
	srv.ServeHTTP(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; bearer tokens must never trigger 401", rec.Code)
	}
	p, ok := PrincipalFromContext(cap.ctx)
	if !ok {
		t.Fatal("PrincipalFromContext returned ok=false")
	}
	if p.Anonymous {
		t.Fatal("Anonymous = true, want false (Authorization header was present)")
	}
	if p.Bearer != "ya29.fake-token" {
		t.Fatalf("Bearer = %q, want %q", p.Bearer, "ya29.fake-token")
	}
}

func TestWithAuthBearerSchemeIsCaseInsensitive(t *testing.T) {
	cap := &captureHandler{}
	srv := WithAuth(cap)

	req := httptest.NewRequest(http.MethodGet, "/anything", nil)
	req.Header.Set("Authorization", "bearer abc.def")
	rec := httptest.NewRecorder()
	srv.ServeHTTP(rec, req)

	p, ok := PrincipalFromContext(cap.ctx)
	if !ok {
		t.Fatal("PrincipalFromContext returned ok=false")
	}
	if p.Bearer != "abc.def" {
		t.Fatalf("Bearer = %q, want %q (scheme match must be case-insensitive)", p.Bearer, "abc.def")
	}
}

func TestWithAuthMalformedHeaderDoesNotReject(t *testing.T) {
	cap := &captureHandler{}
	srv := WithAuth(cap)

	req := httptest.NewRequest(http.MethodGet, "/anything", nil)
	req.Header.Set("Authorization", "garbage-no-scheme")
	rec := httptest.NewRecorder()
	srv.ServeHTTP(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; malformed Authorization headers must not trigger 401", rec.Code)
	}
	p, ok := PrincipalFromContext(cap.ctx)
	if !ok {
		t.Fatal("PrincipalFromContext returned ok=false")
	}
	if p.Anonymous {
		t.Fatal("Anonymous = true, want false (header was present, even if malformed)")
	}
	if p.Bearer != "garbage-no-scheme" {
		t.Fatalf("Bearer = %q, want %q (non-Bearer schemes are stored verbatim)", p.Bearer, "garbage-no-scheme")
	}
}

func TestPrincipalFromContextMissing(t *testing.T) {
	if _, ok := PrincipalFromContext(context.Background()); ok {
		t.Fatal("PrincipalFromContext returned ok=true on a context with no principal")
	}
}
