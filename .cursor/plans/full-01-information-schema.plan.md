---
name: Full 01 — Complete INFORMATION_SCHEMA views
overview: Extend the INFORMATION_SCHEMA virtual-table surface from the current three views (TABLES, COLUMNS, SCHEMATA) to the full set tooling expects - VIEWS, ROUTINES, TABLE_OPTIONS, COLUMN_FIELD_PATHS, PARTITIONS, TABLE_STORAGE, KEY_COLUMN_USAGE/CONSTRAINTS, and the JOBS_* views - all materialized from Storage + the job store through the existing VirtualCatalogTable hook.
est_effort: ~2 weeks
isProject: true
todos:
  - id: enumerate-views
    content: "Inventory BigQuery's INFORMATION_SCHEMA views against docs/bigquery/docs/information-schema-*.md and the upstream reference; pick the tooling-critical subset to land (VIEWS, ROUTINES, TABLE_OPTIONS, COLUMN_FIELD_PATHS, PARTITIONS, TABLE_STORAGE, KEY_COLUMN_USAGE, JOBS / JOBS_BY_PROJECT) and record exact column schemas."
    status: pending
  - id: generalize-kind-enum
    content: "Generalize InfoSchemaViewKind in backend/catalog/info_schema_table.h beyond kTables/kColumns/kSchemata; add a per-view row-schema + GenerateRows() arm for each new view; keep MaterializeInDuckDB / CreateEvaluatorTableIterator generic."
    status: pending
  - id: views-routines
    content: "VIEWS + ROUTINES views: source rows from view_registry / udf_registry / tvf_registry / procedure_registry (read-through to DuckDBStorage __bqemu_routines). Surface view_definition / routine_definition text and routine_type."
    status: pending
  - id: options-fieldpaths
    content: "TABLE_OPTIONS + COLUMN_FIELD_PATHS: emit table option_name/option_value pairs (partitioning/clustering/description/labels metadata accepted by control-op DDL) and recurse STRUCT/ARRAY columns into dotted field_path rows with field_type."
    status: pending
  - id: partitions-storage
    content: "PARTITIONS + TABLE_STORAGE: report partition_id/total_rows/total_logical_bytes from Storage row counts (single __NULL__/__UNPARTITIONED__ partition where partitioning is metadata-only); document the best-effort byte estimates."
    status: pending
  - id: jobs-views
    content: "JOBS / JOBS_BY_PROJECT: materialize from the gateway job store (gateway/jobs/) over the engine boundary - decide whether to serve these from the gateway side (Go) since job state lives there, or pipe a job snapshot to the engine; pick the path that keeps the dataset-qualified region-* selector working."
    status: pending
  - id: register-resolve
    content: "Register each new view in backend/catalog/googlesql_catalog.cc so `FROM <dataset>.INFORMATION_SCHEMA.<VIEW>` and region-qualified `region-<r>.INFORMATION_SCHEMA.<VIEW>` resolve at analyze time; honor project/dataset scoping."
    status: pending
  - id: fixtures-trackers
    content: "Add conformance/fixtures/info_schema/ fixtures per view (schema_only where row content is environment-dependent); update ROADMAP.md + REST/catalog docs; drop dbt skip rows (catalog, change_history, TestDocsGenerateBigQuery) that now pass and re-run task thirdparty:dbt-bigquery-tests."
    status: pending
---

# Full 01 — Complete INFORMATION_SCHEMA views

## Why first

INFORMATION_SCHEMA is how dbt builds its catalog/docs, how bigframes
introspects schemas, and how a lot of "show me the tables" tooling
works. The emulator currently ships only three views, so any tool that
queries `INFORMATION_SCHEMA.VIEWS` / `.ROUTINES` / `.PARTITIONS` /
`.JOBS` falls over. The `dbt-bigquery-tests` skip matrix explicitly
lists `catalog`, `change_history`, and `TestDocsGenerateBigQuery` as
INFORMATION_SCHEMA-blocked.

## Current state

```28:56:backend/catalog/info_schema_table.h
enum class InfoSchemaViewKind {
  kTables,
  kColumns,
  kSchemata,
};
```

Only `TABLES`, `COLUMNS`, `SCHEMATA` are materialized. The machinery —
`VirtualCatalogTable` (`backend/catalog/virtual_table.h`) +
`InfoSchemaTable::GenerateRows` + `MaterializeInDuckDB` — is reusable;
the missing work is per-view row schemas and data sources.

## Key files

- [`backend/catalog/info_schema_table.{h,cc}`](../../backend/catalog/) — the view materializer to extend
- [`backend/catalog/virtual_table.h`](../../backend/catalog/virtual_table.h) — the DuckDB attach hook (don't change the interface)
- [`backend/catalog/googlesql_catalog.cc`](../../backend/catalog/googlesql_catalog.cc) — analyzer-time registration / name resolution
- [`backend/catalog/view_registry.{h,cc}`](../../backend/catalog/), `udf_registry.cc`, `tvf_registry.cc`, `procedure_registry.cc` — sources for VIEWS / ROUTINES
- [`gateway/jobs/`](../../gateway/jobs/) — job store for JOBS / JOBS_BY_PROJECT
- `docs/bigquery/docs/information-schema-*.md` — upstream column schemas (source of truth)

## Steps

1. **Schema inventory.** For each target view, copy the exact column
   list + types from the upstream `information-schema-*.md`. These are
   contract surfaces — column names/order/types must match or tooling
   breaks.
2. **Generalize the materializer.** Replace the 3-value enum with a
   table-driven per-view descriptor (name, row schema, row generator).
3. Land views in dependency order: VIEWS/ROUTINES (registries already
   exist) → TABLE_OPTIONS/COLUMN_FIELD_PATHS (schema metadata) →
   PARTITIONS/TABLE_STORAGE (row counts) → JOBS_* (job store).
4. **JOBS placement decision.** Job state lives in the Go gateway, not
   the engine. Either (a) serve `INFORMATION_SCHEMA.JOBS*` from the
   gateway by intercepting the query before the engine, or (b) snapshot
   job metadata into a storage table the engine can scan. Prefer (b) if
   it keeps a single resolution path; document the choice.
5. Register every view for both `<dataset>.INFORMATION_SCHEMA.<VIEW>`
   and `region-<r>.INFORMATION_SCHEMA.<VIEW>` forms.
6. Fixtures + tracker + ROADMAP updates; remove unblocked dbt skip rows.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run-fixture FIXTURE=conformance/fixtures/info_schema/<new>.yaml
task conformance:run
task lint:dispositions
task thirdparty:dbt-bigquery-tests    # remove catalog / docs-generate skip rows that pass
task bazel:shutdown && task bazel:status
```

## Out of scope

- INFORMATION_SCHEMA write/DDL (these views are read-only in BigQuery).
- Streaming / reservation / capacity views (`*_TIMELINE`,
  `RESERVATIONS*`) — no local analog; leave `unsupported`.
- Real byte-accurate storage accounting; row counts + estimated bytes
  are best-effort per the ROADMAP persistence non-goal.
