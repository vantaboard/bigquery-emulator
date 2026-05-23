---
name: duckdb-transpiler-core
overview: "Phase 5.B part 1: build ResolvedAST→DuckDB SQL transpiler foundation — visitor skeleton, type lowering, literal/scalar nodes, single-table and two-table SELECT, JOIN, GROUP BY, ORDER BY, LIMIT."
todos:
  - id: transpiler-skeleton
    content: "Add backend/engine/duckdb/transpiler/transpiler.{h,cc} with ResolvedASTVisitor base; Transpile(resolved_query) -> std::string; per-node Emit methods returning empty TODO initially"
    status: pending
  - id: shape-tracker
    content: "Add backend/engine/duckdb/transpiler/SHAPE_TRACKER.md listing node kinds: done | skiplist | not_started; mirror go-googlesql per-shape tracker pattern"
    status: pending
  - id: type-lowering
    content: "Implement backend/engine/duckdb/transpiler/types.{h,cc}: map googlesql TypeKind to DuckDB SQL type names for CAST and column defs"
    status: pending
  - id: emit-literals-scalars
    content: "Implement Emit for: ResolvedLiteral, ResolvedColumn, ResolvedFunctionCall (SAFE_CAST, COALESCE, IFNULL only)"
    status: pending
  - id: emit-scan-filter
    content: "Implement Emit for: ResolvedScan (table scan against attached DuckDB table name), ResolvedFilter (WHERE)"
    status: pending
  - id: emit-join-agg-sort
    content: "Implement Emit for: ResolvedJoinScan (INNER/LEFT), ResolvedAggregateScan (GROUP BY), ResolvedOrderByScan, LIMIT/OFFSET"
    status: pending
  - id: duckdb-attach
    content: "In duckdb_engine ExecuteQuery: open DuckDB connection, ATTACH storage Parquet files or CREATE TABLE AS from memory export, run transpiled SQL"
    status: pending
  - id: fallback-policy
    content: "When transpiler hits unimplemented node, return error with code UNIMPLEMENTED; engine factory checks --on_unknown_fn=fallback to delegate to ReferenceImplEngine"
    status: pending
  - id: e2e-basic-select
    content: "E2E with --engine=duckdb --storage=duckdb: SELECT 1; SELECT * FROM ds.t; SELECT a, COUNT(*) FROM ds.t GROUP BY a on seeded data"
    status: pending
isProject: false
---

# Phase 5.B (part 1): DuckDB transpiler core

## Prerequisites

- [query-analysis-dryrun_5e6f7a8b.plan.md](query-analysis-dryrun_5e6f7a8b.plan.md) — resolved AST available from analyzer.
- [duckdb-persistent-storage_6c7d8e9f.plan.md](duckdb-persistent-storage_6c7d8e9f.plan.md) — DuckDB storage + attach path.

## Scope

**In:** Transpiler visitor, basic SELECT/JOIN/AGG/ORDER shapes, DuckDB execution for those shapes, fallback to reference impl.

**Out:** STRUCT, UNNEST, advanced builtins (Plan 09). Arrow→REST marshaling (Plan 09). Full function disposition table.

## Key files

- `backend/engine/duckdb/transpiler/` — new directory
- `backend/engine/duckdb/duckdb_engine.{h,cc}`
- `backend/engine/duckdb/transpiler/SHAPE_TRACKER.md`

## Reference

- ROADMAP Phase 5.B — shape-tracker + skiplist approach.
- Open Questions — dialect friction (defer MERGE/STRUCT to Plan 09/10).

## Verification

```bash
./build-out/emulator_main --engine=duckdb --storage=duckdb --data_dir=/tmp/bqemu &
# seed data via REST insertAll first
curl -X POST localhost:9050/bigquery/v2/projects/test/queries \
  -d '{"query":"SELECT COUNT(*) AS c FROM ds.t","useLegacySql":false}'
```

## Done criteria

- SHAPE_TRACKER.md lists ≥10 node kinds as `done`.
- Basic SELECT/JOIN/GROUP BY queries succeed on DuckDB engine.
- Unimplemented node triggers fallback when `--on_unknown_fn=fallback`.
- Transpiler unit tests for literal and scan emission (C++ gtest).

## Next plan

[duckdb-transpiler-advanced_7e8f9a0b.plan.md](duckdb-transpiler-advanced_7e8f9a0b.plan.md)
