---
name: transpiler-emit-join-agg
overview: "Phase 5h: Emit JOIN, GROUP BY, ORDER BY, LIMIT/OFFSET."
todos:
  - id: emit-join-agg-sort
    content: "Emit ResolvedJoinScan, ResolvedAggregateScan, ResolvedOrderByScan, LIMIT/OFFSET"
    status: completed
isProject: false
---

# Phase 5h: Transpiler Emit Join Agg

## Prerequisites

- [transpiler-emit-scans_d5a6b7c8.plan.md](transpiler-emit-scans_d5a6b7c8.plan.md)

## Verification

```bash
ctest --test-dir build-out -R transpiler_join
```

## Done criteria

- JOIN/GROUP BY transpilation unit tests pass

## Next plan(s)

- [duckdb-engine-exec_f7c8d9e0.plan.md](duckdb-engine-exec_f7c8d9e0.plan.md)
