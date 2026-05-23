// Package middleware contains HTTP middleware for the BigQuery emulator
// gateway. The middleware here is intentionally permissive: the emulator
// follows cloud-spanner-emulator's posture and parses but does not
// validate authentication credentials.
//
// See docs/REST_API.md ("Authentication posture") and ROADMAP.md Phase 1
// for the rationale: clients reuse their real BigQuery code paths by
// pointing at the emulator, and that code unconditionally sends a bearer
// token. Rejecting those tokens would force every client to special-case
// the emulator, which we explicitly want to avoid.
package middleware

import (
	"context"
	"net/http"
	"strings"
)

// principalCtxKey is the unexported context-key type used to stash the
// synthetic [Principal] on each request. Following the standard library
// guidance, we use a private named type so callers cannot collide with
// our key by accident.
type principalCtxKey struct{}

// Principal is the synthetic identity the emulator attributes to every
// request. The fields are populated by [WithAuth]; consumers retrieve
// the value via [PrincipalFromContext].
//
// The emulator is deliberately credulous: it does not validate the
// bearer token, look up an account, or check IAM. Handlers that need
// to differentiate authenticated vs anonymous traffic should consult
// [Principal.Anonymous].
type Principal struct {
	// Email is the synthetic account email attributed to the caller.
	// It is the same value for every request and exists only so logs
	// and (eventually) audit trails have a stable subject string.
	Email string

	// Bearer is the raw token the client presented in the Authorization
	// header, with the "Bearer " prefix stripped. Empty when no
	// Authorization header was sent or the header could not be parsed.
	Bearer string

	// Anonymous is true when the request did not present an
	// Authorization header at all. A request with a malformed header is
	// still considered non-anonymous: the emulator only cares whether
	// the client tried, not whether the credential is well-formed.
	Anonymous bool
}

// defaultPrincipalEmail is the synthetic email used for every request.
// It mirrors cloud-spanner-emulator, which similarly attributes all
// traffic to a fixed local identity.
const defaultPrincipalEmail = "emulator@bigquery.local"

// WithAuth returns middleware that parses the Authorization header (if
// present) and attaches a [Principal] to the request context. It never
// short-circuits the response: every request is allowed through with a
// synthetic identity, matching cloud-spanner-emulator's posture and the
// emulator's documented behavior in docs/REST_API.md.
//
// The middleware accepts any non-empty Authorization header. RFC 6750
// "Bearer" tokens have the "Bearer " prefix stripped; other schemes are
// stored verbatim in [Principal.Bearer]. The header is never logged.
func WithAuth(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		p := Principal{
			Email:     defaultPrincipalEmail,
			Anonymous: true,
		}
		if h := strings.TrimSpace(r.Header.Get("Authorization")); h != "" {
			p.Anonymous = false
			if scheme, token, ok := strings.Cut(h, " "); ok && strings.EqualFold(scheme, "Bearer") {
				p.Bearer = strings.TrimSpace(token)
			} else {
				p.Bearer = h
			}
		}
		ctx := context.WithValue(r.Context(), principalCtxKey{}, p)
		next.ServeHTTP(w, r.WithContext(ctx))
	})
}

// PrincipalFromContext extracts the [Principal] previously attached by
// [WithAuth]. The boolean is false when the context has no principal,
// which should only happen on requests that bypass the middleware (such
// as direct calls in tests).
func PrincipalFromContext(ctx context.Context) (Principal, bool) {
	p, ok := ctx.Value(principalCtxKey{}).(Principal)
	return p, ok
}
