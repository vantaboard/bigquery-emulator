package seed

import (
	"net"
	"net/http"
	"strings"
)

// AccessConfig captures the safety knobs gated on the seed routes.
// The handler closes over one of these per gateway process; mutating
// the struct after registration has no effect.
type AccessConfig struct {
	// AllowRemote, when false (the default), rejects any request
	// whose RemoteAddr is not loopback (127.0.0.0/8 or ::1). The
	// rationale is the same one go-googlesql encodes: a seed
	// operation pulls down real production data and writes it
	// into a local emulator -- the call must originate from the
	// operator who owns both endpoints, not from a co-tenant
	// reachable on the LAN.
	AllowRemote bool

	// Token, when non-empty, requires every request to carry a
	// matching `X-BigQuery-Emulator-Seed-Token` header. This is
	// the additional defense for the
	// `--seed-api-allow-remote=true` case (CI runners, ephemeral
	// VMs) where loopback enforcement is not viable.
	Token string
}

// HeaderName is the canonical header name the token check reads.
// Exported so tests don't have to duplicate the literal.
const HeaderName = "X-BigQuery-Emulator-Seed-Token"

// CheckAccess enforces the loopback / token gates on r and returns
// nil when the request is allowed. On denial, returns a reason
// suitable for the BigQuery error envelope so the handler can map
// straight to 403.
//
// Order: loopback first, then token. The loopback rejection always
// wins so a misconfigured operator who left `--seed-api-allow-remote`
// off but is also sending a token doesn't get confused about which
// gate fired.
func (c AccessConfig) CheckAccess(r *http.Request) error {
	if !c.AllowRemote {
		if !isLoopback(r.RemoteAddr) {
			return ErrAccessDenied
		}
	}
	if c.Token != "" {
		got := r.Header.Get(HeaderName)
		if !secureEqual(got, c.Token) {
			return ErrAccessDenied
		}
	}
	return nil
}

// ErrAccessDenied is the sentinel CheckAccess returns. We don't
// distinguish between "wrong remote" and "wrong token" so an
// attacker probing the seed endpoint can't tell which check fired.
var ErrAccessDenied = httpError{code: http.StatusForbidden, msg: "seed: access denied"}

// httpError carries both the HTTP status the handler must write and
// the human-readable message. Implements error so it survives
// errors.Is comparisons.
type httpError struct {
	code int
	msg  string
}

func (e httpError) Error() string { return e.msg }

// Status returns the HTTP status code the handler should respond
// with for this error.
func (e httpError) Status() int { return e.code }

// secureEqual compares two strings in constant time wrt length.
// Constant-time only matters for the token comparison, but isolating
// the helper keeps the call site obvious.
func secureEqual(a, b string) bool {
	if len(a) != len(b) {
		return false
	}
	var diff byte
	for i := range len(a) {
		diff |= a[i] ^ b[i]
	}
	return diff == 0
}

// isLoopback reports whether remoteAddr (in net/http's
// `host:port` form) is on the local machine. We accept "no port" too
// because tests sometimes inject just an IP for httptest.
func isLoopback(remoteAddr string) bool {
	host, _, err := net.SplitHostPort(remoteAddr)
	if err != nil {
		host = remoteAddr
	}
	host = strings.TrimSpace(host)
	if host == "" {
		// Unix-socket / undefined caller; treat as loopback so
		// internal callers (e.g. a gateway that's bound to a
		// unix socket) aren't locked out.
		return true
	}
	ip := net.ParseIP(host)
	if ip == nil {
		return false
	}
	return ip.IsLoopback()
}
