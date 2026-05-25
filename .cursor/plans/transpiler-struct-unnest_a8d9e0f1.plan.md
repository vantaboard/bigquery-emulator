---
name: transpiler-struct-unnest
overview: "Phase 5j: STRUCT, UNNEST, ARRAY transpiler nodes."
todos:
  - id: struct-unnest
    content: "Emit ResolvedArrayScan, ResolvedStructFieldAccess, ResolvedCreateArray, ResolvedCreateStruct; document LIST/STRUCT quirks"
    status: completed
isProject: false
---

# Phase 5j: Transpiler Struct Unnest

## Prerequisites

- [duckdb-engine-exec_f7c8d9e0.plan.md](duckdb-engine-exec_f7c8d9e0.plan.md)

## Verification

```bash
ctest --test-dir build-out -R transpiler_struct
```

## Done criteria

- UNNEST + STRUCT nodes in SHAPE_TRACKER as done or skiplist

## Next plan(s)

- [transpiler-functions-window_b9e0f1a2.plan.md](transpiler-functions-window_b9e0f1a2.plan.md)
