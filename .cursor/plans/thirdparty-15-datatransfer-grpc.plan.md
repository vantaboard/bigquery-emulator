---
name: datatransfer gRPC adapter
overview: "Expose the existing REST datatransfer handler via a gRPC adapter so the Java DataTransferServiceClient (which uses gRPC by default) stops hitting UNIMPLEMENTED. Adapter approach: wrap the existing handler rather than reimplement against the in-memory store."
todos:
  - id: tp15_audit
    content: "Audit gateway/handlers/datatransfer/handler.go:143-165 — confirm every REST method has a matching gRPC RPC in DataTransferServiceServer."
    status: pending
  - id: tp15_adapter
    content: "Add gateway/handlers/datatransfer/grpc_adapter.go implementing DataTransferServiceServer; each method translates the gRPC request shape into the REST handler's internal request struct, calls the existing handler logic, and translates the response back."
    status: pending
  - id: tp15_proto
    content: "Vendor / wire google/cloud/bigquery/datatransfer/v1 protos if not already present. Confirm Go module pulls them at build time."
    status: pending
  - id: tp15_server_register
    content: "Register the new server on the gRPC frontend (mirror the storage_write registration in Server::Create)."
    status: pending
  - id: tp15_tests
    content: "Unit-test each adapter method (request/response shape translation); rerun task thirdparty:java-bigquery-tests for the 11 DTS ITs."
    status: pending
  - id: tp15_index
    content: "Flip thirdparty-00-completion-index.plan.md todo `tp15` to `completed`."
    status: pending
isProject: false
---

# Thirdparty 15 — datatransfer gRPC adapter

## Source

- `fix_thirdparty_failures_cbe91a41.plan.md` Tier 4.4 (now deleted).
- Affects 11 Java DTS ITs (UNIMPLEMENTED).
- REST surface already mounted at
  [`gateway/handlers/datatransfer/handler.go:143-165`](../../gateway/handlers/datatransfer/handler.go);
  the failures exist because the Java DTS client uses gRPC by
  default.

## Prerequisites

None hard. Easiest of the three gRPC port plans because the data
model and CRUD logic already exist on the REST side.

## Scope

Wrap the existing REST handler with a gRPC adapter, not a
re-implementation. The data model in the existing handler is
authoritative; this plan adds a single new file plus server
registration.

Why the adapter approach beats the rewrite:

- The REST surface is already correct (verified by the existing
  REST-based fixtures).
- The gRPC method surface is a near-perfect superset of the REST
  one — every gRPC RPC corresponds to one REST method.
- Two surfaces driven by one code path means schema changes only
  need to be made once.

The cost is one layer of request/response translation per RPC,
which is mechanical.

## Implementation

### Adapter file

`gateway/handlers/datatransfer/grpc_adapter.go` implements
`google.cloud.bigquery.datatransfer.v1.DataTransferServiceServer`.
Each method:

1. Decodes the gRPC request into the internal request struct the
   REST handler already uses.
2. Calls the existing handler logic.
3. Encodes the internal response back into the gRPC response
   shape.

Error mapping: convert internal errors to gRPC status codes per
[google.rpc.Code](https://grpc.io/grpc/core/md_doc_statuscodes.html)
+ bigquery convention.

### Proto wiring

Vendor `google/cloud/bigquery/datatransfer/v1` if not already
present in the Go module graph. Update `go.mod` accordingly. If a
buf / protoc generation step exists for other handlers, mirror it.

### Server registration

Mirror what
[`storage-read-write-api-plan.plan.md`](./storage-read-write-api-plan.plan.md)
already does for `BigQueryWriteServer` registration in
`Server::Create` (or the equivalent Go gRPC server setup). Add the
new service alongside.

### REST surface

Leave the REST handler in place. It still gets exercised by the
node + python clients. Both surfaces share the same backing store.

## Tests

- Unit tests under
  `gateway/handlers/datatransfer/grpc_adapter_test.go` for each
  RPC (happy path + error envelopes).
- `task thirdparty:java-bigquery-tests` — all 11 DTS ITs go
  green.

## Done criteria

- The adapter file lands.
- DataTransferServiceServer is registered on the gRPC frontend.
- All 11 Java DTS ITs pass.
- `thirdparty-00-completion-index.plan.md` todo `tp15` flipped to
  `completed`.
