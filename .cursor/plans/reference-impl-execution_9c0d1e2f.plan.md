---
name: reference-impl-execution
overview: "Phase 5.A: wire googlesql::reference_impl::Evaluator to Storage, implement ExecuteQuery gRPC streaming, marshal GoogleSQL Value to BigQuery f/v REST rows per StandardSqlDataType, and deliver SELECT 1 + SELECT * E2E via jobs.query."
todos:
  - id: table-adapter
    content: "Implement backend/catalog/storage_table.{h,cc}: googlesql::Table implementation reading rows from Storage ScanRows for reference impl evaluator"
    status: pending
  - id: ref-impl-engine
    content: "Fill backend/engine/reference_impl/reference_impl_engine.cc: Analyze reuses analyzer path; ExecuteQuery runs Algebrizer+Evaluator, streams rows"
    status: pending
  - id: execute-query-rpc
    content: "Implement frontend/handlers/query.cc ExecuteQuery: server-streaming QueryResultRow (schema message first, then row batches)"
    status: pending
  - id: wire-marshal
    content: "Add gateway/bqtypes/wire.go: ValueToCell(googlesql::Value or proto Cell) per StandardSqlDataType.TypeKind — INT64 as decimal string, TIMESTAMP RFC3339Z, STRUCT as positional list, NULL handling"
    status: pending
  - id: go-query-handler
    content: "Wire gateway/handlers/queries.go QueryRun (non-dryRun): call ExecuteQuery stream, assemble QueryResponse with rows, jobReference, jobComplete=true, timing stats"
    status: pending
  - id: job-lifecycle
    content: "Add in-memory job registry in gateway (sync.Map): generate jobId, store Job status DONE with statistics.creationTime/startTime/endTime/totalBytesProcessed for jobs.query responses"
    status: pending
  - id: get-query-results
    content: "Wire QueryGetResults for paginated reads when pageToken set (store result sets in job registry or re-execute — start with single-page only, document limit)"
    status: pending
  - id: e2e-select1
    content: "E2E: SELECT 1 returns one row one column; SELECT * FROM ds.t after insertAll returns inserted rows"
    status: pending
  - id: wire-marshal-test
    content: "Unit tests for wire.go covering INT64, FLOAT64 NaN, BYTES base64, TIMESTAMP, STRUCT, ARRAY, NULL"
    status: pending
isProject: false
---

# Phase 5.A: Reference Impl execution

## Prerequisites

- [query-analysis-dryrun_5e6f7a8b.plan.md](query-analysis-dryrun_5e6f7a8b.plan.md)
- [cpp-interfaces-memory-storage_2e3f4a5b.plan.md](cpp-interfaces-memory-storage_2e3f4a5b.plan.md)
- `--engine=reference_impl --storage=memory` (default ci profile)

## Scope

**In:** Reference impl execution path, wire marshaling, jobs.query E2E for SELECT shapes reference impl handles.

**Out:** DuckDB engine (Plans 08–09). Async jobs.insert query jobs (return sync query results only initially). Full pagination (single page OK).

## Key files

- `backend/engine/reference_impl/reference_impl_engine.{h,cc}`
- `backend/catalog/storage_table.{h,cc}`
- `frontend/handlers/query.{h,cc}`
- `gateway/bqtypes/wire.go` (new)
- `gateway/handlers/queries.go`
- `gateway/jobs/registry.go` (new)

## Reference docs

- [docs/bigquery/docs/reference/rest/v2/StandardSqlDataType.md](../../docs/bigquery/docs/reference/rest/v2/StandardSqlDataType.md)
- [docs/REST_API.md](../../docs/REST_API.md) — Type wire encoding table

## Verification

```bash
curl -X POST localhost:9050/bigquery/v2/projects/test/queries \
  -H 'Content-Type: application/json' \
  -d '{"query":"SELECT 1 AS one","useLegacySql":false}'
# expect rows: [{"f":[{"v":"1"}]}], jobComplete=true

go test ./gateway/bqtypes/... -run Wire
go test -tags=integration ./gateway/e2e/...
```

## Done criteria

- `SELECT 1` and `SELECT * FROM ds.t` work on memory storage via reference impl.
- Result cells match StandardSqlDataType wire rules (spot-check INT64 as string).
- JobReference populated in response.
- `--engine=reference_impl` is the default and passes E2E tests.

## Next plans

- [dml-ddl-statements_b1c2d3e4.plan.md](dml-ddl-statements_b1c2d3e4.plan.md) (needs this plan)
- [storage-read-api_f5a6b7c8.plan.md](storage-read-api_f5a6b7c8.plan.md) (needs row source from here for ref impl path)

Parallel: [duckdb-transpiler-core_3a4b5c6d.plan.md](duckdb-transpiler-core_3a4b5c6d.plan.md)
