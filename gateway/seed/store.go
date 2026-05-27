package seed

import (
	"crypto/rand"
	"encoding/hex"
	"sync"
	"time"
)

// OperationState enumerates the lifecycle states a seed operation
// moves through.
//
//	Pending  -> Running -> Done | Failed
type OperationState string

const (
	OperationPending OperationState = "PENDING"
	OperationRunning OperationState = "RUNNING"
	OperationDone    OperationState = "DONE"
	OperationFailed  OperationState = "FAILED"
)

// Operation is the persisted view of an in-flight or completed seed
// operation. The HTTP handler converts this into the wire-shape
// the polling endpoint serves; keeping it as a Go struct (rather
// than a raw map) makes the store's tests easier to read.
//
// Started/Finished use time.Time even though only Finished can be
// zero-valued (Started is always stamped on New). The wire-shape
// formatting -- including omitting an empty Finished -- happens in
// operationToWire over in handler.go, so this struct itself is
// purely the in-memory view and the json tags are minimal.
type Operation struct {
	ID        string         `json:"id"`
	State     OperationState `json:"state"`
	Started   time.Time      `json:"started"`
	Finished  time.Time      `json:"finished"`
	Request   SeedRequest    `json:"request"`
	Result    *SeedResult    `json:"result,omitempty"`
	FatalErr  string         `json:"error,omitempty"`
	Cancelled bool           `json:"cancelled,omitempty"`
}

// Store holds the per-process operation registry. The HTTP handler
// stores newly-minted operations here and reads them back on poll;
// the orchestrator drives state transitions via Mark*. The store is
// in-memory; restarting the gateway forgets every operation, which
// is consistent with the rest of the emulator's lifecycle.
type Store struct {
	mu  sync.Mutex
	ops map[string]*Operation

	// idGen mints opaque operation IDs. Pulled out into a func
	// so tests can pin "operationN" instead of random hex.
	idGen func() string
}

// NewStore constructs an empty operation registry. Each gateway
// process owns one; the seed handler closes over it.
func NewStore() *Store {
	return &Store{
		ops:   make(map[string]*Operation),
		idGen: newRandomID,
	}
}

// New registers a fresh operation in PENDING state and returns it.
// Callers immediately transition to RUNNING via MarkRunning when the
// orchestrator picks it up.
func (s *Store) New(req SeedRequest) *Operation {
	s.mu.Lock()
	defer s.mu.Unlock()
	op := &Operation{
		ID:      s.idGen(),
		State:   OperationPending,
		Started: time.Now().UTC(),
		Request: req,
	}
	s.ops[op.ID] = op
	return cloneOperation(op)
}

// Get returns a snapshot of the operation with the given ID.
// Returns nil when no such operation exists.
func (s *Store) Get(id string) *Operation {
	s.mu.Lock()
	defer s.mu.Unlock()
	op, ok := s.ops[id]
	if !ok {
		return nil
	}
	return cloneOperation(op)
}

// MarkRunning records that the orchestrator has started processing
// the operation. No-op (returns false) when the operation doesn't
// exist or has already left PENDING.
func (s *Store) MarkRunning(id string) bool {
	s.mu.Lock()
	defer s.mu.Unlock()
	op, ok := s.ops[id]
	if !ok || op.State != OperationPending {
		return false
	}
	op.State = OperationRunning
	return true
}

// MarkResult records a successful (or partially-successful)
// completion. The operation's Finished timestamp is stamped here.
func (s *Store) MarkResult(id string, result *SeedResult) bool {
	s.mu.Lock()
	defer s.mu.Unlock()
	op, ok := s.ops[id]
	if !ok {
		return false
	}
	op.State = OperationDone
	op.Result = result
	op.Finished = time.Now().UTC()
	return true
}

// MarkFailed records a catastrophic failure (unreadable production,
// missing creds, ...). Per-resource failures should be folded into
// Result.ResourceErrors and reported via MarkResult instead.
func (s *Store) MarkFailed(id, errMsg string) bool {
	s.mu.Lock()
	defer s.mu.Unlock()
	op, ok := s.ops[id]
	if !ok {
		return false
	}
	op.State = OperationFailed
	op.FatalErr = errMsg
	op.Finished = time.Now().UTC()
	return true
}

// cloneOperation returns a deep-enough copy of op so the caller can
// mutate it without racing other goroutines reading from the store.
// The struct is small and Result is repointed (we never mutate a
// Result after passing it through MarkResult), so a shallow copy is
// sufficient.
func cloneOperation(op *Operation) *Operation {
	cp := *op
	return &cp
}

// newRandomID mints a 16-character hex ID. 64 bits of entropy is
// plenty -- the seed store is in-memory and never federated, so
// collision risk is bounded by the lifetime of one gateway process.
func newRandomID() string {
	var b [8]byte
	if _, err := rand.Read(b[:]); err != nil {
		// crypto/rand is documented to never fail in practice
		// on modern OSes; if it does, fall back to a clearly
		// debug-able id rather than panic.
		return "op-rand-error"
	}
	return "op-" + hex.EncodeToString(b[:])
}
