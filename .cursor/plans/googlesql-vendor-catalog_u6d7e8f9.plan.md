---
name: googlesql-vendor-catalog
overview: "Phase 4a: vendor GoogleSQL and build googlesql catalog adapter over Storage."
todos:
  - id: vendor-googlesql
    content: "Add GoogleSQL Bazel dep or sibling ../googlesql; MODULE.bazel + local_path_override for dev"
    status: completed
  - id: analyzer-catalog
    content: "Implement googlesql_catalog.{h,cc}: googlesql::Table adapter over Storage for name resolution"
    status: completed
isProject: false
---

# Phase 4a: Googlesql Vendor Catalog

## Prerequisites

- [tabledata-e2e_t5c6d7e8.plan.md](tabledata-e2e_t5c6d7e8.plan.md)

## Verification

```bash
bazel build //frontend/... 2>/dev/null || cmake --build build-out
```

## Done criteria

- GoogleSQL linked or documented dev checkout
- Catalog adapter compiles

## Next plan(s)

- [dryrun-cpp-rpc_v7e8f9a0.plan.md](dryrun-cpp-rpc_v7e8f9a0.plan.md)
