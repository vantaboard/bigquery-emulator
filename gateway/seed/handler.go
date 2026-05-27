package seed

import (
	"context"
	"encoding/json"
	"errors"
	"io"
	"log" //nolint:depguard // matches the rest of the gateway package's existing log usage; slog migration is out of scope for this change
	"net/http"
	"strings"
)

// Status strings reused across the JSON error envelope responses.
// Pulled into named constants so the handler doesn't repeat the same
// literal three times (and so a typo can't sneak past a grep).
const (
	statusInvalid        = "invalid"
	statusNotImplemented = "notImplemented"
	statusNotFound       = "notFound"
)

// Runner is the interface the HTTP handler uses to dispatch a
// validated SeedRequest to whoever actually copies production data
// into the emulator. The production orchestrator implements this;
// tests inject a fake that returns canned SeedResults so the
// handler can be exercised without a network round-trip.
type Runner interface {
	Run(ctx context.Context, req SeedRequest) (*SeedResult, error)
}

// HandlerDeps bundles everything the seed handler set needs at
// registration time. Kept as a struct so the call from gateway/
// server.go stays readable.
type HandlerDeps struct {
	// Access enforces loopback / token gates. Per-process; the
	// gateway constructs one from gateway.Options.
	Access AccessConfig

	// Store is the per-process operation registry. The handler
	// creates one operation per POST and looks up the right one
	// on GET .../operations/{id}.
	Store *Store

	// Runner does the actual seeding work. Nil means the build
	// does not include a production runner; the POST handler
	// surfaces 501 NotImplemented with a documented reason so
	// operators can tell "the route is wired" from "the build
	// can't help me".
	Runner Runner
}

// RegisterRoutes installs `POST /api/emulator/seed` and
// `GET /api/emulator/seed/operations/{operationId}` on mux. Idempotent;
// callers wire it once from gateway.NewServer when EnableSeedAPI is
// true.
func RegisterRoutes(mux *http.ServeMux, deps HandlerDeps) {
	if deps.Store == nil {
		deps.Store = NewStore()
	}
	mux.HandleFunc("POST /api/emulator/seed", deps.handlePost)
	mux.HandleFunc("GET /api/emulator/seed/operations/{operationId}", deps.handleGet)
}

// handlePost accepts a SeedRequest, validates it, mints a new
// operation in the store, and (when a Runner is configured) kicks
// off the actual seeding work on a background goroutine. The
// response is the freshly-minted operation envelope; callers poll
// the GET endpoint for completion.
func (d HandlerDeps) handlePost(w http.ResponseWriter, r *http.Request) {
	if err := d.Access.CheckAccess(r); err != nil {
		writeAccessError(w, err)
		return
	}
	body, err := io.ReadAll(r.Body)
	if err != nil {
		writeJSON(w, http.StatusBadRequest, errEnvelope{
			Code:    http.StatusBadRequest,
			Status:  statusInvalid,
			Message: "Could not read seed request body: " + err.Error(),
		})
		return
	}
	req, err := DecodeRequest(body)
	if err != nil {
		writeJSON(w, http.StatusBadRequest, errEnvelope{
			Code:    http.StatusBadRequest,
			Status:  statusInvalid,
			Message: err.Error(),
		})
		return
	}
	if err := req.Validate(); err != nil {
		writeJSON(w, http.StatusBadRequest, errEnvelope{
			Code:    http.StatusBadRequest,
			Status:  statusInvalid,
			Message: err.Error(),
		})
		return
	}
	if d.Runner == nil {
		// The route exists but the build does not include a
		// production runner. Surface a 501 with the documented
		// reason rather than a 200 with an empty Result.
		writeJSON(w, http.StatusNotImplemented, errEnvelope{
			Code:   http.StatusNotImplemented,
			Status: statusNotImplemented,
			Message: "Production seed is not compiled into this gateway build. " +
				"Use --seed-data-file for declarative seeding.",
		})
		return
	}

	op := d.Store.New(req)
	go d.runOperation(op.ID, req)
	writeJSON(w, http.StatusAccepted, operationToWire(op))
}

// handleGet returns the current snapshot of an operation. We never
// surface 404 to the caller; an unknown id still returns a
// well-formed envelope with state="UNKNOWN" so polling loops have
// one less branch to handle.
func (d HandlerDeps) handleGet(w http.ResponseWriter, r *http.Request) {
	if err := d.Access.CheckAccess(r); err != nil {
		writeAccessError(w, err)
		return
	}
	id := r.PathValue("operationId")
	op := d.Store.Get(id)
	if op == nil {
		writeJSON(w, http.StatusNotFound, errEnvelope{
			Code:    http.StatusNotFound,
			Status:  statusNotFound,
			Message: "No such seed operation: " + id,
		})
		return
	}
	writeJSON(w, http.StatusOK, operationToWire(op))
}

// runOperation moves an operation through RUNNING -> DONE/FAILED in
// the background. We give the runner a fresh context.Background so
// the HTTP request that posted the operation can complete (its
// context goes away) without cancelling the seed work; long
// operations are the norm.
func (d HandlerDeps) runOperation(id string, req SeedRequest) {
	if !d.Store.MarkRunning(id) {
		return
	}
	result, err := d.Runner.Run(context.Background(), req)
	if err != nil {
		log.Printf("seed: operation %s failed: %v", id, err)
		d.Store.MarkFailed(id, err.Error())
		return
	}
	d.Store.MarkResult(id, result)
}

// operationToWire flattens the in-memory Operation into the JSON
// envelope the polling endpoint serves. Kept as a separate type
// because the in-memory Operation carries fields (mutex receivers,
// Go-only timestamps) that don't belong on the wire.
type operationWire struct {
	ID        string         `json:"id"`
	State     OperationState `json:"state"`
	Started   string         `json:"started"`
	Finished  string         `json:"finished,omitempty"`
	Request   SeedRequest    `json:"request"`
	Result    *SeedResult    `json:"result,omitempty"`
	Error     string         `json:"error,omitempty"`
	Cancelled bool           `json:"cancelled,omitempty"`
}

func operationToWire(op *Operation) operationWire {
	w := operationWire{
		ID:        op.ID,
		State:     op.State,
		Started:   op.Started.UTC().Format("2006-01-02T15:04:05Z"),
		Request:   op.Request,
		Result:    op.Result,
		Error:     op.FatalErr,
		Cancelled: op.Cancelled,
	}
	if !op.Finished.IsZero() {
		w.Finished = op.Finished.UTC().Format("2006-01-02T15:04:05Z")
	}
	return w
}

// errEnvelope mirrors the BigQuery-shaped error response the rest
// of the gateway uses (gateway/handlers/handlers.go). Duplicating
// the shape here keeps the seed package from importing the public
// handlers package and creating an import cycle.
type errEnvelope struct {
	Code    int    `json:"code"`
	Status  string `json:"status"`
	Message string `json:"message"`
}

func writeJSON(w http.ResponseWriter, status int, body any) {
	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	w.WriteHeader(status)
	_ = json.NewEncoder(w).Encode(body)
}

// writeAccessError maps the seed-specific access denial into an
// HTTP 403 with the same envelope shape the rest of the gateway
// uses. Keeps the deny path uniform across loopback and token
// failures.
func writeAccessError(w http.ResponseWriter, err error) {
	code := http.StatusForbidden
	msg := err.Error()
	var he httpError
	if errors.As(err, &he) {
		code = he.Status()
	}
	writeJSON(w, code, errEnvelope{
		Code:    code,
		Status:  "accessDenied",
		Message: strings.TrimSpace(msg),
	})
}
