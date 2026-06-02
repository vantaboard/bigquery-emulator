---
name: jobs.insert + resumable upload
overview: "Implement the four `gateway/handlers/jobs.go` stubs (`JobInsert`, `JobInsertUpload`, plus the get/cancel paths that share their plumbing) so that `POST /bigquery/v2/projects/{p}/jobs` and `POST /upload/bigquery/v2/projects/{p}/jobs` stop returning 501. Prerequisite for plans 09, 10, 11."
todos:
  - id: tp08_audit
    content: "Map the four stubs in gateway/handlers/jobs.go:24-57 to their request shapes (query / load / extract / copy / ddl) and the matching engine entry points (Engine::ExecuteQuery, Storage::*)."
    status: pending
  - id: tp08_sync_query
    content: "Sync-query path: confirm it already works (this is the existing JobInsert path); ensure JobInsert records into the registry from plan 04."
    status: pending
  - id: tp08_async_query
    content: "Async-query path: queue the job in jobs.Registry, return immediately with state=PENDING, run the query on a background goroutine, transition through RUNNING -> DONE with results."
    status: pending
  - id: tp08_ddl_via_job
    content: "DDL-via-job path (test_ddl_create_view): same async surface, but the job body is a DDL statement that ends up at the control-op executor."
    status: pending
  - id: tp08_multipart_upload
    content: "JobInsertUpload (multipart): parse the multipart body (metadata + media), persist the media to a temp buffer, then dispatch to the load-job pipeline (plan 10 owns the data side, this plan owns the protocol)."
    status: pending
  - id: tp08_resumable_upload
    content: "JobInsertUpload (resumable): implement the three-phase resumable protocol (initiate -> chunk uploads with Content-Range -> finalize), persist the upload session, then hand off to the load-job pipeline."
    status: pending
  - id: tp08_load_dispatch
    content: "Inside JobInsert with configuration.load, mint a job, return immediately, and dispatch to Storage::LoadFromURIs (the implementation comes from plan 10; this plan stops at the dispatch boundary with a 501 from the storage layer when plan 10 has not landed)."
    status: pending
  - id: tp08_get_cancel
    content: "JobGet / JobCancel — refresh against the registry now that async jobs have lifecycle (these handlers were stubbed in plan 04 against sync-only jobs)."
    status: pending
  - id: tp08_tests
    content: "Add gateway/handlers/jobs_insert_test.go covering each job kind dispatch; rerun task thirdparty:node-bigquery-tests and python-bigquery-tests for the load + DDL-via-job rows."
    status: pending
  - id: tp08_index
    content: "Flip thirdparty-00-completion-index.plan.md todo `tp08` to `completed`."
    status: pending
isProject: false
---

# Thirdparty 08 — `JobInsert` + resumable upload

## Source

- `fix_thirdparty_failures_cbe91a41.plan.md` Tier 4.1 (now deleted).
- Failing tests:
  - python-bigquery-tests `test_load_table_add_column`,
    `test_load_table_relax_column` — 501 on
    `POST /upload/bigquery/v2/projects/{p}/jobs?uploadType=resumable`.
  - python-bigquery-tests `test_ddl_create_view` — 501 on
    `POST /bigquery/v2/projects/{p}/jobs`.
- Stubs: [`gateway/handlers/jobs.go:24-57`](../../gateway/handlers/jobs.go).
- Approach precedent: the conformance-driven approach in
  [`.cursor/plans/conformance-routing-matrix.plan.md`](./conformance-routing-matrix.plan.md).

## Prerequisites

- [`thirdparty-04-jobs-registry-surface.plan.md`](./thirdparty-04-jobs-registry-surface.plan.md)
  — the registry surface is in place so `JobInsert` can register
  minted jobs and `JobGet` / `JobCancel` can find them.

## Scope

This plan covers the **gateway-side protocol** for `JobInsert` and
`JobInsertUpload` for every job kind (query, load, extract, copy,
DDL-via-job, script). The **data-plane** for load / extract / copy
lives in plans 09 / 10 / 11. When this plan lands, the dispatcher
that decodes the inbound request and either runs synchronously or
mints an async registry entry is complete; the storage primitives
the dispatcher calls are stubs until those plans land.

## Implementation

### Sequencing

1. **Sync-query path** (`configuration.query` + `dryRun=false` +
   sync semantics) — already routed; confirm the registry handoff
   from plan 04 is intact.
2. **Async-query path** — queue, return immediately, run on a
   background goroutine, transition state.
3. **DDL-via-job path** — async-query path where the body is a DDL
   statement; routed through the control-op executor.
4. **Load-job dispatch** — `configuration.load` job kind. Mint
   job, return immediately, hand off to `Storage::LoadFromURIs`
   (the call is wired here; the storage primitive itself comes
   from plan 10).
5. **Multipart upload** — `POST /upload/.../jobs?uploadType=multipart`.
   Parse the multipart body (job metadata + media), buffer the
   media, then enter the load-job dispatch.
6. **Resumable upload** — `POST /upload/.../jobs?uploadType=resumable`.
   Three-phase protocol:
   - Initiate (`POST` with empty body + `X-Upload-Content-*` headers)
     returns a `Location:` upload session URL.
   - Chunk PUTs to the session URL with `Content-Range`.
   - Finalize PUT returns the minted job and dispatches.
7. **Extract / copy dispatch** — same pattern as load; the storage
   primitives come from plans 11 / 09.

### Async job lifecycle

- `jobs.Registry` already supports state transitions (used by
  plan 04). Each async dispatcher:
  - `Register(job)` immediately, state=`PENDING`.
  - Goroutine: state=`RUNNING`, do the work, state=`DONE` (with
    `errorResult` set on failure) or state=`CANCELLED` if
    `cancelRequested` was set.
  - Caller polls via `JobGet`; cancels via `JobCancel`.

### Error envelopes

- Every dispatcher returns BigQuery-shaped errors via
  `gateway/handlers/errors.go` (existing helpers).
- Resumable upload session expiry is handled per BigQuery docs
  (24h default; surface 410 Gone after expiry).

### Backpressure & limits

- The multipart and resumable buffers must cap inbound size
  (configurable, default e.g. 5 GiB to match BigQuery's per-load
  limit). Exceeding the cap returns 413.

## Tests

- `gateway/handlers/jobs_insert_test.go` — table-driven dispatch
  tests covering every job kind, with the storage primitives
  mocked.
- `gateway/handlers/jobs_upload_test.go` — multipart + resumable
  protocol tests, with focus on the three-phase resumable flow
  including chunk-out-of-order rejection and resumable-session
  expiry.
- `task thirdparty:python-bigquery-tests` — `test_ddl_create_view`
  goes green. Load tests still fail with the storage-layer 501s
  until plan 10 lands; this is expected.
- `task thirdparty:node-bigquery-tests` — Jobs sub-suite from
  plan 04 still passes; load / extract / copy rows are blocked on
  plans 09 / 10 / 11.

## Done criteria

- All four stubs in `gateway/handlers/jobs.go:24-57` are replaced
  with implementations.
- Multipart + resumable upload handshakes are spec-compliant
  enough for the python and node clients to drive them.
- Async lifecycle (PENDING -> RUNNING -> DONE / CANCELLED) is
  observable via JobGet / JobCancel.
- `thirdparty-00-completion-index.plan.md` todo `tp08` flipped to
  `completed`.
