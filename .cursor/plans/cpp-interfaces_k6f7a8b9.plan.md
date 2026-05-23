---
name: cpp-interfaces
overview: "Phase 3a: Storage and Engine C++ interfaces plus internal schema types."
todos:
  - id: storage-interface
    content: "Replace storage.h with Storage abstract class: CreateDataset, DropDataset, CreateTable, DropTable, GetSchema, AppendRows, ScanRows"
    status: pending
  - id: engine-interface
    content: "Add backend/engine/engine.h: Analyze, DryRun, ExecuteQuery against resolved AST + catalog"
    status: pending
  - id: schema-types
    content: "Add backend/schema/schema.h: proto FieldSchema ↔ internal ColumnType conversions"
    status: pending
isProject: false
---

# Phase 3a: Cpp Interfaces

## Prerequisites

- [grpc-gateway-client_j5e6f7a8.plan.md](grpc-gateway-client_j5e6f7a8.plan.md)

## Verification

```bash
cmake --build build-out
```

## Done criteria

- Interfaces compile and link

## Next plan(s)

- [memory-storage_l7a8b9c0.plan.md](memory-storage_l7a8b9c0.plan.md)
