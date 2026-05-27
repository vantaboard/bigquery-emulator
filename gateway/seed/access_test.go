package seed

import (
	"errors"
	"net/http"
	"net/http/httptest"
	"testing"
)

// reqFrom builds a request with the given RemoteAddr and optional
// header pair so the access tests can set up the wire conditions
// without going through the full mux.
func reqFrom(remoteAddr, headerKey, headerVal string) *http.Request {
	r := httptest.NewRequest(http.MethodPost, "/api/emulator/seed", nil)
	r.RemoteAddr = remoteAddr
	if headerKey != "" {
		r.Header.Set(headerKey, headerVal)
	}
	return r
}

// TestAccess_LoopbackByDefault pins the safest posture: AllowRemote
// off rejects anyone whose RemoteAddr is not on the loopback range.
func TestAccess_LoopbackByDefault(t *testing.T) {
	a := AccessConfig{}
	cases := []struct {
		name       string
		remoteAddr string
		wantOK     bool
	}{
		{"ipv4-loopback", "127.0.0.1:1234", true},
		{"ipv4-loopback-range", "127.5.5.5:1234", true},
		{"ipv6-loopback", "[::1]:1234", true},
		{"private-lan", "10.0.0.5:1234", false},
		{"public-internet", "8.8.8.8:1234", false},
		{"empty-remoteaddr", "", true},
	}
	for _, c := range cases {
		t.Run(c.name, func(t *testing.T) {
			err := a.CheckAccess(reqFrom(c.remoteAddr, "", ""))
			if (err == nil) != c.wantOK {
				t.Errorf("CheckAccess(%q) err=%v, wantOK=%v",
					c.remoteAddr, err, c.wantOK)
			}
		})
	}
}

// TestAccess_AllowRemoteSkipsLoopback verifies the CI/CD opt-in
// path: AllowRemote=true accepts non-loopback callers (when no
// token is set).
func TestAccess_AllowRemoteSkipsLoopback(t *testing.T) {
	a := AccessConfig{AllowRemote: true}
	if err := a.CheckAccess(reqFrom("10.0.0.5:1234", "", "")); err != nil {
		t.Errorf("AllowRemote=true loopback check: %v", err)
	}
}

// TestAccess_TokenRequiredWhenSet pins the second-line defense:
// when an operator sets a token, every request -- loopback included
// -- must present a matching header.
func TestAccess_TokenRequiredWhenSet(t *testing.T) {
	a := AccessConfig{AllowRemote: true, Token: testSeedToken}

	if err := a.CheckAccess(reqFrom("10.0.0.5:1234", "", "")); err == nil {
		t.Error("missing token accepted")
	}
	if err := a.CheckAccess(reqFrom("10.0.0.5:1234", HeaderName, "wrong")); err == nil {
		t.Error("wrong token accepted")
	}
	if err := a.CheckAccess(reqFrom("10.0.0.5:1234", HeaderName, testSeedToken)); err != nil {
		t.Errorf("correct token rejected: %v", err)
	}
}

// TestAccess_LoopbackBeatsTokenOrder pins the documented
// short-circuit order: loopback fails before the token even gets
// inspected, so an attacker probing the seed endpoint with a guessed
// token still hits 403 without revealing whether the token was
// close.
func TestAccess_LoopbackBeatsTokenOrder(t *testing.T) {
	a := AccessConfig{Token: testSeedToken}
	err := a.CheckAccess(reqFrom("10.0.0.5:1234", HeaderName, testSeedToken))
	if err == nil {
		t.Error("non-loopback caller with correct token was accepted; AllowRemote=false must short-circuit")
	}
}

// TestAccess_HttpErrorStatus checks the sentinel maps to the right
// HTTP status the handler turns into the wire response.
func TestAccess_HttpErrorStatus(t *testing.T) {
	a := AccessConfig{}
	err := a.CheckAccess(reqFrom("8.8.8.8:1234", "", ""))
	if err == nil {
		t.Fatal("non-loopback caller accepted")
	}
	var he httpError
	ok := errors.As(err, &he)
	if !ok {
		t.Fatalf("error is not httpError: %T", err)
	}
	if he.Status() != http.StatusForbidden {
		t.Errorf("status=%d, want 403", he.Status())
	}
}
