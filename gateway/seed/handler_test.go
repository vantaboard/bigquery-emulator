package seed

import (
	"context"
	"encoding/json"
	"errors"
	"net/http"
	"net/http/httptest"
	"strings"
	"sync"
	"testing"
	"time"
)

// fakeRunner is the in-memory Runner used by handler tests. It can
// either return a canned result, sleep, or return an error so the
// handler test cases drive every branch.
type fakeRunner struct {
	mu       sync.Mutex
	calls    []SeedRequest
	result   *SeedResult
	err      error
	sleepFor time.Duration
}

func (f *fakeRunner) Run(_ context.Context, req SeedRequest) (*SeedResult, error) {
	if f.sleepFor > 0 {
		time.Sleep(f.sleepFor)
	}
	f.mu.Lock()
	f.calls = append(f.calls, req)
	f.mu.Unlock()
	return f.result, f.err
}

// doRequest builds an *httptest.ResponseRecorder by handing the
// request to a fresh ServeMux that has the seed routes registered
// on it. RemoteAddr defaults to loopback so the AccessConfig is
// never the blocker unless a test explicitly overrides it.
func doRequest(
	t *testing.T,
	deps HandlerDeps,
	method, path string,
	body string,
	remote string,
) *httptest.ResponseRecorder {
	t.Helper()
	mux := http.NewServeMux()
	RegisterRoutes(mux, deps)
	req := httptest.NewRequest(method, path, strings.NewReader(body))
	if remote != "" {
		req.RemoteAddr = remote
	} else {
		req.RemoteAddr = "127.0.0.1:1234"
	}
	rec := httptest.NewRecorder()
	mux.ServeHTTP(rec, req)
	return rec
}

