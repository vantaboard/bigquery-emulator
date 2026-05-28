---
name: java-its-shallow-emulators
overview: "Java live-IT track Phase B: manually port go-googlesql's `api/{bqconnection,bqstorage,datatransfer}` packages into this repo's `gateway/`, adapt to this repo's catalog/storage types, wire into the gateway server, and turn Phase A's 9 expected-fail ITs green."
todos:
  - id: surface-mapping-and-package-skeletons
    content: "Produce a per-IT RPC matrix mapping each Phase A failing IT to the gRPC/REST surface it hits; scaffold `gateway/handlers/{bqconnection,bqstorage,datatransfer}/` Go packages with `Register` entry points and adapter stubs against this repo's catalog types."
    status: pending
  - id: port-and-wire
    content: "Port go-googlesql logic file-by-file: `api/bqconnection/{server,rest_handler,connection_mask_paths,connection_properties,connection_update}.go`; minimum `api/bqstorage/write*.go` slice that satisfies `WriteBufferedStreamIT`; `api/datatransfer/{handler,handler_runs,handler_scheduled_query,handlers_project_scoped,catalog,paging}.go`. Replace the existing partial `gateway/handlers/data_transfer.go` shell. Wire each into `gateway/server.go` (gRPC ports for connection/storage, HTTP mux for datatransfer)."
    status: pending
  - id: port-tests-and-validate
    content: "Port go-googlesql `*_test.go` (adapted), add `gateway/e2e` coverage for new surfaces, re-run Phase A's `task thirdparty:java-bigquery-tests`, and capture a 15/15-green per-IT verdict table into Done criteria."
    status: pending
isProject: false
---

# Phase B: Shallow Emulator Port (Connection / Storage / DataTransfer)

## Prerequisites

- [java-its-task-conversion_a7b8c9d0.plan.md](java-its-task-conversion_a7b8c9d0.plan.md) — Phase A must have landed; its per-IT pass/fail baseline drives Phase B's surface-mapping table.

## Scope

This phase brings the bigquery-emulator's REST/gRPC surface up to the minimum needed to satisfy the 9 Phase A ITs that currently fail (5 `bigqueryconnection` + 3 `bigquerydatatransfer` + 1 `bigquerystorage`). The implementation is a **manual port** from go-googlesql's `/home/brighten-tompkins/Code/go-googlesql/api/{bqconnection,bqstorage,datatransfer}/` packages, adapted to this repo's `backend/catalog/` and `backend/storage/` types. No go-googlesql Go-module dependency is added; no `third_party/go-googlesql/` subtree is created.

Out of scope: the full go-googlesql `bqstorage` Read path (~30 files), `bqanalyticshub`, `bqreservation`, `datapolicy`, etc. — only the slice that Phase A's failing ITs exercise.

## Work items

### 1. `surface-mapping-and-package-skeletons`

