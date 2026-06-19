package sqltools

import (
	"net"
	"net/http"
	"strings"
)

// AccessConfig captures loopback / token gates for the SQL tools routes.
type AccessConfig struct {
	AllowRemote bool
	Token       string
}

// HeaderName is the canonical token header for remote SQL tools access.
const HeaderName = "X-BigQuery-Emulator-SqlTools-Token"

// CheckAccess enforces the loopback / token gates on r.
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

// ErrAccessDenied is returned when access checks fail.
var ErrAccessDenied = httpError{code: http.StatusForbidden, msg: "sqltools: access denied"}

type httpError struct {
	code int
	msg  string
}

func (e httpError) Error() string { return e.msg }

// Status returns the HTTP status for this error.
func (e httpError) Status() int { return e.code }

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

func isLoopback(remoteAddr string) bool {
	host, _, err := net.SplitHostPort(remoteAddr)
	if err != nil {
		host = remoteAddr
	}
	host = strings.TrimSpace(host)
	if host == "" {
		return true
	}
	ip := net.ParseIP(host)
	if ip == nil {
		return false
	}
	return ip.IsLoopback()
}
