---
name: ""
overview: ""
todos: []
isProject: false
---

# Control-Op Executor

## Goal

Implement the `control_op` route. DDL, metadata, and catalog
operations bypass the DuckDB SQL evaluator entirely and run through
the storage layer (`DuckDBStorage` + the C++ catalog adapter),
emitting BigQuery-shaped responses directly.

## Background

Today, DDL that lands on the engine (`CREATE TABLE`, `CREATE TABLE
AS SELECT`, `DROP TABLE`, `MERGE` storage side) is handled by ad-hoc
code paths inside the DuckDB engine. Other DDL/metadata shapes
(`CREATE VIEW`, `ALTER TABLE`, `ANALYZE`, `EXPORT`, the pipe
operator's CREATE TABLE / EXPORT forms) surface `UNIMPLEMENTED`.

The DuckDB-only roadmap would have folded each shape into the
DuckDB SQL transpiler. That is wrong for two reasons:

1. The transpiler is for query-shape SQL, not catalog mutations.
2. BigQuery's DDL response shape (the `Job` resource with the
   appropriate `statistics.query.statementType`) does not come from
   DuckDB SQL execution; it comes from the storage layer producing
   the right catalog change.

This plan extracts a dedicated `ControlOpExecutor` that owns the
routing for every `control_op` row.

## Dependencies

- `execution-disposition-registry.plan.md` so DDL/metadata rows
  carry the `control_op` disposition.
- `engine-router-foundation.plan.md` so the coordinator dispatches
  control-op statements directly to this executor.

## Scope

Statements that route through this executor:

- `ResolvedCreateTableStmt`
- `ResolvedCreateTableAsSelectStmt`
  (the SELECT half runs through the DuckDB fast path / coordinator
  as a sub-query; the write half is `control_op`)
- `ResolvedCreateViewStmt`
- `ResolvedCreateMaterializedViewStmt` (best-effort metadata, full
  refresh behavior tracked in
  `specialized-feature-policy.plan.md`)
- `ResolvedDropStmt`, `ResolvedDropFunctionStmt`
- `ResolvedAnalyzeStmt` (metadata refresh; see
  `specialized-feature-policy.plan.md` for the limits)
- `ResolvedAuxLoadDataStmt`
- `ResolvedExportDataStmt`
- `ResolvedPipeExportDataScan`
- `ResolvedPipeCreateTableScan`
- `ResolvedCatalogColumnRef` (DDL-only expression)
- `ResolvedCreateFunctionStmt` /
  `ResolvedCreateTableFunctionStmt` registration side (the function
  body's execution route is decided in
  `udf-tvf-module-routing.plan.md`)

## Implementation Plan

1. Add `backend/engine/control/` with
   `control_op_executor.{h,cc}`. Wire it into the coordinator from
   `engine-router-foundation.plan.md`.
2. Implement per-statement handlers, each producing the right
   BigQuery `Job` envelope and updating `DuckDBStorage` /
   `Catalog`. Where DuckDB SQL is the easiest implementation
   primitive (e.g. `CREATE TABLE` shapes that map to DuckDB
   `CREATE TABLE`), use DuckDB SQL as an implementation detail
   inside the handler — but the handler owns the response shape.
3. For `CREATE TABLE AS SELECT`, run the SELECT half through the
   coordinator (so it picks up the fast-path / semantic-executor
   route as appropriate) and stream the rows into a fresh
   `DuckDBStorage` table.
4. For `EXPORT DATA`, write Arrow / Parquet files to the
   configured destination (URI scheme determines the writer;
   `gs://` paths route through `fake-gcs-server` when configured,
   `file://` and bare paths write locally).
5. Update `gateway/handlers/jobs.go` so the `statistics.query.
   statementType` reflects what the control-op executor returns.

## Tests

- Unit tests per handler under `backend/engine/control/`.
- Engine-level integration tests in `gateway/e2e/` covering each
  control-op statement end-to-end through the gateway, including
  the `Job.statistics.query.statementType` field.
- Conformance fixtures under `conformance/fixtures/ddl/` asserting
  the route label is `control_op`.

## Done Criteria

- Every `control_op` row in `SHAPE_TRACKER.md` has a handler in
  `ControlOpExecutor`, a test in `gateway/e2e/`, and a conformance
  fixture under `conformance/fixtures/ddl/`.
- The DuckDB fast path no longer carries any DDL-specific code
  paths (those move into the control-op executor).
- `Job.statistics.query.statementType` matches the BigQuery REST
  documentation for every supported DDL/metadata statement.
- ROADMAP.md "DML / DDL" section's `🟡` bullets for `CREATE VIEW`,
  `ALTER TABLE`, and `ANALYZE` flip to `✅`.
