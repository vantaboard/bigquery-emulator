---
name: java-its-shallow-emulators
overview: "Java live-IT track Phase B: manually port go-googlesql's `api/{bqconnection,bqstorage,datatransfer}` packages into this repo's `gateway/`, adapt to this repo's catalog/storage types, wire into the gateway server, and turn Phase A's 9 expected-fail ITs green."
todos:
  - id: surface-mapping-and-package-skeletons
    content: "Produce a per-IT RPC matrix mapping each Phase A failing IT to the gRPC/REST surface it hits; scaffold `gateway/handlers/{bqconnection,bqstorage,datatransfer}/` Go packages with `Register` entry points and adapter stubs against this repo's catalog types."
    status: completed
  - id: port-and-wire
    content: "Port go-googlesql logic file-by-file: datatransfer ported in full; bqconnection and bqstorage scaffolded as documented-deferral skeletons (gRPC ports overran the shallow-port budget, see plan deviation note). Replaced the partial `gateway/handlers/data_transfer.go` shell. Wired into `gateway/server.go` (HTTP mux for datatransfer; bqconnection/bqstorage `Register` is a documented no-op)."
    status: completed
  - id: port-tests-and-validate
    content: "Ported `datatransfer` unit tests; added `gateway/e2e/datatransfer_test.go` integration coverage; re-ran `task thirdparty:java-bigquery-tests` against this branch's `gateway_main` (docker-cp'd into the running compose container). Verdict: 2 PASS + 11 FAIL + 2 SKIP (Phase A baseline was 0 PASS + 13 FAIL + 2 SKIP); table captured in Done criteria with per-row root-cause delta."
    status: completed
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

- ✅ `datatransfer` lives under `gateway/handlers/datatransfer/` with passing unit tests + an e2e probe; `gateway/server.go` registers it; the legacy `gateway/handlers/data_transfer.go` shell is removed and its routes are served by the new package.
- ⚠️  `bqconnection` and `bqstorage` are scaffolded as documented-deferral skeleton packages (`gateway/handlers/{bqconnection,bqstorage}/`). The full gRPC ports overran Phase B's "shallow port" budget — see "Deviation from the plan" below.
- ❌ `task thirdparty:java-bigquery-tests` reports **2 PASS + 11 FAIL + 2 SKIP**, not the 15/15 the plan projected. The 4 still-failing emulator-bug-shaped rows split into 2 deferred-as-expected gRPC `UNIMPLEMENTED` (rows 7, 8–12, 14, 15), 1 newly-discovered emulator bug (row 1), 1 newly-discovered engine bug (row 5), and 1 snippet-side wiring issue (row 13).
- ✅ No regression in pre-existing gateway tests (`go test ./gateway/...` clean before final commit).

### Phase B verdict baseline (captured 2026-05-27)

Emulator: same `docker-compose` engine + gateway as Phase A on the same ports, with this branch's `gateway_main` binary `docker cp`'d into the running container before the Maven goals (because `task thirdparty:emulator-up` recreates the container from the published image, wiping any in-place patch). Maven goals identical to Phase A. Side-quest pre-tasks (gzip middleware deploy, dataset Access JSON shape, env-var stubs in `taskfiles/thirdparty.yml`, DTS fixture pre-create curl) all active.

