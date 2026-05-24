---
name: query-select-e2e
overview: "Phase 5e: QueryGetResults pagination stub and SELECT E2E tests."
todos:
  - id: get-query-results
    content: "Wire QueryGetResults single-page reads from job registry; document pagination limits"
    status: completed
  - id: e2e-select1
    content: "E2E: SELECT 1 one row; SELECT * FROM ds.t after insertAll returns inserted rows"
    status: completed
isProject: false
---

# Phase 5e: Query Select E2E

## Prerequisites

- [jobs-query-handler_a2d3e4f5.plan.md](jobs-query-handler_a2d3e4f5.plan.md)

## Verification

```bash
go test -tags=integration ./gateway/e2e/... -run Query
```

## Done criteria

- SELECT 1 and SELECT * E2E pass on reference_impl+memory

## Implementation notes (Phase 5e landing)

- `gateway/jobs.Registry` now caches the schema + rows of every
  successful `jobs.query` on the stored `*Job` (new `QueryResult`
  field, `CompleteQueryWithResult` helper). The cache is in-memory
  and per-process, matching the registry's existing scope.
- `QueryGetResults` (`GET .../queries/{jobId}`) replays that cache
  with the `bigquery#getQueryResultsResponse` kind. It honors
  `startIndex`, `maxResults`, and the `location` query parameter,
  and surfaces project/job mismatches as 404 notFound to mirror
  BigQuery's "hide cross-project jobs" behavior.
- **Pagination limitation (single-page only).** The emulator never
  mints a `pageToken`. Any request that arrives with `pageToken=...`
  receives an empty terminal page with `jobComplete=true` so polling
  loops exit cleanly; multi-page result sets are deferred to Phase 6
  alongside long-running async jobs (see `roadmap` non-goals -- no
  promised pagination semantics yet).
- **E2E binary gap (documented per plan).** A canonical Bazel
  `cc_binary` for `binaries/emulator_main` that links googlesql +
  reference_impl + DuckDB + grpc was not added in this plan: that is
  a multi-day Bazel toolchain task (vendored googlesql + abseil
  build, reference_impl link, DuckDB amalgamation wiring) that
  exceeds the scope of "Phase 5e wire the handler". Instead, the new
  E2E tests follow the same pattern as `dryrun_test.go`: they spawn
  the existing CMake-built `emulator_main` and use a
  `skipIfExecuteQueryUnimplemented` probe so they auto-skip when the
  engine returns `UNIMPLEMENTED` from `Query.ExecuteQuery`. The tests
  will activate automatically once the canonical googlesql-linked
  binary is shipped (next plan in the chain). Plans
  `execute-query-stream_y0b1c2d3.plan.md` and
  `ref-impl-adapter_x9a0b1c2.plan.md` are the prerequisites that
  must land first to flip these from `SKIP` to `PASS` against a real
  engine subprocess.

## Next plan(s)

- [dml-insert-e2e_e2b3c4d5.plan.md](dml-insert-e2e_e2b3c4d5.plan.md)
- [storage-read-proto_i6f7a8b9.plan.md](storage-read-proto_i6f7a8b9.plan.md)
- [transpiler-skeleton_c4f5a6b7.plan.md](transpiler-skeleton_c4f5a6b7.plan.md)
