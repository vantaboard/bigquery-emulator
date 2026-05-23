---
name: ref-impl-adapter
overview: "Phase 5a: storage_table adapter and reference impl engine ExecuteQuery core."
todos:
  - id: table-adapter
    content: "Implement storage_table.{h,cc}: googlesql::Table reading Storage ScanRows"
    status: pending
  - id: ref-impl-engine
    content: "Fill reference_impl_engine.cc: ExecuteQuery runs Algebrizer+Evaluator, streams rows"
    status: pending
isProject: false
---

# Phase 5a: Ref Impl Adapter

## Prerequisites

- [dryrun-gateway-e2e_w8f9a0b1.plan.md](dryrun-gateway-e2e_w8f9a0b1.plan.md)

## Verification

```bash
ctest --test-dir build-out -R reference_impl || cmake --build build-out
```

## Done criteria

- Reference impl engine executes SELECT 1 in C++ unit test or manual grpc

## Next plan(s)

- [execute-query-stream_y0b1c2d3.plan.md](execute-query-stream_y0b1c2d3.plan.md)
