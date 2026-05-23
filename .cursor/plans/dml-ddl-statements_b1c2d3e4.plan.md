---
name: dml-ddl-statements
overview: "Phase 6: implement DML (INSERT, UPDATE, DELETE, MERGE) and DDL (CREATE/DROP/ALTER TABLE, TRUNCATE) via GoogleSQL analyzer + engine-specific execution, expose dmlStats on jobs.query, and wire tabledata.insertAll as streaming insert path."
todos:
  - id: dml-analyzer
    content: "Extend query handler to classify statement kind (SELECT vs DML vs DDL) from analyzed AST; reject unsupported kinds with clear UNIMPLEMENTED until implemented"
    status: pending
  - id: insert-dml
    content: "Reference impl: INSERT INTO ds.t VALUES / INSERT SELECT via evaluator write path; update Storage AppendRows; return numDmlAffectedRows in dmlStats"
    status: pending
  - id: update-delete
    content: "Reference impl: UPDATE/DELETE against memory storage (scan+rewrite rows); DuckDB engine: emit UPDATE/DELETE SQL against attached tables"
    status: pending
  - id: merge-basic
    content: "MERGE INTO ... WHEN MATCHED/NOT MATCHED for single-table target (memory + DuckDB); document limitations vs BigQuery MERGE"
    status: pending
  - id: ddl-create-drop
    content: "Implement CREATE TABLE AS SELECT, CREATE TABLE (schema), DROP TABLE via catalog + storage; ALTER TABLE ADD COLUMN"
    status: pending
  - id: insertall-handler
    content: "Wire tables.insertAll: validate schema, convert JSON rows to storage rows, append via Storage; return insertErrors per row"
    status: pending
  - id: dmlstats-response
    content: "Populate bqtypes.DmlStats (insertedRowCount, updatedRowCount, deletedRowCount) on jobs.query responses for DML statements"
    status: pending
  - id: dml-e2e
    content: "E2E: INSERT then SELECT count; UPDATE then verify; DELETE; CREATE TABLE AS SELECT; insertAll round-trip"
    status: pending
isProject: false
---

# Phase 6: DML & DDL

## Prerequisites

- [reference-impl-execution_9c0d1e2f.plan.md](reference-impl-execution_9c0d1e2f.plan.md) — query execution path
- [catalog-grpc-tabledata-e2e_1a2b3c4d.plan.md](catalog-grpc-tabledata-e2e_1a2b3c4d.plan.md) — catalog + tabledata stubs

## Scope

**In:** Core DML/DDL on both engines where feasible, insertAll, dmlStats.

**Out:** COPY/export jobs, load jobs from GCS, schema update jobs async lifecycle, TRUNCATE TABLE via jobs API.

## Key files

- `frontend/handlers/query.{h,cc}` — statement routing
- `frontend/handlers/tabledata.{h,cc}` — insertAll
- `backend/engine/reference_impl/` — write paths
- `backend/engine/duckdb/duckdb_engine.{h,cc}` — DML SQL emission
- `gateway/handlers/tables.go` — insertAll REST
- `gateway/bqtypes/types.go` — DmlStats

## Reference

- [docs/bigquery/docs/reference/rest/v2/tables/insertAll.md](../../docs/bigquery/docs/reference/rest/v2/tables/insertAll.md)
- ROADMAP Phase 6

## Verification

```bash
# insertAll
curl -X POST localhost:9050/bigquery/v2/projects/test/datasets/ds/tables/t/insertAll \
  -H 'Content-Type: application/json' \
  -d '{"rows":[{"json":{"id":"1","name":"a"}}]}'

# DML via query
curl -X POST localhost:9050/bigquery/v2/projects/test/queries \
  -d '{"query":"INSERT INTO ds.t (id,name) VALUES (\"2\",\"b\")","useLegacySql":false}'
```

## Done criteria

- insertAll appends rows readable via SELECT *.
- INSERT/UPDATE/DELETE return dmlStats with correct counts on memory storage.
- CREATE TABLE + DROP TABLE round-trip via SQL.
- DuckDB engine handles same DML shapes via emitted SQL (or documents gaps).

## Next plan

[storage-read-api_f5a6b7c8.plan.md](storage-read-api_f5a6b7c8.plan.md)