// TestSeedHandler_PostAcceptsAndDispatches walks the happy path:
// the handler validates, mints an op, kicks off the runner, and
// returns 202 with the op envelope.
func TestSeedHandler_PostAcceptsAndDispatches(t *testing.T) {
	store := NewStore()
	pinIDs(store)
	runner := &fakeRunner{result: &SeedResult{DatasetsCreated: 1, TablesCreated: 1, RowsCopied: 5}}
	deps := HandlerDeps{
		Access: AccessConfig{},
		Store:  store,
		Runner: runner,
	}

	rec := doRequest(t, deps, http.MethodPost, "/api/emulator/seed",
		`{"source":{"project":"p","dataset":"d","table":"t"}}`, "")
	if rec.Code != http.StatusAccepted {
		t.Fatalf("POST status=%d, want 202; body=%s", rec.Code, rec.Body.String())
	}
	var got operationWire
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if got.ID == "" || got.State != OperationPending {
		t.Errorf("op envelope=%+v", got)
	}

	// Wait for the background goroutine to mark the op DONE so
	// the GET polling path is exercised against a complete op.
	deadline := time.Now().Add(2 * time.Second)
	for time.Now().Before(deadline) {
		if op := store.Get(got.ID); op != nil && op.State == OperationDone {
			break
		}
		time.Sleep(10 * time.Millisecond)
	}
	op := store.Get(got.ID)
	if op == nil || op.State != OperationDone {
		t.Fatalf("op never moved to DONE: %+v", op)
	}
	if op.Result == nil || op.Result.RowsCopied != 5 {
		t.Errorf("Result=%+v, want RowsCopied=5", op.Result)
	}

	// Now poll via the public GET endpoint.
	rec = doRequest(t, deps, http.MethodGet,
		"/api/emulator/seed/operations/"+got.ID, "", "")
	if rec.Code != http.StatusOK {
		t.Fatalf("GET status=%d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	var poll operationWire
	if err := json.NewDecoder(rec.Body).Decode(&poll); err != nil {
		t.Fatalf("decode poll: %v", err)
	}
	if poll.State != OperationDone || poll.Result == nil || poll.Result.RowsCopied != 5 {
		t.Errorf("poll envelope=%+v", poll)
	}
}

// TestSeedHandler_RejectsInvalidJSON pins the 400 path for body
// parse failures. The runner must not see a call.
func TestSeedHandler_RejectsInvalidJSON(t *testing.T) {
	runner := &fakeRunner{}
	deps := HandlerDeps{Store: NewStore(), Runner: runner}
	rec := doRequest(t, deps, http.MethodPost, "/api/emulator/seed",
		`{"source":{"project":}`, "")
	if rec.Code != http.StatusBadRequest {
		t.Errorf("status=%d, want 400", rec.Code)
	}
	if len(runner.calls) != 0 {
		t.Error("runner.Run invoked despite parse error")
	}
}

// TestSeedHandler_RejectsInvalidRequest pins the validation 400.
func TestSeedHandler_RejectsInvalidRequest(t *testing.T) {
	deps := HandlerDeps{Store: NewStore(), Runner: &fakeRunner{}}
	rec := doRequest(t, deps, http.MethodPost, "/api/emulator/seed",
		`{"source":{}}`, "")
	if rec.Code != http.StatusBadRequest {
		t.Errorf("status=%d, want 400", rec.Code)
	}
}

// TestSeedHandler_DenyRemoteByDefault pins the loopback gate at
// the handler boundary. A LAN caller gets 403 without the runner
// being touched.
func TestSeedHandler_DenyRemoteByDefault(t *testing.T) {
	deps := HandlerDeps{Store: NewStore(), Runner: &fakeRunner{}}
	rec := doRequest(t, deps, http.MethodPost, "/api/emulator/seed",
		`{"source":{"project":"p"}}`, "10.0.0.5:1234")
	if rec.Code != http.StatusForbidden {
		t.Errorf("status=%d, want 403", rec.Code)
	}
}

// TestSeedHandler_TokenRequired pins the token gate at the handler
// boundary. We use AllowRemote=true so loopback is not the
// confounder.
func TestSeedHandler_TokenRequired(t *testing.T) {
	deps := HandlerDeps{
		Access: AccessConfig{AllowRemote: true, Token: testSeedToken},
		Store:  NewStore(),
		Runner: &fakeRunner{result: &SeedResult{}},
	}
	rec := doRequest(t, deps, http.MethodPost, "/api/emulator/seed",
		`{"source":{"project":"p"}}`, "10.0.0.5:1234")
	if rec.Code != http.StatusForbidden {
		t.Errorf("status=%d, want 403", rec.Code)
	}

	// With the right token the request is accepted.
	mux := http.NewServeMux()
	RegisterRoutes(mux, deps)
	req := httptest.NewRequest(http.MethodPost, "/api/emulator/seed",
		strings.NewReader(`{"source":{"project":"p"}}`))
	req.RemoteAddr = "10.0.0.5:1234"
	req.Header.Set(HeaderName, testSeedToken)
	rec = httptest.NewRecorder()
	mux.ServeHTTP(rec, req)
	if rec.Code != http.StatusAccepted {
		t.Errorf("status=%d, want 202", rec.Code)
	}
}

// TestSeedHandler_RunnerNilReturns501 pins the documented
// degradation path when production seeding isn't compiled in.
func TestSeedHandler_RunnerNilReturns501(t *testing.T) {
	deps := HandlerDeps{Store: NewStore()} // Runner: nil
	rec := doRequest(t, deps, http.MethodPost, "/api/emulator/seed",
		`{"source":{"project":"p"}}`, "")
	if rec.Code != http.StatusNotImplemented {
		t.Errorf("status=%d, want 501", rec.Code)
	}
}

// TestSeedHandler_GetUnknownReturns404 verifies the polling
// endpoint's 404 envelope shape.
func TestSeedHandler_GetUnknownReturns404(t *testing.T) {
	deps := HandlerDeps{Store: NewStore(), Runner: &fakeRunner{}}
	rec := doRequest(t, deps, http.MethodGet,
		"/api/emulator/seed/operations/op-nope", "", "")
	if rec.Code != http.StatusNotFound {
		t.Errorf("status=%d, want 404", rec.Code)
	}
}

// TestSeedHandler_FailedRunSurfacesError pins how a runner-side
// fatal error materializes on the polling endpoint: state=FAILED,
// error populated, no result.
func TestSeedHandler_FailedRunSurfacesError(t *testing.T) {
	store := NewStore()
	pinIDs(store)
	runner := &fakeRunner{err: errors.New("ADC missing")}
	deps := HandlerDeps{Store: store, Runner: runner}

	rec := doRequest(t, deps, http.MethodPost, "/api/emulator/seed",
		`{"source":{"project":"p"}}`, "")
	if rec.Code != http.StatusAccepted {
		t.Fatalf("POST status=%d, want 202", rec.Code)
	}
	var posted operationWire
	if err := json.NewDecoder(rec.Body).Decode(&posted); err != nil {
		t.Fatalf("decode: %v", err)
	}

	deadline := time.Now().Add(2 * time.Second)
	for time.Now().Before(deadline) {
		if op := store.Get(posted.ID); op != nil && op.State == OperationFailed {
			break
		}
		time.Sleep(10 * time.Millisecond)
	}
	op := store.Get(posted.ID)
	if op == nil || op.State != OperationFailed {
		t.Fatalf("op state=%v, want FAILED", op)
	}
	if op.FatalErr == "" {
		t.Error("FatalErr not populated on FAILED")
	}
}
