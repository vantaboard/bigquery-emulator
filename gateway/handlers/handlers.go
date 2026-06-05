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
	"regexp"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"github.com/vantaboard/bigquery-emulator/gateway/routines"
	"github.com/vantaboard/bigquery-emulator/gateway/snapshots"
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

	// Metadata caches REST-only Dataset/Table fields the engine
	// does not yet persist (labels, defaultCollation, expirationTime,
	// rangePartitioning, clustering, ...). Insert/Patch/Update
	// populate it; Get reads it back and merges with the engine
	// response. Nil is treated as a no-op store so legacy unit
	// tests that do not opt in keep their echo posture.
	Metadata *MetadataStore

	// Snapshots retains deleted-table row captures for COPY jobs that
	// reference table@epoch decorators (undelete samples). Nil is a
	// no-op store.
	Snapshots *snapshots.Store

	// Routines is the in-memory UDF / TVF / procedure registry REST
	// handlers use for routines.* and DDL query jobs register into.
	// Nil is treated as a per-handler fallback store.
	Routines *routines.Store
}

// NewRoutineStore returns an empty routine registry for gateway deps.
func NewRoutineStore() *routines.Store {
	return routines.NewStore()
}

// NewSnapshotStore returns an empty table snapshot store for gateway deps.
func NewSnapshotStore() *snapshots.Store {
	return snapshots.NewStore()
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
	writeError(w, http.StatusNotImplemented, reasonNotImplemented,
		"This BigQuery emulator route is registered but not yet implemented. "+
			"See ROADMAP.md.")
}

// NotFound is the catch-all handler for paths not in the route table. It
// returns a BigQuery-shaped 404 so client libraries see a structured error.
func NotFound(w http.ResponseWriter, r *http.Request) {
	writeError(w, http.StatusNotFound, reasonNotFound,
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
	for i := range len(segment) {
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
//
// The error message itself is rewritten into BigQuery's canonical
// shape via bqStyleMessage so client-side assertions like
// `expect(err.message).to.include('Not found')` and
// `expect(err.message).to.include('Already Exists')` match the live
// surface.
func grpcToHTTPError(w http.ResponseWriter, err error) bool {
	if err == nil {
		return false
	}
	st, ok := status.FromError(err)
	if !ok {
		writeError(w, http.StatusInternalServerError, reasonInternalError,
			"Engine RPC failed: "+err.Error())
		return true
	}
	httpStatus, reason := http.StatusInternalServerError, reasonInternalError
	switch st.Code() {
	case codes.OK:
		return false
	case codes.NotFound:
		httpStatus, reason = http.StatusNotFound, reasonNotFound
	case codes.AlreadyExists:
		httpStatus, reason = http.StatusConflict, reasonDuplicate
	case codes.InvalidArgument:
		httpStatus, reason = http.StatusBadRequest, reasonInvalid
	case codes.FailedPrecondition:
		httpStatus, reason = http.StatusBadRequest, reasonFailedPrecondition
	case codes.PermissionDenied:
		httpStatus, reason = http.StatusForbidden, reasonAccessDenied
	case codes.Unauthenticated:
		// The emulator never authenticates so this is unlikely, but
		// map it so a buggy engine doesn't crash through to 500.
		httpStatus, reason = http.StatusUnauthorized, reasonAuthError
	case codes.Unimplemented:
		httpStatus, reason = http.StatusNotImplemented, reasonNotImplemented
	case codes.Unavailable:
		httpStatus, reason = http.StatusServiceUnavailable, reasonBackendError
	case codes.DeadlineExceeded:
		httpStatus, reason = http.StatusGatewayTimeout, reasonBackendError
	case codes.ResourceExhausted:
		httpStatus, reason = http.StatusTooManyRequests, reasonQuotaExceeded
	}
	writeError(w, httpStatus, reason, bqStyleMessage(st.Message()))
	return true
}

// notFoundResourceRE / alreadyExistsResourceRE match the engine's
// canonical storage-layer error strings produced by DuckDBStorage
// (see backend/storage/duckdb/duckdb_storage.cc): "<noun> not found:
// <project>.<dataset>[.<table>]" and "<noun> already exists:
// <project>.<dataset>[.<table>]" where <noun> is "table" or
// "dataset". The resource path uses `.` between every segment on the
// engine side; BigQuery REST uses `:` between project and dataset and
// `.` between dataset and table. The captured suffix is rewritten to
// the REST shape and the noun is capitalised so client assertions for
// "Not found" / "Already Exists" prefixes (live BigQuery's canonical
// shape) match.
var (
	notFoundResourceRE = regexp.MustCompile(
		`^(table|dataset) not found: ([^.]+)\.([^.]+)(?:\.([^.]+))?$`)
	alreadyExistsResourceRE = regexp.MustCompile(
		`^(table|dataset) already exists: ([^.]+)\.([^.]+)(?:\.([^.]+))?$`)
)

// bqStyleMessage rewrites the small set of engine-side storage errors
// the gateway forwards into BigQuery's canonical wire shape. Examples:
//
//	"table not found: dev.foo.bar"      -> "Not found: Table dev:foo.bar"
//	"dataset not found: dev.foo"        -> "Not found: Dataset dev:foo"
//	"table already exists: dev.foo.bar" -> "Already Exists: Table dev:foo.bar"
//	"dataset already exists: dev.foo"   -> "Already Exists: Dataset dev:foo"
//
// Any message that does not match a known pattern passes through
// verbatim so non-storage errors (analysis failures, etc.) keep their
// engine-side wording. The regexes anchor on `^...$` to avoid matching
// embedded substrings and accept only the two storage nouns the engine
// emits today; future additions go here as the catalog grows.
func bqStyleMessage(msg string) string {
	if m := notFoundResourceRE.FindStringSubmatch(msg); m != nil {
		return bqStyleResourceMessage("Not found", m[1], m[2], m[3], m[4])
	}
	if m := alreadyExistsResourceRE.FindStringSubmatch(msg); m != nil {
		return bqStyleResourceMessage("Already Exists", m[1], m[2], m[3], m[4])
	}
	return msg
}

// bqStyleResourceMessage assembles "<verb>: <Noun> <project>:<dataset>[.<table>]".
// `table` is empty when the engine matched the dataset variant.
func bqStyleResourceMessage(verb, noun, project, dataset, table string) string {
	resource := project + ":" + dataset
	if table != "" {
		resource += "." + table
	}
	switch noun {
	case "table":
		return verb + ": Table " + resource
	case "dataset":
		return verb + ": Dataset " + resource
	default:
		// Unreachable given the regex character class, but keep a
		// defensive fall-through so a future regex tweak that adds a
		// new noun without a switch arm cannot silently lose the
		// rewrite.
		return verb + ": " + noun + " " + resource
	}
}
