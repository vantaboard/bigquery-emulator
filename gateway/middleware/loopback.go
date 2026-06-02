package middleware

import (
	"context"
	"net"
	"net/http"
)

// loopbackCtxKey is the unexported context-key type used to stash the
// per-request loopback flag [WithLoopbackTag] computes. Following the
// standard library guidance, we use a private named type so callers
// cannot collide with our key by accident.
type loopbackCtxKey struct{}

// WithLoopbackTag returns middleware that records whether the request
// arrived from a loopback caller (an HTTP client bound to `127.0.0.0/8`
// or `::1`, or a unix-socket connection where `RemoteAddr` is empty).
// The flag is stashed in the request context so handlers can decide
// whether to surface emulator-internal debug fields back to the caller
// without having to re-parse `r.RemoteAddr`.
//
// The single user today is the synchronous query handler, which uses
// the flag to gate `Job.statistics.query.emulatorRoute` (the canonical
// `Disposition` string the C++ coordinator's `RouteClassifier`
// produced for the query). The contract is that field is observable
// ONLY to loopback callers; non-loopback callers receive a response
// with the field omitted entirely, matching the public BigQuery REST
// surface byte-for-byte. See
// `.cursor/plans/conformance-routing-matrix.plan.md` for the wider
// rationale.
//
// The middleware never short-circuits the response: it only attaches
// a boolean to the context. Handlers that want loopback-only behavior
// call [IsLoopback]; handlers that don't care are unaffected.
func WithLoopbackTag(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		ctx := context.WithValue(r.Context(), loopbackCtxKey{},
			isLoopbackRemoteAddr(r.RemoteAddr))
		next.ServeHTTP(w, r.WithContext(ctx))
	})
}

// IsLoopback reports whether the request that owns this context
// originated from a loopback caller. It returns false when the
// context has no loopback flag (i.e. the request bypassed
// [WithLoopbackTag], which should only happen in direct unit tests
// that don't go through the middleware stack). Callers that need the
// inverse default for tests can adjust at the call site -- the
// middleware-bypass case is rare enough that "treat as non-loopback"
// is the safer default for production code.
func IsLoopback(ctx context.Context) bool {
	v, _ := ctx.Value(loopbackCtxKey{}).(bool)
	return v
}

// isLoopbackRemoteAddr returns true when `addr` is the standard Go
// `host:port` shape and the host resolves to a loopback IP. Unix-
// socket connections present an empty `RemoteAddr` on most servers
// (httptest's `httptest.Server` always binds TCP, but a unix-listener
// backed gateway leaves the field empty); we treat the empty string
// as loopback because that is the only realistic deployment shape
// for a unix-socket emulator.
//
// Malformed addresses are treated as non-loopback so a bug in an
// upstream proxy that strips the port can't accidentally elevate a
// public caller to loopback status.
func isLoopbackRemoteAddr(addr string) bool {
	if addr == "" {
		return true
	}
	host, _, err := net.SplitHostPort(addr)
	if err != nil {
		// `RemoteAddr` should be in `host:port` form per the
		// net/http documentation. A failure here means the input
		// is malformed; default to non-loopback so a misrouted
		// caller cannot accidentally observe loopback-only fields.
		return false
	}
	ip := net.ParseIP(host)
	if ip == nil {
		return false
	}
	return ip.IsLoopback()
}
