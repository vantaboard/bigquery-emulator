---
name: transpiler-skeleton
overview: "Phase 5f: DuckDB transpiler visitor skeleton, shape tracker, type lowering."
todos:
  - id: transpiler-skeleton
    content: "Add transpiler.{h,cc} ResolvedASTVisitor; Transpile(resolved_query)->std::string"
    status: completed
  - id: shape-tracker
    content: "Add SHAPE_TRACKER.md: node kinds done|skiplist|not_started"
    status: completed
  - id: type-lowering
    content: "Add types.{h,cc}: googlesql TypeKind → DuckDB SQL type names"
    status: completed
isProject: false
---

# Phase 5f: Transpiler Skeleton

## Prerequisites

- [dryrun-gateway-e2e_w8f9a0b1.plan.md](dryrun-gateway-e2e_w8f9a0b1.plan.md)

## Verification

```bash
cmake --build build-out --target duckdb_transpiler
```

## Done criteria

- Transpiler skeleton compiles
- SHAPE_TRACKER.md exists

## Next plan(s)

- [transpiler-emit-scans_d5a6b7c8.plan.md](transpiler-emit-scans_d5a6b7c8.plan.md)
