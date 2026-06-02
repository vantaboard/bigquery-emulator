---
name: bqconnection gRPC service
overview: "Register the `bigquery.connection.v1` gRPC service per the existing intake table in `gateway/handlers/bqconnection/server.go`. Mechanical port: the data model + go-googlesql intake table are already in place; this plan registers the service, wires the handlers, and ships the protobuf dependencies."
todos:
  - id: tp16_audit
    content: "Audit gateway/handlers/bqconnection/server.go:22-48 — confirm the intake table covers every RPC the Java client exercises (Create, Get, List, Update, Delete, plus Share / CreateAws variants)."
    status: pending
  - id: tp16_proto_deps
    content: "Add the required Go module deps: connectionpb (google.cloud.bigquery.connection.v1) + iampb (google.iam.v1). Vendor protos if not present."
    status: pending
  - id: tp16_server
    content: "Implement ConnectionServiceServer with each RPC delegating to the intake-table-backed logic. Mirror the storage_write registration in Server::Create."
    status: pending
  - id: tp16_iam
    content: "IAM RPCs (GetIamPolicy / SetIamPolicy / TestIamPermissions): minimal pass-through implementation since the emulator has no auth model — return a documented empty policy."
    status: pending
  - id: tp16_tests
    content: "Unit-test each RPC; rerun `task thirdparty:java-bigquery-tests` for the five connection ITs (ShareConnectionIT, GetConnectionIT, DeleteConnectionIT, CreateAwsConnectionIT, UpdateConnectionIT)."
    status: pending
  - id: tp16_index
    content: "Flip thirdparty-00-completion-index.plan.md todo `tp16` to `completed`."
    status: pending
isProject: false
---

# Thirdparty 16 — bqconnection gRPC service

## Source

- `fix_thirdparty_failures_cbe91a41.plan.md` Tier 4.3 (now deleted).
- Failing Java ITs: `ShareConnectionIT`, `GetConnectionIT`,
  `DeleteConnectionIT`, `CreateAwsConnectionIT`, `UpdateConnectionIT`
  (all UNIMPLEMENTED).
- Skeleton + go-googlesql intake table already in
  [`gateway/handlers/bqconnection/server.go:22-48`](../../gateway/handlers/bqconnection/server.go).
- Dependency note from the original Tier 4.3: `connectionpb` +
  `iampb` Go modules.

## Prerequisites

None hard. The intake table already exists; this plan is the wiring
+ dependency landing.

## Scope

Register `google.cloud.bigquery.connection.v1.ConnectionService` on
the gRPC frontend. Implement every RPC the Java client uses:

- `CreateConnection`
- `GetConnection`
- `ListConnections`
- `UpdateConnection`
- `DeleteConnection`
- `GetIamPolicy` / `SetIamPolicy` / `TestIamPermissions`
- (AWS-flavor: `CreateConnection` with an `aws` config block — same
  RPC, different config payload)

Out of scope:

- Actually validating AWS / Azure / Cloud SQL connection credentials.
  The emulator stores the spec verbatim and returns it; it does not
  authenticate against external systems. Document this in the
  handler.
- Routing query-time `EXTERNAL` table references through these
  connection specs. That is a separate plan.

## Implementation

### Proto deps

Add `connectionpb` (`google.cloud.bigquery.connection.v1`) and
`iampb` (`google.iam.v1`) to `go.mod`. If proto generation is
checked-in elsewhere, vendor the generated files; otherwise wire
the existing proto generation step (mirror what the storage_write
plan did).

### Server implementation

Implement `ConnectionServiceServer` next to the existing skeleton:

- Each non-IAM RPC delegates to the intake-table-backed logic from
  the existing server.go.
- IAM RPCs return a minimal documented empty policy (the emulator
  has no auth model; do not silently fabricate one).

### Server registration

Register on the gRPC frontend. Mirror the storage_write +
datatransfer registrations.

### Error envelopes

Map internal errors to gRPC status codes per BigQuery convention
(404 -> NOT_FOUND, 409 -> ALREADY_EXISTS, etc.).

## Tests

- Unit tests under
  `gateway/handlers/bqconnection/server_test.go` for each RPC.
- `task thirdparty:java-bigquery-tests` — the five connection ITs
  go green.
- No regression on the rest of the Java IT suite.

## Done criteria

- ConnectionServiceServer registered + wired.
- Five connection ITs pass.
- IAM RPCs return a documented stub policy.
- `thirdparty-00-completion-index.plan.md` todo `tp16` flipped to
  `completed`.
