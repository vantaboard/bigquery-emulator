package middleware

import (
	"net/http"
	"net/http/httptest"
	"testing"
)

func TestWithLoopbackTagFlagsLoopback(t *testing.T) {
	cases := []struct {
		name       string
		remoteAddr string
		want       bool
	}{
		{"ipv4-loopback", "127.0.0.1:54321", true},
		{"ipv4-loopback-other-port", "127.0.0.7:1", true},
		{"ipv6-loopback", "[::1]:9050", true},
		{"public-ipv4", "203.0.113.10:443", false},
		{"private-ipv4", "10.0.0.42:9050", false},
		{"public-ipv6", "[2001:db8::1]:443", false},
		{"empty-unix-socket", "", true},
		{"malformed", "not-a-valid-addr", false},
		{"hostname-not-ip", "localhost:9050", false},
	}
	for _, c := range cases {
		t.Run(c.name, func(t *testing.T) {
			var observed bool
			handler := WithLoopbackTag(http.HandlerFunc(func(_ http.ResponseWriter, r *http.Request) {
				observed = IsLoopback(r.Context())
			}))
			req := httptest.NewRequest(http.MethodGet, "http://example.test/x", nil)
			req.RemoteAddr = c.remoteAddr
			rec := httptest.NewRecorder()
			handler.ServeHTTP(rec, req)
			if observed != c.want {
				t.Fatalf("IsLoopback(ctx)=%v for RemoteAddr=%q, want %v",
					observed, c.remoteAddr, c.want)
			}
		})
	}
}

// TestIsLoopbackDefaultsFalseWithoutMiddleware pins the contract that
// a handler reached without the loopback middleware sees a non-
// loopback flag. This matters because the production default for any
// emulator-internal debug field is "do not surface" -- a bypass would
// otherwise quietly expose those fields.
func TestIsLoopbackDefaultsFalseWithoutMiddleware(t *testing.T) {
	req := httptest.NewRequest(http.MethodGet, "http://example.test/x", nil)
	if IsLoopback(req.Context()) {
		t.Fatal("IsLoopback returned true for a context without WithLoopbackTag")
	}
}
