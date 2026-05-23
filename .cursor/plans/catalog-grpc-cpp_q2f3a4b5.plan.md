---
name: catalog-grpc-cpp
overview: "Phase 3g: Catalog gRPC handlers delegating to Storage."
todos:
  - id: catalog-handlers
    content: "Implement catalog.cc: RegisterDataset, DropDataset, RegisterTable, DropTable, DescribeTable; map errors to grpc Status"
    status: pending
  - id: catalog-errors
    content: "Map Storage NotFound→NOT_FOUND, AlreadyExists→ALREADY_EXISTS consistently"
    status: pending
isProject: false
---

# Phase 3g: Catalog Grpc Cpp

## Prerequisites

- [engine-cli-scaffold_m8b9c0d1.plan.md](engine-cli-scaffold_m8b9c0d1.plan.md)
- [grpc-gateway-client_j5e6f7a8.plan.md](grpc-gateway-client_j5e6f7a8.plan.md)

## Verification

```bash
grpcurl -plaintext localhost:9060 list
```

## Done criteria

- Catalog RPCs callable (may return real data after REST wiring)

## Next plan(s)

- [catalog-rest-go_r3a4b5c6.plan.md](catalog-rest-go_r3a4b5c6.plan.md)
