// Package bqconnection is the shallow-emulator skeleton for the
// BigQuery Connection API surface (gRPC, exposed at the storage gRPC
// port per docker-compose.yml). The gRPC layer is intentionally NOT
// registered in this skeleton because doing so would require:
//
//  1. Adding `cloud.google.com/go/bigquery/connection/apiv1/connectionpb`
//     and the associated `cloud.google.com/go/iam/apiv1/iampb` Go
//     dependencies, which transitively pull ~30 packages this repo
//     does not currently link.
//  2. Building a connection-record storage layer (this repo's
//     `backend/catalog/` is C++ and does not yet model connection
//     records).
//
// Both are explicitly larger than the shallow-emulator port budget
// per `docs/ENGINE_POLICY.md`.
// The surface-mapping table below documents which failing-IT each
// intended handler symbol satisfies, so follow-up ports use a
// one-to-one mapping rather than a free-form rebuild.
//
// Failing-IT → intended handler mapping (shallow-emulator intake table):
//
//	CreateAwsConnectionIT  → connectionpb.ConnectionService.CreateConnection
//	                          ⇒ gateway/handlers/bqconnection/server.go: CreateConnection
//	                          ⇒ gateway/handlers/bqconnection/rest_handler.go (HTTP/JSON variant)
//	                          ⇒ gateway/handlers/bqconnection/connection_properties.go: applyCloudSQLFromCreate,
//	                             validateConnectionPropertiesOneof
//
//	DeleteConnectionIT     → connectionpb.ConnectionService.DeleteConnection
//	                          ⇒ gateway/handlers/bqconnection/server.go: DeleteConnection
//	GetConnectionIT        → connectionpb.ConnectionService.GetConnection
//	                          ⇒ gateway/handlers/bqconnection/server.go: GetConnection
//	ShareConnectionIT      → connectionpb.ConnectionService.{GetIamPolicy,SetIamPolicy}
//	                          ⇒ gateway/handlers/bqconnection/server.go: {GetIamPolicy,SetIamPolicy}
//	                            (currently UNIMPLEMENTED — IT will fail-fast)
//	UpdateConnectionIT     → connectionpb.ConnectionService.UpdateConnection
//	                          ⇒ gateway/handlers/bqconnection/server.go: UpdateConnection
//	                          ⇒ gateway/handlers/bqconnection/connection_mask_paths.go: applyConnectionUpdateMask
//	                          ⇒ gateway/handlers/bqconnection/connection_update.go: per-field setters
//
// Storage adapter shim (deferred): connection-record helpers
// (GetConnectionRecord, PutConnectionRecord, ListConnectionRecords,
// DeleteConnectionRecord, IsNotFound) map onto
// this repo's `backend/storage/` once a connections table lands. The
// initial cut should keep them in-process (a `sync.Map`-backed store
// is fine for the live-IT track) and add a SQLite-backed
// implementation only when persistence becomes necessary.
package bqconnection

import (
	"net/http"
)

// Register is the symbolic entry point the gateway will call once the
// gRPC surface lands. Until then the gateway routes the few REST
// shapes the Java client falls back to (POST /v1beta1/projects/...
// and equivalent gapic-rest paths) to NotImplementedHTTP below.
func Register(_ *http.ServeMux) {}

// NotImplementedHTTP returns a structured 501 for any Connection API
// REST probe the gateway might add ahead of the full gRPC port. The
// existing gateway/handlers.NotImplemented helper would do; this
// indirection keeps the package self-contained.
func NotImplementedHTTP(w http.ResponseWriter, _ *http.Request) {
	const body = `{"error":{"code":501,"message":"BigQuery Connection API is not yet implemented by the emulator. See docs/ENGINE_POLICY.md and ROADMAP.md.","status":"notImplemented","errors":[{"reason":"notImplemented","message":"BigQuery Connection API is not yet implemented by the emulator.","domain":"global"}]}}`
	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	w.WriteHeader(http.StatusNotImplemented)
	_, _ = w.Write([]byte(body))
}