Build a single canonical mapping table (lives in this plan's `## Surface map` section after completion) of every failing Phase A IT → the exact gRPC method or REST path it invokes → the go-googlesql source file that implements it today.

Anticipated rows:

- `CreateAwsConnectionIT` → `connectionpb.ConnectionService.CreateConnection` → [`api/bqconnection/server.go`](../../../go-googlesql/api/bqconnection/server.go) + [`rest_handler.go`](../../../go-googlesql/api/bqconnection/rest_handler.go).
- `DeleteConnectionIT` → `ConnectionService.DeleteConnection`.
- `GetConnectionIT` → `ConnectionService.GetConnection`.
- `ShareConnectionIT` → `ConnectionService.{GetIamPolicy,SetIamPolicy}`.
- `UpdateConnectionIT` → `ConnectionService.UpdateConnection` (drives `connection_mask_paths.go` + `connection_update.go`).
- `CreateAmazonS3TransferIT` → `POST /v1/projects/{p}/locations/{l}/transferConfigs` (datatransfer REST).
- `DisableTransferConfigIT` → `PATCH /v1/.../transferConfigs/{id}` (disabled=true).
- `ReEnableTransferConfigIT` → `PATCH /v1/.../transferConfigs/{id}` (disabled=false).
- `WriteBufferedStreamIT` → `BigQueryWrite.{CreateWriteStream, AppendRows, FinalizeWriteStream, BatchCommitWriteStreams}` → [`api/bqstorage/write*.go`](../../../go-googlesql/api/bqstorage/) (subset: `write.go`, `write_streams.go`, `write_append.go`, `write_quota.go`, `proto_descriptor_normalize.go`, `proto_rows*.go`).

Then scaffold three new Go packages under [gateway/handlers/](../../gateway/handlers/) (or sibling locations if `gateway/server.go` wiring prefers that):

- `gateway/handlers/bqconnection/` — `server.go` with `Server` struct + `Register(grpcServer *grpc.Server, srv *Server)`; stub methods returning `codes.Unimplemented`.
- `gateway/handlers/bqstorage/` — same shape for the BigQueryWrite slice; `Register` accepts the gRPC server registered on port 9060 per [proto/storage_read.proto](../../proto/storage_read.proto).
- `gateway/handlers/datatransfer/` — REST handlers + in-memory `transferConfigs` / `transferRuns` maps; replace the existing partial [gateway/handlers/data_transfer.go](../../gateway/handlers/data_transfer.go) shell (which returns empty/501 today).

Adapter shims: go-googlesql's `storage.EmulatorCatalog` and `apiregion.{CheckGRPC,CheckHTTP}` need this-repo equivalents. Read [backend/catalog/](../../backend/catalog/) + [gateway/middleware/](../../gateway/middleware/) and document the type-mapping in this plan before porting any logic.

### 2. `port-and-wire`

Port the go-googlesql logic file-by-file into the new packages. Strategy:

1. **bqconnection** (smaller, do first):
   - Port [`server.go`](../../../go-googlesql/api/bqconnection/server.go), [`rest_handler.go`](../../../go-googlesql/api/bqconnection/rest_handler.go), [`connection_mask_paths.go`](../../../go-googlesql/api/bqconnection/connection_mask_paths.go), [`connection_properties.go`](../../../go-googlesql/api/bqconnection/connection_properties.go), [`connection_update.go`](../../../go-googlesql/api/bqconnection/connection_update.go).
   - Adapt all `storage.EmulatorCatalog` references to this repo's catalog.
   - Persist connections in this repo's storage layer; if the existing storage doesn't model connections, add a minimal in-memory map keyed by `projects/{p}/locations/{l}/connections/{id}`.

2. **datatransfer** (largest surface, smallest required slice):
   - Port [`handler.go`](../../../go-googlesql/api/datatransfer/handler.go), [`handler_runs.go`](../../../go-googlesql/api/datatransfer/handler_runs.go), [`handler_scheduled_query.go`](../../../go-googlesql/api/datatransfer/handler_scheduled_query.go), [`handlers_project_scoped.go`](../../../go-googlesql/api/datatransfer/handlers_project_scoped.go), [`catalog.go`](../../../go-googlesql/api/datatransfer/catalog.go), [`paging.go`](../../../go-googlesql/api/datatransfer/paging.go).
   - For Phase B, only `amazon_s3` data source is required in the catalog (drives `CreateAmazonS3TransferIT`); `scheduled_query` may be ported but its `ScheduledQueryRunner` wiring can stay nil (the Phase A IT doesn't exercise it). Phase C extends the catalog.
   - Delete the existing `gateway/handlers/data_transfer.go` shell; its three stub functions (`DataTransferDataSourceList`, etc.) are superseded.

3. **bqstorage** (port only the minimum write slice):
   - Port [`write.go`](../../../go-googlesql/api/bqstorage/write.go), [`write_streams.go`](../../../go-googlesql/api/bqstorage/write_streams.go), [`write_append.go`](../../../go-googlesql/api/bqstorage/write_append.go), [`write_quota.go`](../../../go-googlesql/api/bqstorage/write_quota.go), [`proto_descriptor_normalize.go`](../../../go-googlesql/api/bqstorage/proto_descriptor_normalize.go), [`proto_rows.go`](../../../go-googlesql/api/bqstorage/proto_rows.go), [`proto_rows_normalize_test.go` helpers](../../../go-googlesql/api/bqstorage/proto_rows_normalize_test.go), [`proto_rows_coercion.go`](../../../go-googlesql/api/bqstorage/proto_rows_coercion.go).
   - Skip Read-path files (`read*.go`, `avro_arrow.go`, etc.) — not needed for Phase A's `WriteBufferedStreamIT`.
   - Hook AppendRows into this repo's storage backend; if direct row-insertion isn't exposed, route through the existing tabledata insertAll path.

Wiring in [gateway/server.go](../../gateway/server.go):

- `bqconnection.Register(grpcServer, &bqconnection.Server{Cat: catalog})` on the existing gRPC server.
- `bqstorage.Register(storageGrpcServer, &bqstorage.WriteServer{Cat: catalog})` on the BigQuery Storage gRPC server (port 9060).
- Mount `datatransfer.Handler` routes into the existing HTTP mux; remove the old `gateway/handlers/data_transfer.go` registration.

### 3. `port-tests-and-validate`

- Port the go-googlesql test files (`*_test.go`) for each ported source, adapting to this repo's `gateway_test` patterns. At minimum: `bqconnection/server_test.go`, `bqconnection/rest_handler_test.go`, `datatransfer/handler_test.go`, `bqstorage/write_test.go`, `bqstorage/write_quota_test.go`.
- Add e2e tests under [gateway/e2e/](../../gateway/e2e/) (mirror the shape of [gateway/e2e/storage_read_test.go](../../gateway/e2e/storage_read_test.go)) covering at least one happy path per new surface.
- Re-run Phase A's `task thirdparty:java-bigquery-tests`. Capture the 15-IT verdict table into Done criteria; expected state is 15/15 PASS.
- Extend [docs/REST_API.md](../../docs/REST_API.md) with the three new surfaces (one section each, mention which surface is gRPC vs REST and which port serves it).

## Verification

```bash
go test ./gateway/handlers/bqconnection/... ./gateway/handlers/bqstorage/... ./gateway/handlers/datatransfer/...
go test ./gateway/e2e/... -run 'Connection|Storage|DataTransfer'
task emulator:run-full
task thirdparty:java-bigquery-tests
```

## Done criteria

- Three new packages live under `gateway/handlers/` with passing unit tests ported from go-googlesql.
- `gateway/server.go` registers all three.
- `gateway/handlers/data_transfer.go` shell is removed and its routes are served by the new `datatransfer` package.
- `task thirdparty:java-bigquery-tests` reports 15/15 PASS (verdict table captured here, replacing Phase A's 6+9 baseline).
- No regression in `gateway/e2e/storage_read_test.go` or any other pre-existing gateway test.

## Next plan(s)

- [java-its-missing-tests_c9d0e1f2.plan.md](java-its-missing-tests_c9d0e1f2.plan.md)
