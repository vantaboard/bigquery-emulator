---
name: grpc-gateway-client
overview: "Phase 2c: Go gRPC client, health-based readiness, integration smoke test."
todos:
  - id: gateway-client
    content: "Add gateway/engine/client.go; expose CatalogClient and QueryClient on handlers.Dependencies"
    status: completed
  - id: wait-for-ready
    content: "Replace waitForEngine sleep with grpc.health.v1 Check loop (30s timeout)"
    status: completed
  - id: integration-test
    content: "Add gateway_test.go or smoke script: gateway connects to emulator_main, health passes"
    status: completed
isProject: false
---

# Phase 2c: Grpc Gateway Client

## Prerequisites

- [grpc-cpp-server_i4d5e6f7.plan.md](grpc-cpp-server_i4d5e6f7.plan.md)

## Verification

```bash
go test ./gateway/...
```

## Done criteria

- No 200ms sleep in waitForEngine
- Integration test passes

## Next plan(s)

- [cpp-interfaces_k6f7a8b9.plan.md](cpp-interfaces_k6f7a8b9.plan.md)
- [vendor-duckdb_n9c0d1e2.plan.md](vendor-duckdb_n9c0d1e2.plan.md)
