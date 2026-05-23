---
name: grpc-proto-codegen
overview: "Phase 2a: finalize proto/emulator.proto and generate Go + C++ gRPC bindings."
todos:
  - id: proto-finalize
    content: "Review proto/emulator.proto; consistent go_package and C++ package names"
    status: completed
  - id: buf-or-protoc-go
    content: "Add buf.gen.yaml or Makefile target generating gateway/enginepb/ from proto"
    status: completed
  - id: protoc-cpp
    content: "Add CMake rule for C++ grpc stubs; link grpc++ in CMakeLists.txt"
    status: pending
isProject: false
---

# Phase 2a: Grpc Proto Codegen

## Prerequisites

- [bootstrap-ci_e0f1a2b3.plan.md](bootstrap-ci_e0f1a2b3.plan.md)

## Verification

```bash
go build ./gateway/... && cmake --build build-out --target emulator_main
```

## Done criteria

- Go and C++ codegen compile in CI

## Next plan(s)

- [grpc-cpp-server_i4d5e6f7.plan.md](grpc-cpp-server_i4d5e6f7.plan.md)
