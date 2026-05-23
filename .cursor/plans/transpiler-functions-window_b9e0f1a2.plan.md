---
name: transpiler-functions-window
overview: "Phase 5k: function disposition table, window functions, skiplist fallback."
todos:
  - id: function-table
    content: "Add functions.yaml: BQ function → DuckDB | skiplist | fallback; loader for ResolvedFunctionCall"
    status: pending
  - id: emit-window
    content: "Emit ResolvedAnalyticScan: ROW_NUMBER, RANK, SUM OVER PARTITION BY"
    status: pending
  - id: skiplist-fallback
    content: "Skiplist functions trigger ReferenceImplEngine fallback; log reason at debug"
    status: pending
isProject: false
---

# Phase 5k: Transpiler Functions Window

## Prerequisites

- [transpiler-struct-unnest_a8d9e0f1.plan.md](transpiler-struct-unnest_a8d9e0f1.plan.md)
- [query-select-e2e_b3e4f5a6.plan.md](query-select-e2e_b3e4f5a6.plan.md)

## Verification

```bash
go test -tags=integration ./gateway/e2e/... -run Window
```

## Done criteria

- functions.yaml covers ≥50 functions
- Window queries work or fall back

## Next plan(s)

- [duckdb-arrow-pipeline_c0f1a2b3.plan.md](duckdb-arrow-pipeline_c0f1a2b3.plan.md)
