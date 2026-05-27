package seed

import (
	"strings"
	"testing"
)

// pinIDs swaps the store's id generator for a deterministic counter
// so tests can assert on stable operation ids.
func pinIDs(s *Store) {
	var i int
	s.idGen = func() string {
		i++
		return "op-test-" + itoa(i)
	}
}

// itoa avoids depending on strconv just for the per-test id helper.
func itoa(n int) string {
	if n == 0 {
		return "0"
	}
	var buf [20]byte
	pos := len(buf)
	for n > 0 {
		pos--
		buf[pos] = byte('0' + n%10)
		n /= 10
	}
	return string(buf[pos:])
}

// TestStore_NewAssignsIDAndPending pins the freshly-minted shape.
// The store should hand back a snapshot the caller can mutate
// without racing the goroutine that later runs the operation.
func TestStore_NewAssignsIDAndPending(t *testing.T) {
	s := NewStore()
	pinIDs(s)
	op := s.New(SeedRequest{Source: SeedEndpointRef{Project: "p"}})

	if op.ID != "op-test-1" {
		t.Errorf("ID=%q, want %q (id generator unstable?)", op.ID, "op-test-1")
	}
	if op.State != OperationPending {
		t.Errorf("State=%s, want PENDING", op.State)
	}
	if op.Started.IsZero() {
		t.Error("Started not stamped on New")
	}

	// Mutating the returned op must not affect the stored copy.
	op.State = OperationDone
	got := s.Get("op-test-1")
	if got == nil {
		t.Fatal("Get returned nil for freshly-minted op")
	}
	if got.State != OperationPending {
		t.Errorf("stored State=%s, want PENDING (snapshot leaked)", got.State)
	}
}

// TestStore_LifecycleTransitions walks the canonical happy path:
// PENDING -> RUNNING -> DONE. The result and finished timestamp
// must come out on the polling read.
func TestStore_LifecycleTransitions(t *testing.T) {
	s := NewStore()
	pinIDs(s)
	op := s.New(SeedRequest{Source: SeedEndpointRef{Project: "p"}})

	if !s.MarkRunning(op.ID) {
		t.Fatal("MarkRunning returned false")
	}
	if got := s.Get(op.ID); got.State != OperationRunning {
		t.Errorf("State=%s, want RUNNING", got.State)
	}

	result := &SeedResult{DatasetsCreated: 1}
	if !s.MarkResult(op.ID, result) {
		t.Fatal("MarkResult returned false")
	}
	got := s.Get(op.ID)
	if got.State != OperationDone {
		t.Errorf("State=%s, want DONE", got.State)
	}
	if got.Result == nil || got.Result.DatasetsCreated != 1 {
		t.Errorf("Result=%+v, want DatasetsCreated=1", got.Result)
	}
	if got.Finished.IsZero() {
		t.Error("Finished not stamped on DONE transition")
	}
}

// TestStore_FailedSetsFatalErr pins the FAILED branch separately so
// a regression here does not hide behind a happy-path test.
func TestStore_FailedSetsFatalErr(t *testing.T) {
	s := NewStore()
	op := s.New(SeedRequest{Source: SeedEndpointRef{Project: "p"}})
	s.MarkRunning(op.ID)
	if !s.MarkFailed(op.ID, "boom") {
		t.Fatal("MarkFailed returned false")
	}
	got := s.Get(op.ID)
	if got.State != OperationFailed {
		t.Errorf("State=%s, want FAILED", got.State)
	}
	if got.FatalErr != "boom" {
		t.Errorf("FatalErr=%q, want %q", got.FatalErr, "boom")
	}
	if got.Finished.IsZero() {
		t.Error("Finished not stamped on FAILED transition")
	}
}

// TestStore_MarkRunning_GuardsState ensures double-RUNNING (e.g.
// from a buggy retry loop) is a no-op rather than corrupting state.
func TestStore_MarkRunning_GuardsState(t *testing.T) {
	s := NewStore()
	op := s.New(SeedRequest{Source: SeedEndpointRef{Project: "p"}})
	if !s.MarkRunning(op.ID) {
		t.Fatal("first MarkRunning returned false")
	}
	if s.MarkRunning(op.ID) {
		t.Error("second MarkRunning returned true; should reject non-PENDING")
	}
}

// TestStore_GetUnknownReturnsNil is the contract the HTTP handler
// relies on for the 404 path.
func TestStore_GetUnknownReturnsNil(t *testing.T) {
	s := NewStore()
	if s.Get("no-such-op") != nil {
		t.Error("Get returned non-nil for unknown id")
	}
}

// TestStore_RandomIDFormat exists so a refactor that changes the
// id format flags itself; ids show up in operator logs and in any
// downstream tooling.
func TestStore_RandomIDFormat(t *testing.T) {
	s := NewStore() // real (non-pinned) idGen
	op := s.New(SeedRequest{Source: SeedEndpointRef{Project: "p"}})
	if !strings.HasPrefix(op.ID, "op-") {
		t.Errorf("ID=%q does not start with op- prefix", op.ID)
	}
	if len(op.ID) < 5 {
		t.Errorf("ID=%q suspiciously short", op.ID)
	}
}