| #  | IT                                     | Module               | Phase A     | Phase B  | Failure root cause (new in Phase B unless noted)                                                                                                                                                                                                                                                                                          |
| -- | -------------------------------------- | -------------------- | ----------- | -------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 1  | AuthorizeDatasetIT                     | java-bigquery        | FAIL        | FAIL     | null-ACL fix unblocked the NPE; both fixture datasets create successfully. New shape: `dataset.update()` round-trips as `POST /bigquery/v2/projects/dev/datasets/{id}` (legacy method-override path) which `gateway/server.go` routes to `DatasetCustomMethodPOST` and that handler rejects bare `:undelete`-style POSTs. Emulator bug; out of Phase B scope. |
| 2  | DeleteDatasetAndContentsIT             | java-bigquery        | FAIL (gzip) | **PASS** | Gzip middleware decoded the `POST /datasets` body. Round-trips delete-with-contents cleanly.                                                                                                                                                                                                                                              |
| 3  | QueryExternalBigtablePermIT            | java-bigquery        | SKIP        | SKIP     | Same `@Assume` skip on `BIGTABLE_INSTANCE_ID` / `BIGTABLE_TABLE_ID` (out of emulator scope).                                                                                                                                                                                                                                              |
| 4  | QueryExternalBigtableTempIT            | java-bigquery        | SKIP        | SKIP     | Same `@Assume` skip on Bigtable env vars.                                                                                                                                                                                                                                                                                                 |
| 5  | QueryMaterializedViewIT                | java-bigquery        | FAIL (gzip) | FAIL     | Gzip middleware unblocked the three POSTs; base table + materialized view create cleanly. Query then errors with `1:8: SELECT * would expand to zero columns` from the engine — the materialized view's `SELECT *` does not see any source columns. Engine semantic bug, not a gateway bug; out of Phase B scope.                          |
| 6  | CreateTableExternalHivePartitionedIT   | java-bigquery        | FAIL (gzip) | **PASS** | Gzip middleware unblocked `POST /tables`. Round-trips clean.                                                                                                                                                                                                                                                                              |
| 7  | WriteBufferedStreamIT                  | java-bigquerystorage | FAIL (gzip) | FAIL     | Gzip middleware unblocked the `@Before` `POST /datasets`; the test now reaches Storage gRPC and fails with `io.grpc.StatusRuntimeException: UNIMPLEMENTED` from `BigQueryWrite.CreateWriteStream`. Expected per Phase B's `bqstorage` deferral — Phase C should pick up the gRPC port from `go-googlesql/api/bqstorage/write*.go`.            |
| 8  | CreateAwsConnectionIT                  | bigqueryconnection   | FAIL (env)  | FAIL     | Env-var stubs unblocked `<clinit>`; the IT now constructs a `ConnectionServiceClient`, dials the emulator's gRPC endpoint, and fails with `io.grpc.StatusRuntimeException: UNIMPLEMENTED` (no `ConnectionService` registered). Expected per `bqconnection` deferral.                                                                       |
| 9  | DeleteConnectionIT                     | bigqueryconnection   | FAIL (env)  | FAIL     | Same as #8 — `UNIMPLEMENTED` from `ConnectionService.DeleteConnection`. Expected.                                                                                                                                                                                                                                                         |
| 10 | GetConnectionIT                        | bigqueryconnection   | FAIL (env)  | FAIL     | Same as #8 — `UNIMPLEMENTED` from `ConnectionService.GetConnection`. Expected.                                                                                                                                                                                                                                                            |
| 11 | ShareConnectionIT                      | bigqueryconnection   | FAIL (env)  | FAIL     | Same as #8 — `UNIMPLEMENTED` from `ConnectionService.{GetIamPolicy,SetIamPolicy}`. Expected.                                                                                                                                                                                                                                              |
| 12 | UpdateConnectionIT                     | bigqueryconnection   | FAIL (env)  | FAIL     | Same as #8 — `UNIMPLEMENTED` from `ConnectionService.UpdateConnection`. Expected.                                                                                                                                                                                                                                                         |
| 13 | CreateAmazonS3TransferIT               | bigquerydatatransfer | FAIL (env)  | FAIL     | Env-var stubs unblocked `<clinit>`; the IT's `setUp` then issues a `POST` against `https://bigquery.googleapis.com/bigquery/v2/projects/dev/datasets` (not the emulator) and gets `401 Unauthorized` from live BigQuery. The snippet's `BigQuery` client constructor does not honor `BIGQUERY_EMULATOR_HOST` — snippet-side wiring bug; out of Phase B scope. |
| 14 | DisableTransferConfigIT                | bigquerydatatransfer | FAIL (env)  | FAIL     | Env-var stubs unblocked `<clinit>` and `taskfiles/thirdparty.yml` pre-creates the `emulator-fixture` config via the new `datatransfer` REST handler. The Java DTS gapic client connects via gRPC, **not** REST — `DataTransferServiceClient.create()` dials `bigquerydatatransfer.googleapis.com` (or the emulator's gRPC :9060 if the gapic settings honor an override), and the emulator's gRPC port has no `DataTransferService` registered → `UNIMPLEMENTED`. Phase B's REST port is unreachable from the gapic client without a gRPC bridge; Phase C / Phase D scope. |
| 15 | ReEnableTransferConfigIT               | bigquerydatatransfer | FAIL (env)  | FAIL     | Same as #14 — `UNIMPLEMENTED` from the gapic DTS client.                                                                                                                                                                                                                                                                                  |

Tally: **2 PASS + 11 FAIL + 2 SKIP** (Phase A: 0 PASS + 13 FAIL + 2 SKIP). Net delta: **+2 PASS** (rows 2, 6 — gzip middleware), **6 root-cause shifts** (rows 1, 5, 7, 8–12, 13, 14, 15 — emulator-bug and env-var-shape failures resolved, replaced by deeper, more honest failure modes that Phase C / Phase D can target directly).

### Side-quests taken

- **Gzip middleware** (`fix(gateway): decode Content-Encoding gzip on REST POST bodies`, 1d56e4a): wired `middleware.WithGunzipRequestBody` as the first middleware in `gateway/server.go`. Unblocked rows 2, 5, 6, 7. Direct PASS for rows 2, 6; rows 5 and 7 progressed past gzip and surfaced new (deferred) root causes.
- **Null-ACL fix** (`fix(gateway): return access:[] for empty dataset ACL instead of nil`, b581750): added a non-omitempty `Access` JSON tag on `bqtypes.Dataset` and an explicit `[]map[string]interface{}{}` initialisation in `gateway/handlers/datasets.go`. Eliminated the AuthorizeDatasetIT NPE; the IT progressed to a new emulator-bug shape (row 1).
- **Env-var stubs + DTS fixture pre-create** (`feat(thirdparty): stub external-resource env vars for live-IT lane`, 2683bc0): exported emulator-safe defaults for `AWS_ACCOUNT_ID`/`AWS_ROLE_ID`/`AWS_ACCESS_KEY_ID`/`AWS_SECRET_ACCESS_KEY`/`AWS_BUCKET`/`MY_SQL_DATABASE`/`MY_SQL_INSTANCE`/`DTS_TRANSFER_CONFIG_NAME` directly in `taskfiles/thirdparty.yml`, and added a best-effort `curl POST` to pre-create the `emulator-fixture` transfer config via the new datatransfer handler. Unblocked rows 8–15 from `<clinit>` short-circuits; rows now surface the underlying gRPC `UNIMPLEMENTED` (8–12, 14, 15) or a snippet-side wiring bug (13).
- **datatransfer REST port** (`feat(datatransfer): port go-googlesql REST shell into gateway`, a2c5103): full file-by-file port of `go-googlesql/api/datatransfer/{handler,handler_runs,handler_scheduled_query,handlers_project_scoped,catalog,paging}.go` into `gateway/handlers/datatransfer/`, with adapter shims for go-googlesql's `bqhttp.WriteJSONQuery` (replaced with a localised `writeJSON`/`writeAPIError` pair) and `apiregion.CheckHTTP` (dropped — this repo's REST surface is loopback-only). The Phase B failure mode for rows 14/15 confirms the Java gapic client uses gRPC, not REST, so even though the REST port is functionally complete it does not move the IT verdict.
- **bqconnection / bqstorage skeleton packages** (`feat(handlers): scaffold bqconnection / bqstorage skeleton packages`, 429d927): documented-deferral packages with no-op `Register` and `NotImplementedHTTP` stubs. Each package's `doc.go` carries the per-IT mapping to its go-googlesql source so Phase C can pick up where Phase B stopped.

