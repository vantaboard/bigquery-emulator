package handlers

import (
	"net/http"

	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

// queryGRPCToHTTPError translates a gRPC error returned by the engine's
// Query service into a BigQuery-shaped JSON error envelope and writes
// it to w. Returns true when err was non-nil (and therefore an error
// was written), so callers can use it as `if queryGRPCToHTTPError(...)
// { return }`.
//
// The mapping mirrors grpcToHTTPError but uses query-specific REST
// reason codes the BigQuery client libraries recognize:
//
//   - INVALID_ARGUMENT → 400 invalidQuery (parse / analysis errors,
//     unknown table or column references, type mismatches; see
//     `frontend/handlers/query.cc::AnalyzeStatusToGrpc` and
//     docs/REST_API.md "SQL dialect" for why analysis errors must
//     carry `reason: invalidQuery` rather than the generic `invalid`).
//   - NOT_FOUND → 404 notFound (a referenced table or dataset is
//     missing; the engine usually wraps these as INVALID_ARGUMENT
//     because GoogleSQL surfaces them through the analyzer, but
//     storage-side NOT_FOUNDs from `DescribeTable` need their own
//     mapping to keep parity with `tables.get`).
//   - ALREADY_EXISTS → 409 duplicate (DDL/control-op conflicts such as
//     UNDROP SCHEMA after recreating the same dataset id).
//   - FAILED_PRECONDITION → 400 invalidQuery (the engine raises this
//     when the catalog has not been initialized; the gateway folds it
//     into the same 400 reason a client sees when the SQL itself is
//     invalid because there is nothing actionable beyond "the
//     emulator is not ready" and the BigQuery REST envelope has no
//     dedicated code for that).
//   - UNIMPLEMENTED → 501 notImplemented (the gateway is talking to a
//     legacy engine build with `--googlesql=off`).
//   - UNAVAILABLE / DEADLINE_EXCEEDED → 503 backendError /
//     504 backendError; same as `grpcToHTTPError`.
//
// Anything else (INTERNAL, plain Go errors) is reported as 500
// internalError so a misbehaving engine cannot be mistaken for a
// recoverable client-side issue.
func queryGRPCToHTTPError(w http.ResponseWriter, err error) bool {
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
	case codes.InvalidArgument, codes.FailedPrecondition:
		httpStatus, reason = http.StatusBadRequest, reasonInvalidQuery
	case codes.NotFound:
		httpStatus, reason = http.StatusNotFound, reasonNotFound
	case codes.AlreadyExists:
		httpStatus, reason = http.StatusConflict, reasonDuplicate
	case codes.PermissionDenied:
		httpStatus, reason = http.StatusForbidden, reasonAccessDenied
	case codes.Unauthenticated:
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
