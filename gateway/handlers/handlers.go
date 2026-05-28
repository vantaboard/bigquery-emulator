// Package handlers contains HTTP handlers for the BigQuery REST surface.
//
// At this stage of the project most handlers are intentional stubs that
// return http.StatusNotImplemented. They exist so that:
//
//   - The route table in gateway/server.go is exhaustive and easy to scan,
//     which doubles as a checklist for the gateway-HTTP-surface section of
//     ROADMAP.md.
//   - Client libraries get a structurally-valid BigQuery error envelope
//     instead of a 404 when they hit something we have not implemented yet.
//   - Each handler can be flipped to a real implementation in isolation.
package handlers

import (
	"encoding/json"
	"net/http"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

// Dependencies bundles everything a handler might need to reach (engine
// gRPC client, in-memory catalog, logger, etc.). It grows as the gateway
// wires in real backends.
//
// Catalog and Query are the engine-side gRPC clients defined in
// proto/emulator.proto; both are nil when the gateway is started with
// --engine_binary="" (gateway-only / unit-test mode) and handlers
// must nil-check before dispatching to them.
type Dependencies struct {
	// Catalog is the gRPC client used by datasets/tables/tabledata
	// handlers to mirror catalog mutations into the engine.
	Catalog enginepb.CatalogClient

	// Query is the gRPC client used by jobs.query and the query branch
	// of jobs.insert to forward SQL execution to the engine.
	Query enginepb.QueryClient

	// Jobs is the in-memory job registry the synchronous jobs.query
	// handler records DONE jobs in, and that future jobs.get /
	// jobs.list handlers will read back from. When nil (legacy unit
	// tests that predate the registry), QueryRun lazily mints a
	// per-handler fallback so behavior stays compatible.
	Jobs *jobs.Registry
}

// Health is a trivial liveness endpoint useful for `docker-compose`
// health checks and CI smoke tests.
func Health(w http.ResponseWriter, r *http.Request) {
	writeJSON(w, http.StatusOK, map[string]string{
		"status":  "ok",
		"service": "bigquery-emulator",
	})
}

// NotImplemented returns a BigQuery-shaped 501 response. Used by routes
// that are registered but not yet implemented.
func NotImplemented(w http.ResponseWriter, r *http.Request) {
	writeError(w, http.StatusNotImplemented, "notImplemented",
		"This BigQuery emulator route is registered but not yet implemented. "+
			"See ROADMAP.md.")
}

// NotFound is the catch-all handler for paths not in the route table. It
// returns a BigQuery-shaped 404 so client libraries see a structured error.
func NotFound(w http.ResponseWriter, r *http.Request) {
	writeError(w, http.StatusNotFound, "notFound",
		"No route matches "+r.Method+" "+r.URL.Path+".")
}

// splitColonOp splits an AIP-136 custom-method path segment of the form
// "{resource}:{op}" into its resource and op halves. If there is no colon
// the op is returned empty and the input is the resource. This is how the
// emulator dispatches BigQuery REST custom methods like
// `datasets/{datasetId}:undelete` and `tables/{tableId}:getIamPolicy`,
// because Go's net/http mux cannot match a literal segment after a
// wildcard.
func splitColonOp(segment string) (resource, op string) {
	for i := 0; i < len(segment); i++ {
		if segment[i] == ':' {
			return segment[:i], segment[i+1:]
		}
	}
	return segment, ""
}

// errorEnvelope matches the shape BigQuery returns for non-2xx responses.
// See https://cloud.google.com/bigquery/docs/reference/rest -> error format.
type errorEnvelope struct {
	Error errorBody `json:"error"`
}

type errorBody struct {
	Code    int           `json:"code"`
	Message string        `json:"message"`
	Errors  []errorDetail `json:"errors,omitempty"`
	Status  string        `json:"status,omitempty"`
}

type errorDetail struct {
	Reason  string `json:"reason"`
	Message string `json:"message"`
	Domain  string `json:"domain,omitempty"`
}

func writeJSON(w http.ResponseWriter, status int, body any) {
	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	w.WriteHeader(status)
	_ = json.NewEncoder(w).Encode(body)
}

func writeError(w http.ResponseWriter, status int, reason, msg string) {
	writeJSON(w, status, errorEnvelope{
		Error: errorBody{
			Code:    status,
			Message: msg,
			Status:  reason,
			Errors: []errorDetail{{
				Reason:  reason,
				Message: msg,
				Domain:  "global",
			}},
		},
	})
}

// grpcToHTTPError translates a gRPC error returned by the engine into
// the BigQuery-shaped JSON error envelope and writes it to w. Returns
// true when err was non-nil (and therefore an error was written), so
// callers can use it as `if grpcToHTTPError(...) { return }`.
//
// The mapping mirrors the Storage→gRPC mapping in
// frontend/handlers/catalog.cc: NOT_FOUND → 404 notFound,
// ALREADY_EXISTS → 409 duplicate, INVALID_ARGUMENT → 400 invalid,
// FAILED_PRECONDITION → 400 failedPrecondition, UNIMPLEMENTED → 501
// notImplemented, UNAVAILABLE → 503 backendError. Anything else
// (INTERNAL, plain Go errors) is reported as 500 internalError so a
// misbehaving engine cannot be mistaken for a 404 on the wire.
func grpcToHTTPError(w http.ResponseWriter, err error) bool {
	if err == nil {
		return false
	}
	st, ok := status.FromError(err)
	if !ok {
		writeError(w, http.StatusInternalServerError, "internalError",
			"Engine RPC failed: "+err.Error())
		return true
	}
	httpStatus, reason := http.StatusInternalServerError, "internalError"
	switch st.Code() {
	case codes.OK:
		return false
	case codes.NotFound:
		httpStatus, reason = http.StatusNotFound, "notFound"
	case codes.AlreadyExists:
		httpStatus, reason = http.StatusConflict, "duplicate"
	case codes.InvalidArgument:
		httpStatus, reason = http.StatusBadRequest, "invalid"
	case codes.FailedPrecondition:
		httpStatus, reason = http.StatusBadRequest, "failedPrecondition"
	case codes.PermissionDenied:
		httpStatus, reason = http.StatusForbidden, "accessDenied"
	case codes.Unauthenticated:
		// The emulator never authenticates so this is unlikely, but
		// map it so a buggy engine doesn't crash through to 500.
		httpStatus, reason = http.StatusUnauthorized, "authError"
	case codes.Unimplemented:
		httpStatus, reason = http.StatusNotImplemented, "notImplemented"
	case codes.Unavailable:
		httpStatus, reason = http.StatusServiceUnavailable, "backendError"
	case codes.DeadlineExceeded:
		httpStatus, reason = http.StatusGatewayTimeout, "backendError"
	case codes.ResourceExhausted:
		httpStatus, reason = http.StatusTooManyRequests, "quotaExceeded"
	}
	writeError(w, httpStatus, reason, st.Message())
	return true
}
