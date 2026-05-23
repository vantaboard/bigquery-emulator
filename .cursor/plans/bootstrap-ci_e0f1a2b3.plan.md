---
name: bootstrap-ci
overview: "Phase 0b: GitHub Actions CI, local task ci:run, and README docker quickstart."
todos:
  - id: ci-workflow
    content: "Add .github/workflows/ci.yml: matrix linux/amd64+arm64; go vet, go test ./..., cmake build emulator_main, go build gateway"
    status: pending
  - id: taskfile-ci
    content: "Add task ci:run to Taskfile.yml mirroring CI steps"
    status: pending
  - id: readme-docker
    content: "Document docker build/run in README Quickstart"
    status: pending
isProject: false
---

# Phase 0b: Bootstrap Ci

## Prerequisites

- [bootstrap-docker_a0b1c2d3.plan.md](bootstrap-docker_a0b1c2d3.plan.md)

## Verification

```bash
task ci:run
```

## Done criteria

- CI green on push
- task ci:run passes locally

## Next plan(s)

- [grpc-proto-codegen_h3c4d5e6.plan.md](grpc-proto-codegen_h3c4d5e6.plan.md)
- [gateway-auth-discovery_f1a2b3c4.plan.md](gateway-auth-discovery_f1a2b3c4.plan.md)
