---
name: grpc-cpp-server
overview: "Phase 2b: real gRPC server with health check and Catalog/Query UNIMPLEMENTED stubs."
todos:
  - id: health-service
    content: "Implement grpc.health.v1.Health; Check returns SERVING when listening"
    status: pending
  - id: catalog-query-stubs
    content: "Add catalog.cc and query.cc returning UNIMPLEMENTED for every rpc"
    status: pending
  - id: real-grpc-server
    content: "Replace StubServer with grpc::ServerBuilder; register Catalog, Query, Health"
    status: pending
isProject: false
---

# Phase 2b: Grpc Cpp Server

## Prerequisites

- [grpc-proto-codegen_h3c4d5e6.plan.md](grpc-proto-codegen_h3c4d5e6.plan.md)

## Verification

```bash
grpc_health_probe -addr=localhost:9060
```

## Done criteria

- Health check passes on emulator_main

## Next plan(s)

- [grpc-gateway-client_j5e6f7a8.plan.md](grpc-gateway-client_j5e6f7a8.plan.md)
