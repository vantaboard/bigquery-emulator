---
name: transpiler-emit-scans
overview: "Phase 5g: Emit literals, columns, functions, table scan, and WHERE filter."
todos:
  - id: emit-literals-scalars
    content: "Emit ResolvedLiteral, ResolvedColumn, ResolvedFunctionCall (SAFE_CAST, COALESCE, IFNULL)"
    status: pending
  - id: emit-scan-filter
    content: "Emit ResolvedScan (table scan), ResolvedFilter (WHERE)"
    status: pending
isProject: false
---

# Phase 5g: Transpiler Emit Scans

## Prerequisites

- [transpiler-skeleton_c4f5a6b7.plan.md](transpiler-skeleton_c4f5a6b7.plan.md)

## Verification

```bash
ctest --test-dir build-out -R transpiler_scan
```

## Done criteria

- Literal and scan emission unit tests pass

## Next plan(s)

- [transpiler-emit-join-agg_e6b7c8d9.plan.md](transpiler-emit-join-agg_e6b7c8d9.plan.md)
