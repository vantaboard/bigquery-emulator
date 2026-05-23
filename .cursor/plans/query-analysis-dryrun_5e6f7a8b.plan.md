---
name: query-analysis-dryrun
overview: "Phase 4: link GoogleSQL Analyzer into the engine, implement Query.DryRun gRPC, map analysis errors to BigQuery HTTP envelope, and make jobs.query?dryRun=true work end-to-end."
todos:
  - id: vendor-googlesql
    content: "Add GoogleSQL as Bazel external dep (preferred) or document sibling checkout at ../googlesql; MODULE.bazel bazel_dep + local_path_override for dev; CMake remains stub-only with clear error"
    status: pending
  - id: analyzer-catalog
    content: "Implement backend/catalog/googlesql_catalog.{h,cc}: googlesql::Table adapter over Storage; register datasets/tables for name resolution"
    status: pending
  - id: dryrun-rpc
    content: "Implement frontend/handlers/query.cc DryRun: parse SQL with googlesql::Parser, Analyze with googlesql::Analyzer, return DryRunResponse schema + estimated_bytes_processed"
    status: pending
  - id: analyze-errors
    content: "Map googlesql analysis errors to gRPC InvalidArgument with line/col in status details; gateway maps to HTTP 400 invalidQuery with ErrorProto location field"
    status: pending
  - id: go-dryrun-handler
    content: "Wire gateway/handlers/queries.go QueryRun: when dryRun=true, call QueryClient.DryRun, return QueryResponse with schema + totalBytesProcessed, jobComplete=true, empty rows"
    status: pending
  - id: type-reflection
    content: "Add backend/schema/googlesql_to_bq.{h,cc}: googlesql::Type → proto FieldSchema / bqtypes.TableSchema"
    status: pending
  - id: dryrun-test
    content: "E2E test: create table, dryRun SELECT * FROM ds.t returns schema matching table; dryRun SELECT bad syntax returns 400 invalidQuery"
    status: pending
isProject: false
---

# Phase 4: Query analysis (GoogleSQL Analyzer)

## Prerequisites

- [catalog-grpc-tabledata-e2e_1a2b3c4d.plan.md](catalog-grpc-tabledata-e2e_1a2b3c4d.plan.md) — catalog populated with tables.
- [grpc-contract-go-cpp_8a9b0c1d.plan.md](grpc-contract-go-cpp_8a9b0c1d.plan.md)
- GoogleSQL source available (sibling `../googlesql` or Bazel fetch).

## Scope

**In:** Analyzer dry-run, error mapping, schema reflection, `jobs.query?dryRun=true` E2E.

**Out:** Query execution (Plans 07–09). DML analysis edge cases beyond returning errors.

## Key files

- `MODULE.bazel`, `BUILD.bazel` — GoogleSQL deps
- `backend/catalog/googlesql_catalog.{h,cc}`
- `backend/schema/googlesql_to_bq.{h,cc}`
- `frontend/handlers/query.{h,cc}`
- `gateway/handlers/queries.go`
- `gateway/handlers/errors.go` (new) — gRPC→HTTP error mapping

## Reference

- `cloud-spanner-emulator` — links GoogleSQL via Bazel; copy dependency pattern.
- ROADMAP Open Questions: vendoring strategy.

## Verification

```bash
# after building with GoogleSQL linked:
curl -X POST localhost:9050/bigquery/v2/projects/test/queries \
  -H 'Content-Type: application/json' \
  -d '{"query":"SELECT * FROM ds.t","dryRun":true,"defaultDataset":{"datasetId":"ds"}}'
# expect schema + totalBytesProcessed, jobComplete=true
```

## Done criteria

- Dry run against empty catalog: `SELECT 1` returns INT64 schema.
- Dry run against populated table: schema matches table fields.
- Syntax error returns BigQuery-shaped 400 with `invalidQuery`.
- No query execution yet (rows empty on dryRun).

## Next plans (parallel)

- [reference-impl-execution_9c0d1e2f.plan.md](reference-impl-execution_9c0d1e2f.plan.md)
- [duckdb-transpiler-core_3a4b5c6d.plan.md](duckdb-transpiler-core_3a4b5c6d.plan.md)
