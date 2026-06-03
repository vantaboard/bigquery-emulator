# CI engine build pipeline

GitHub Actions builds the C++ engine (`emulator_main` + `libduckdb.so`) **once**
per push/PR in the [`build-engine`](../../.github/workflows/build-engine.yml)
workflow. Downstream lanes download that artifact instead of compiling GoogleSQL
again.

## Flow

```text
push / pull_request
        │
        ▼
 build-engine / build          ──► engine-binaries artifact
        │                         (bin/emulator_main, bin/libduckdb.so,
        │                          bin/gateway_main, engine-provenance.json)
        │ workflow_run (success)
        ├─► ci / build-and-test (amd64)
        ├─► conformance / conformance (duckdb)
        ├─► docker-smoke / quickstart-smoke
        └─► thirdparty-samples / java-bigquery-tests (live emulator)
```

Cheap jobs that do **not** need the engine stay on the direct `push` /
`pull_request` trigger:

- `ci / cpp-analysis (cppcheck)`
- `conformance / routing-matrix`
- `thirdparty-samples / golang-bigquery-tests (compile + skip)`

## Caching

| Layer | Where | Purpose |
|-------|--------|---------|
| `actions/cache` on `bin/` | `build-engine` | Skip Bazel when engine inputs + GoogleSQL pin unchanged across commits |
| Bazel `disk-cache: engine` | `build-engine`, `ci` cc_test | Shared incremental compile cache |
| `engine-binaries` artifact | per successful `build-engine` run | Consumers + re-runs download without rebuilding |

Re-running a failed consumer (e.g. conformance) on the same commit reuses the
artifact via [`.github/actions/setup-engine-from-artifact`](../../.github/actions/setup-engine-from-artifact/action.yml),
which locates the latest successful `build-engine` run for the SHA when no
explicit run id is passed.

## Branch protection

Add **`build-engine / build`** as a required status check alongside the existing
consumer checks:

- `ci / build-and-test (amd64)`
- `conformance / conformance (duckdb)`
- `docker-smoke / quickstart-smoke`
- `thirdparty-samples / java-bigquery-tests (live emulator)`

## Local development

[`task ci:run`](../../taskfiles/ci.yml) still builds the engine sequentially on
one machine (`task emulator:build-engine:bazel`). CI splits build and test
across workflows for runner efficiency; local dev keeps the single-process mirror.

Docker lanes locally continue to use `task docker:smoke` (full build). CI
docker-smoke sets `DOCKER_SMOKE_SKIP_BUILD=1` after loading a pre-built image
assembled with `ENGINE_SOURCE=prebuilt`.

## Out of scope

Tag releases ([`release.yml`](../../.github/workflows/release.yml)) build and
stamp their own engine with release metadata and remain independent of this
pipeline.