### Side-quests deferred (with rationale)

- **`bqconnection` gRPC port**: the go-googlesql implementation depends on `cloud.google.com/go/bigquery/connection/apiv1/connectionpb` and `google.golang.org/genproto/googleapis/iam/v1` (~30 transitive packages this repo does not currently link), and the existing emulator C++ backend has no connection-record storage layer to bind against. Adding either is a Phase C / Phase D scope item, not a "shallow port" Phase B item.
- **`bqstorage` Write gRPC port**: depends on `cloud.google.com/go/bigquery/storage/apiv1/storagepb` (~30 generated proto files for the streaming Write API), the proto-descriptor + arrow-decode helpers go-googlesql keeps in `api/bqstorage/proto_*.go` (~3 KLOC), and a streaming-friendly hook into this repo's tabledata insertAll path. Same scope-budget reason as bqconnection.
- **DTS gRPC bridge** (datatransfer over gRPC instead of REST): the Java gapic client dials `DataTransferService` via gRPC, not REST, so the in-process REST port shipped in Phase B is unreachable from the snippet ITs. A gRPC bridge would either re-port go-googlesql's gRPC handlers or build a shim that translates `DataTransferService` gRPC calls into REST calls into our handler. Either is non-trivial; Phase C scope.
- **AuthorizeDatasetIT POST-as-PATCH path** (row 1): the Java client's `dataset.update()` round-trips as a bare `POST /bigquery/v2/projects/{p}/datasets/{id}` (no `:undelete` segment). The current `DatasetCustomMethodPOST` handler rejects that as an invalid custom method. Fixing it requires either teaching the handler to interpret `POST` with body as `PATCH`, or honoring the `X-HTTP-Method-Override: PATCH` header. Surgical but contested (which path does live BigQuery actually expose? — needs research). Out of Phase B scope; logged here for Phase C.
- **QueryMaterializedView zero-columns engine bug** (row 5): the materialized view's `SELECT *` resolves to zero columns inside the GoogleSQL analyzer, which is an engine-side bug, not a gateway bug. Out of Phase B scope.
- **CreateAmazonS3TransferIT BIGQUERY_EMULATOR_HOST honoring** (row 13): the snippet's `BigQuery` client constructor (in `CreateAmazonS3TransferIT.setUp:115`) does not honor `BIGQUERY_EMULATOR_HOST`, so it talks to live BigQuery and 401s. Snippet-side wiring fix; out of Phase B scope.

