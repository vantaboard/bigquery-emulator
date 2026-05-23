---
name: bootstrap-docker
overview: "Phase 0a: multi-stage Dockerfile and devcontainer for reproducible local builds."
todos:
  - id: dockerfile
    content: "Add Dockerfile multi-stage: build gateway_main (Go 1.25+), build emulator_main (CMake/clang), final image ships both + LICENSE"
    status: pending
  - id: devcontainer
    content: "Add .devcontainer/devcontainer.json: task, go, cmake, clang; forward ports 9050 and 9060"
    status: pending
isProject: false
---

# Phase 0a: Bootstrap Docker

## Verification

```bash
docker build -t bigquery-emulator:local . && docker run --rm -p 9050:9050 bigquery-emulator:local /gateway_main --engine_binary='' --http_port=9050
```

## Done criteria

- docker build succeeds
- healthz returns 200

## Next plan(s)

- [bootstrap-ci_e0f1a2b3.plan.md](bootstrap-ci_e0f1a2b3.plan.md)
