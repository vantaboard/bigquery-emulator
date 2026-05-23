---
name: grpc-contract-go-cpp
overview: "Phase 2: wire up proto/emulator.proto end-to-end — generate Go and C++ bindings, implement grpc.health.v1 on the engine, replace gateway waitForEngine sleep with a real readiness probe, and add a minimal Catalog/Query stub server in frontend/."
todos:
  - id: proto-finalize
    content: "Review proto/emulator.proto against ROADMAP Phase 2; add any missing rpc options; ensure go_package and C++ package names are consistent"
    status: pending
  - id: buf-or-protoc-go
    content: "Add buf.gen.yaml (or Makefile target) generating Go bindings into gateway/enginepb/ from proto/emulator.proto + google/api annotations if needed"
    status: pending
  - id: protoc-cpp
    content: "Add CMake/Bazel rule generating C++ grpc stubs into frontend/proto/ or build/gen/; link grpc++ in CMakeLists.txt"
    status: pending
  - id: health-service
    content: "Implement grpc.health.v1.Health on emulator_main; Check returns SERVING when gRPC server is listening"
    status: pending
  - id: catalog-query-stubs
    content: "Implement frontend/handlers/catalog.cc and query.cc as UNIMPLEMENTED stubs returning grpc::StatusCode::UNIMPLEMENTED for every rpc"
    status: pending
  - id: real-grpc-server
    content: "Replace frontend/server/server.cc StubServer with real grpc::ServerBuilder on --host_port; register Catalog, Query, Health services"
    status: pending
  - id: gateway-client
    content: "Add gateway/engine/client.go wrapping grpc.Dial to engine; expose CatalogClient and QueryClient on handlers.Dependencies"
    status: pending
  - id: wait-for-ready
    content: "Replace gateway/gateway.go waitForEngine sleep with grpc.health.v1 Check loop (30s timeout, cf. cloud-spanner-emulator waitForReady)"
    status: pending
  - id: integration-test
    content: "Add gateway/gateway_test.go or scripts/smoke_grpc.sh: start emulator_main, gateway connects and health check passes"
    status: pending
isProject: false
---

# Phase 2: Internal gRPC contract (Go ↔ C++)

## Prerequisites

- [bootstrap-ci-docker_e0f1a2b3.plan.md](bootstrap-ci-docker_e0f1a2b3.plan.md) recommended (CI validates codegen).
- Existing [proto/emulator.proto](../../proto/emulator.proto) v0 sketch.

## Scope

**In:** Codegen, real gRPC server skeleton, Go client, health-based readiness.

**Out:** Catalog/Query business logic (Phases 03–07). GoogleSQL linkage.

## Key files

- `proto/emulator.proto`
- `buf.gen.yaml` or `Makefile` codegen targets
- `gateway/enginepb/` — generated Go
- `gateway/engine/client.go` (new)
- `gateway/gateway.go` — `waitForReady`
- `frontend/server/server.cc` — real gRPC server
- `frontend/handlers/catalog.{h,cc}`, `query.{h,cc}` (new)
- `CMakeLists.txt` — grpc/protobuf deps

## Reference

- `cloud-spanner-emulator/gateway/gateway.go` — `waitForReady` pattern (ListInstanceConfigs probe); we use health check instead.
- Spanner subprocess spawn in [gateway/gateway.go](../../gateway/gateway.go) — already mirrors upstream.

## Verification

```bash
cmake -S . -B build-out && cmake --build build-out --target emulator_main
./build-out/emulator_main --host_port localhost:9060 &
grpc_health_probe -addr=localhost:9060   # or grpcurl
go build -o bin/gateway_main ./binaries/gateway_main
./bin/gateway_main --engine_binary=./build-out/emulator_main --http_port=9050
# gateway logs "Engine gRPC expected at localhost:9060" without fatal
go test ./gateway/...
```

## Done criteria

- No more 200ms sleep in `waitForEngine`; health check passes or gateway fatals with clear error.
- Go handlers can obtain a `CatalogClient` (even if all RPCs return Unimplemented).
- CI builds C++ with gRPC linked.

## Next plans

- [cpp-interfaces-memory-storage_2e3f4a5b.plan.md](cpp-interfaces-memory-storage_2e3f4a5b.plan.md)
- [duckdb-persistent-storage_6c7d8e9f.plan.md](duckdb-persistent-storage_6c7d8e9f.plan.md) (parallel)
