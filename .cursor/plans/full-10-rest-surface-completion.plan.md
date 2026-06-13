---
name: Full 10 — REST surface completion
overview: Fill the REST gaps behind client-library probes and load jobs - resumable/multipart media upload edge cases, the peripheral resource surfaces (models / routines / rowAccessPolicies / migration / dataTransfer) moving from structural stubs to per-method behavior where feasible, and jobs.list filters / pagination - so client libraries that exercise these paths stop hitting stubs.
est_effort: ~2 weeks
isProject: true
todos:
  - id: resumable-upload
    content: "Resumable + multipart media upload: the python docs snippets `test_load_table_add_column` / `test_load_table_relax_column` are skipped as 'resumable upload or other REST gaps'. Audit /upload/bigquery/v2/.../jobs against docs/bigquery/docs/reference/api-uploads.md; finish chunked/resumable session handling so a load job streamed in parts completes and applies schema add/relax."
    status: pending
  - id: routines-rest
    content: "routines.* REST: confirm list/get/insert/update/delete fully round-trip (parity-08 landed persistence + catalog RPC); fill any per-method gaps and ensure routine bodies / argument metadata serialize per the REST reference."
    status: pending
  - id: models-rest
    content: "models.* REST: with CREATE MODEL as local_stub, make the models resource return the stub-registered model metadata for list/get/delete so client-library model-management probes round-trip (without claiming inference)."
    status: pending
  - id: rap-migration-dts
    content: "rowAccessPolicies / migration / dataTransfer: keep structural stubs but make CRUD round-trips consistent (create then list/get returns the created resource); coordinate rowAccessPolicies persistence with full-07 if both are in flight."
    status: pending
  - id: jobs-list-filters
    content: "jobs.list: honor stateFilter / allUsers / minCreationTime / maxCreationTime / parentJobId query params and pagination (pageToken/maxResults) against the gateway job store; jobs.getQueryResults pagination edges."
    status: pending
  - id: fixtures-skips
    content: "Add gateway/e2e + conformance coverage for the finished paths; remove the python docs-snippets skip rows (_DOCS_SNIPPETS_EMULATOR_SKIP) that now pass; update docs/REST_API.md per-method status + ROADMAP gateway bullets."
    status: pending
---

# Full 10 — REST surface completion

## Why

The core CRUD + query surface is end-to-end, but several REST paths are
either structural stubs or have edge gaps that client libraries hit:

- Resumable/multipart upload edges (python docs snippets
  `test_load_table_add_column` / `test_load_table_relax_column` skipped
  as "resumable upload or other REST gaps").
- `models` / `routines` / `rowAccessPolicies` / `migration` /
  `dataTransfer` are "wired stubs" (ROADMAP) — per-method behavior lives
  in `docs/REST_API.md`.
- `jobs.list` filter/pagination params.

## Key files

- `gateway/handlers/` — jobs, tabledata, routines, models, etc.
- `gateway/handlers/bqv2grpc/` — gRPC adapters over the REST/catalog handlers
- [`gateway/jobs/`](../../gateway/jobs/) — job store for list filters
- `docs/bigquery/docs/reference/api-uploads.md` — upload contract
- `docs/REST_API.md` — per-method status to update
- `third_party/python-bigquery-tests/emulator_pytest_skip.py` — skip rows to remove

## Steps

1. Resumable/multipart upload completion (highest user impact: load
   jobs are how data gets in via the REST path).
2. routines / models REST round-trip completeness.
3. rowAccessPolicies / migration / dataTransfer CRUD consistency.
4. jobs.list filters + pagination.
5. e2e + conformance coverage; remove unblocked python skip rows; update
   `docs/REST_API.md` + ROADMAP.

## Verify

```bash
go test ./gateway/...                  # gateway-side, no engine rebuild needed for most of this
task conformance:run
task thirdparty:python                 # docs snippets load_table_add_column / relax_column
task bazel:shutdown && task bazel:status
```

This plan is mostly Go (gateway) work; it may not need an engine rebuild
for the REST-only pieces, which makes it a good candidate to interleave
with an engine-lane plan if the bazel lane is busy — but still respect
the one-bazel-at-a-time rule for any piece that does.

## Out of scope

- Real BigQuery ML inference (`models` stays metadata-only).
- DataTransfer scheduled-run execution (stub CRUD only).
- Google Sheets external tables (no emulator stub, per ENGINE_POLICY).