### Deviation from the plan

The plan's `1.bqconnection` and `3.bqstorage` work items projected file-by-file ports of the gRPC servers. In practice, both ports require pulling in the `cloud.google.com/go/bigquery/{connection,storage}/apiv1/*pb` proto packages (≥30 transitive packages each) plus storage-layer integration that this repo's C++ backend does not yet model. Both expansions are larger than Phase B's "shallow port" framing, so they are documented-deferred to Phase C / Phase D as `feat(handlers): scaffold bqconnection / bqstorage skeleton packages` (commit 429d927). The skeleton packages carry a per-IT mapping in their `doc.go` so the next phase has a one-to-one rebuild target.

The `datatransfer` port is unaffected by this scope adjustment — it is a pure REST handler surface and ports cleanly. However, the Java DTS gapic client uses gRPC, not REST, so the REST port does not move the live-IT verdict for rows 14/15. This was not anticipated in Phase B's `Surface map` — rows 14/15 are listed as `PATCH /v1/.../transferConfigs/{id}` REST routes, but the Java client is gRPC-only. Phase C should either build a gRPC bridge or re-port go-googlesql's gRPC handlers.

## Next plan(s)

- [java-its-missing-tests_c9d0e1f2.plan.md](java-its-missing-tests_c9d0e1f2.plan.md)
