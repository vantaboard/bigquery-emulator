---
name: jobs.Registry handler surface
overview: "Wire JobList, JobGet, JobCancel, and JobDelete in `gateway/handlers/jobs.go` against the in-memory `gateway/jobs.Registry`. The sync-query path already populates the registry, so no engine work is needed — this is a handler-only pass."
todos:
  - id: tp04_list
    content: "Implement JobList — page over Registry by project, respect maxResults / pageToken / parentJobId."
    status: pending
  - id: tp04_get
    content: "Implement JobGet — return registry entry by JobReference; 404 with bigquery-shaped JSON error envelope when missing."
    status: pending
  - id: tp04_cancel
    content: "Implement JobCancel — flip state to RUNNING-with-cancel-requested then DONE/CANCELLED; idempotent on terminal jobs."
    status: pending
  - id: tp04_delete
    content: "Implement JobDelete — remove from registry; cascade child jobs (script statements)."
    status: pending
  - id: tp04_insert_register
    content: "Confirm JobInsert (sync path) records the minted job in the registry; if it does not, add the registration call before returning."
    status: pending
  - id: tp04_tests
    content: "Add Go unit tests under gateway/handlers/jobs_test.go covering list/get/cancel/delete; rerun task thirdparty:node-bigquery-tests to confirm the Jobs > should ... block goes green."
    status: pending
  - id: tp04_index
    content: "Flip thirdparty-00-completion-index.plan.md todo `tp04` to `completed`."
    status: pending
isProject: false
---

# Thirdparty 04 — surface `jobs.Registry` over the REST handlers

## Source

- `not-implemented-routes-catalog.plan.md` Group 1 (now deleted).
- Failing node tests (from the same catalog):
  - `Jobs > should list jobs` (before-all)
  - parent of `Jobs > should retrieve a job`
  - parent of `Jobs > should cancel a job`
  - parent of `Jobs > should delete a job`

All four hit handler stubs in
[`gateway/handlers/jobs.go`](../../gateway/handlers/jobs.go) that
return 501 today.

## Prerequisites

None for the handler surface. The synchronous `jobs.query` path
already populates `Jobs` from `QueryRun`, so the registry has live
entries to surface.

## Scope

This plan covers **only** the four registry-backed CRUD handlers and
the registration step from the sync path. It does **not** implement
`JobInsert` for async load / DDL / extract paths — that is plan
[`thirdparty-08-jobs-insert-implementation.plan.md`](./thirdparty-08-jobs-insert-implementation.plan.md).

## Implementation

1. **`JobList`.** Page over `Registry.ListByProject(projectID,
   opts)`. Honor `maxResults`, `pageToken`, `parentJobId`,
   `minCreationTime`, `maxCreationTime`, `stateFilter`. Reject
   `allUsers=true` with a 501 documented in the response body since
   the emulator has no auth context.
2. **`JobGet`.** Look up by `{project, location, jobId}`. Return the
   BigQuery `Job` resource as-is. 404 with the standard
   `googleapi.Error` shape if absent.
3. **`JobCancel`.** If the job is `PENDING` or `RUNNING`, mark
   `cancelRequested=true`, transition the registry entry to
   `CANCELLED`, and return the updated job. Idempotent on `DONE` /
   `CANCELLED`.
4. **`JobDelete`.** Remove from the registry. If the entry is a
   script parent, cascade to children. Reject with a documented
   error if the job is still running.
5. **`JobInsert` (sync path only here).** Verify the sync-query path
   in `JobInsert` registers the minted job before returning. If the
   existing implementation does not, add a `Registry.Register(job)`
   call there. Async/load/upload paths stay 501 until plan 08.

## Tests

- Unit tests under `gateway/handlers/jobs_test.go` for each handler
  (list pagination, get-missing, cancel-terminal-is-noop,
  delete-cascade-children).
- `task thirdparty:node-bigquery-tests` runs to completion for the
  `Jobs > should list / retrieve / cancel / delete` block. Other
  failing rows in the same suite stay failing — they are owned by
  plans 08-11.

## Done criteria

- The four handler stubs are replaced with registry-backed
  implementations.
- The sync-query path registers minted jobs.
- New unit tests pass; the node Jobs sub-suite goes green.
- `thirdparty-00-completion-index.plan.md` todo `tp04` flipped to
  `completed`.
