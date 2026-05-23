---
name: duckdb-engine-exec
overview: "Phase 5i: DuckDB engine ExecuteQuery, attach storage, fallback policy, basic E2E."
todos:
  - id: duckdb-attach
    content: "duckdb_engine ExecuteQuery: ATTACH Parquet tables, run transpiled SQL"
    status: pending
  - id: fallback-policy
    content: "Unimplemented node → UNIMPLEMENTED; --on_unknown_fn=fallback delegates to ReferenceImplEngine"
    status: pending
  - id: e2e-basic-select
    content: "E2E --engine=duckdb --storage=duckdb: SELECT 1; SELECT * FROM ds.t; GROUP BY query"
    status: pending
isProject: false
---

# Phase 5i: Duckdb Engine Exec

## Prerequisites

- [transpiler-emit-join-agg_e6b7c8d9.plan.md](transpiler-emit-join-agg_e6b7c8d9.plan.md)
- [duckdb-storage-ddl_p1e2f3a4.plan.md](duckdb-storage-ddl_p1e2f3a4.plan.md)

## Verification

```bash
curl -X POST localhost:9050/bigquery/v2/projects/test/queries -d '{"query":"SELECT COUNT(*) c FROM ds.t","useLegacySql":false}'
```

## Done criteria

- Basic SELECT/JOIN/GROUP BY succeed on DuckDB engine

## Next plan(s)

- [transpiler-struct-unnest_a8d9e0f1.plan.md](transpiler-struct-unnest_a8d9e0f1.plan.md)
