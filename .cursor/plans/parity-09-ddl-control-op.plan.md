---
name: Parity 09 — DDL control-op completion
overview: Finish the control-op DDL matrix - migrate ALTER TABLE off the DuckDB transpiler, land CREATE MATERIALIZED VIEW (full refresh), the EXPORT DATA writer family, LOAD DATA LOCAL, and reconcile the remaining (planned) control_op tracker rows (ResolvedDropStmt, ResolvedAnalyzeStmt, ResolvedCatalogColumnRef).
est_effort: ~1-2 weeks
isProject: true
todos:
  - id: alter-table
    content: "Migrate ALTER TABLE (ADD/DROP/RENAME COLUMN, SET OPTIONS) from the DuckDB transpiler lowering to ControlOpExecutor mutating Storage directly, emitting the BigQuery statementType envelope; per ROADMAP this is the last DDL still lowering through DuckDB SQL."
    status: completed
  - id: materialized-view
    content: "CREATE MATERIALIZED VIEW with full-refresh execution: register like a view, materialize via the coordinator into a storage table at creation (and on a manual refresh path); document the no-incremental-refresh posture in ENGINE_POLICY."
    status: completed
  - id: export-data
    content: "EXPORT DATA writer family (ResolvedExportDataStmt): local-filesystem URI export (CSV/JSON/Parquet via DuckDB COPY) through a control-op handler; gs:// URIs follow the fake-gcs-server convention the third-party lane already uses or surface the documented unsupported envelope."
    status: completed
  - id: load-data-local
    content: "LOAD DATA LOCAL <local-uri> (ResolvedAuxLoadDataStmt): control-op reader for CSV/JSON/Parquet local files into Storage (reuse the jobs.insert media-upload load-job machinery where possible); gs:// stays unsupported per ENGINE_POLICY."
    status: completed
    note: "ResolvedCatalogColumnRef stays (planned): no plan-09 DDL shape emitted that node; ALTER/LOAD use ResolvedColumnDefinition instead."
  - id: reconcile-planned-rows
    content: "Reconcile tracker rows that say (planned) but may already be landed (ResolvedDropStmt - DROP TABLE is implemented; ResolvedAnalyzeStmt - ROADMAP says ANALYZE flows through control op): verify behavior, fix the row or the code, keep parity gate green. Implement ResolvedCatalogColumnRef if any landed DDL shape needs it."
    status: completed
  - id: fixtures-trackers
    content: Fixtures under conformance/fixtures/ddl/ (alter round-trip via tables.get schema, matview query-after-create, export file content assertion, load-data row count); flip SHAPE_TRACKER rows; update ROADMAP Schema-and-DDL bullet.
    status: completed
---

# Parity 09 — DDL control-op completion

## Why

ROADMAP's DDL bullet is one migration away from ✅: *"`ALTER TABLE`
still lowers through the DuckDB transpiler pending its own migration;
`CREATE MATERIALIZED VIEW` (full refresh), `EXPORT DATA`, partitioning /
clustering hints ... still pending."* ENGINE_POLICY additionally defers
`LOAD DATA LOCAL` on the control-op route. These are catalog/storage
operations — exactly what `ControlOpExecutor` exists for — and several
tracker rows still carry `(planned)` markers that may already be stale.

## Key files

- [`backend/engine/control/control_op_executor.{h,cc}`](../../backend/engine/control/) (+ sibling translation units like `pipe_export_data.cc` for the lint-cap pattern)
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc)
- [`backend/storage/storage.h`](../../backend/storage/storage.h) — may need `AlterTable` surface
- [`backend/schema/`](../../backend/schema/) — schema mutation + BQ TableSchema conversion
- `gateway/handlers/` load-job machinery (LOAD DATA reuse)
- `conformance/fixtures/ddl/`

## Steps

1. Start with the reconcile todo — cheap, and it prevents building on
   stale assumptions about what is actually landed.
2. ALTER TABLE migration: add the storage mutation primitive, route the
   statement kinds to `ControlOpExecutor`, delete the transpiler
   lowering in the same change so there is never a dual path.
3. Materialized views: creation-time materialization through the
   coordinator (the SELECT half routes normally), stored as a regular
   table + registry entry so reads need no new path; statement type
   envelope per the REST reference.
4. EXPORT DATA: new control handler doing DuckDB `COPY (SELECT ...) TO`
   for local URIs; the existing `ResolvedPipeExportDataScan` stub
   handler should delegate to the same writer (unblocks part of plan 12).
5. LOAD DATA LOCAL: parse options (format, schema mode), read via
   DuckDB readers, append through storage; keep `gs://` unsupported.
6. Fixtures + tracker/ROADMAP flips; partitioning/clustering hints stay
   accepted-as-metadata (document if encountered).

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task bazel:shutdown && task bazel:status
```

Re-run Java/Python third-party ITs whose skip rows cite ALTER TABLE or
EXPORT DATA.

## Out of scope

- `LOAD DATA` from `gs://` — `unsupported` per ENGINE_POLICY
- Incremental materialized-view refresh — full refresh only, documented
- External Parquet drop-in / per-dataset directory layout — ROADMAP open question, not this plan
