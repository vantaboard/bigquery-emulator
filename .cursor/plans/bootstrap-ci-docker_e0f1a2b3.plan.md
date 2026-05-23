---
name: bootstrap-ci-docker
overview: "Complete Phase 0 remainder: reproducible Dockerfile/devcontainer and CI that builds gateway_main + emulator_main on linux/amd64 and arm64, runs go vet/test, and smoke-compiles the C++ stub."
todos:
  - id: dockerfile
    content: "Add Dockerfile multi-stage build: stage 1 builds gateway_main (Go 1.25+), stage 2 builds emulator_main (clang++ or gcc via CMake), final image ships both binaries + LICENSE"
    status: pending
  - id: devcontainer
    content: "Add .devcontainer/devcontainer.json referencing the Dockerfile; install task, go, cmake, clang; forward ports 9050 (HTTP) and 9060 (engine gRPC)"
    status: pending
  - id: ci-workflow
    content: "Add .github/workflows/ci.yml: matrix {linux/amd64, linux/arm64}, steps: go vet, go test ./..., cmake build emulator_main, go build ./binaries/gateway_main"
    status: pending
  - id: taskfile-ci
    content: "Add task ci:run to Taskfile.yml that mirrors the CI steps locally"
    status: pending
  - id: readme-docker
    content: "Document docker build/run in README.md Quickstart section"
    status: pending
isProject: false
---

# Phase 0 (remainder): Docker + CI

## Prerequisites

- Phase 0 scaffold already landed (`gateway_main`, `emulator_main` stub, Taskfile).

## Scope

**In:** Dockerfile, devcontainer, GitHub Actions CI, local `task ci:run`.

**Out:** Publishing to GHCR (Phase 13). Linking GoogleSQL in C++ build (Phase 3+).

## Key files

- `Dockerfile` (new)
- `.devcontainer/devcontainer.json` (new)
- `.github/workflows/ci.yml` (new)
- `Taskfile.yml` — add `ci:run` task
- `README.md` — docker quickstart

## Implementation notes

- Mirror `cloud-spanner-emulator` release layout: both `gateway_main` and `emulator_main` in the same directory inside the image.
- C++ build uses existing `CMakeLists.txt` (stub only — no GoogleSQL yet).
- CI should **not** require Bazel or a GoogleSQL checkout for this plan.
- Pin base images (`golang:1.25-bookworm`, `debian:bookworm-slim` or similar).

## Verification

```bash
docker build -t bigquery-emulator:local .
docker run --rm -p 9050:9050 bigquery-emulator:local \
  /gateway_main --engine_binary="" --http_port=9050 &
curl -sf http://localhost:9050/healthz
task ci:run   # or run CI steps manually
```

## Done criteria

- CI green on push for both architectures (or amd64-only if arm64 runner unavailable — document the limitation).
- `docker build` produces a runnable image with health check passing.
- Plan todos all `completed`.

## Next plan

[grpc-contract-go-cpp_8a9b0c1d.plan.md](grpc-contract-go-cpp_8a9b0c1d.plan.md) — can run in parallel with [gateway-polish_c4d5e6f7.plan.md](gateway-polish_c4d5e6f7.plan.md).
