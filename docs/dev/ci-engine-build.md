# CI engine build pipeline

GitHub Actions builds the C++ engine (`emulator_main` + `libduckdb.so`) **once**
per push/PR in the [`build-engine`](../../.github/workflows/build-engine.yml)
workflow. Downstream lanes download that artifact instead of compiling GoogleSQL
again.

## Flow

```text
push / pull_request
        │
        ├─► build-engine / build          ──► engine-binaries artifact
        ├─► ci-cpp-analysis / cpp-analysis (parallel, no engine)
        ├─► conformance-routing-matrix / routing-matrix (parallel)
        └─► thirdparty-golang-compile / golang compile (parallel)
                │
                │ workflow_run (build-engine success)
                ├─► ci / build-and-test (amd64) ──► go-coverage artifact
                ├─► conformance / conformance (duckdb)
                ├─► docker-smoke / quickstart-smoke
                ├─► thirdparty-samples / java live
                ├─► thirdparty-samples / python nox snippets
                ├─► thirdparty-samples / node Mocha
                ├─► thirdparty-samples / golang live
                ├─► thirdparty-samples / bigframes snippet gate
                └─► coverage-bazel / bazel-coverage (main push only)
                        │
                        └─► coverage-publish (after ci or coverage-bazel)
```

Consumer workflows (`ci`, `conformance`, `docker-smoke`, `thirdparty-samples`)
are **`workflow_run` only**. They never run on push, so they cannot show green
with skipped gate jobs. If `build-engine` fails, each consumer runs an
`engine-build-failed` job that exits non-zero.

[`coverage-bazel`](../../.github/workflows/coverage-bazel.yml) is also
**`workflow_run` only** and runs **only on push to `main`** (not on PRs). PR
coverage gates use Go coverage from `ci` only; C++ Bazel coverage is
informational and runs post-merge.

Cheap push/PR workflows (no engine):

- [`ci-cpp-analysis.yml`](../../.github/workflows/ci-cpp-analysis.yml)
- [`conformance-routing-matrix.yml`](../../.github/workflows/conformance-routing-matrix.yml)
- [`thirdparty-golang-compile.yml`](../../.github/workflows/thirdparty-golang-compile.yml)

## Caching

| Layer | Where | Purpose |
|-------|--------|---------|
| `actions/cache` on `bin/` | `build-engine` | Skip Bazel when engine inputs + GoogleSQL pin unchanged **and** the cache key matches exactly |
| `actions/cache` on `.cache/googlesql-prebuilt/` | `build-engine`, `coverage-bazel` | Skip tarball download when prebuilt SHA256 pin unchanged |
| Bazel `disk-cache: engine` | `build-engine`, `coverage-bazel`, `ci` cc_test | Shared incremental compile cache (safe partial reuse) |
| `engine-binaries` artifact | per successful `build-engine` run | Consumers + re-runs download without rebuilding |

### `bin/` cache safety (exact hit only)

The staged `bin/` cache is an all-or-nothing shortcut: when `actions/cache`
reports `cache-hit: true` for the full key (currently prefixed
`engine-binaries-v2-…`), `build-engine` skips Bazel if
`./bin/emulator_main --version` succeeds.

**Do not use `restore-keys` prefix fallback on `bin/`.** A partial restore can
leave pre-change `emulator_main` binaries that still pass `--version`; a
subsequent run can save them under the exact key and ship stale engines into
conformance and thirdparty consumers (2026-06 incident: SIGINT teardown mutex
fatals). Incremental compile speed belongs in the Bazel `disk-cache: engine`
layer, not prefix-matched staged binaries.

When the cache key does not match exactly, the workflow removes any restored
`bin/emulator_main` / `bin/libduckdb.so` and runs
`task emulator:build-engine:bazel`. Bump the `engine-binaries-vN` prefix in
[`.github/workflows/build-engine.yml`](../../.github/workflows/build-engine.yml)
only when invalidating poisoned or incompatible cached `bin/` trees.

`engine-provenance.json` in the uploaded artifact records
`engine_binaries_cache_key_version`, `engine_binaries_cache_hit` (exact
`actions/cache` match), and `cache_hit` (whether Bazel compile was skipped).

Re-running a failed consumer (e.g. conformance) on the same commit reuses the
artifact via [`.github/actions/setup-engine-from-artifact`](../../.github/actions/setup-engine-from-artifact/action.yml),
which locates the latest successful `build-engine` run for the SHA when no
explicit run id is passed.

## Branch protection

**Engine pipeline (required):**

- `build-engine / build`
- `ci / build-and-test (amd64)`
- `conformance / conformance (duckdb)`
- `docker-smoke / quickstart-smoke`
- `thirdparty-samples / java-bigquery-tests (live emulator)`
- `thirdparty-samples / python-bigquery-tests (live emulator)` (plan 14; may be red until thirdparty-02–09 land)
- `thirdparty-samples / node-bigquery-tests (live emulator)` (plan 14)
- `thirdparty-samples / python-bigquery-dataframes snippet gate (live emulator)` (plan 14)
- `thirdparty-samples / golang-bigquery-tests (live emulator)`

**Fast lanes (optional but recommended):**

- `ci-cpp-analysis / cpp-analysis (cppcheck)`
- `conformance-routing-matrix / routing-matrix` (`continue-on-error: true` in workflow)
- `thirdparty-golang-compile / golang-bigquery-tests (compile + skip)`

If branch protection previously required `ci / cpp-analysis (cppcheck)`, update
it to **`ci-cpp-analysis / cpp-analysis (cppcheck)`** (workflow was split out).

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
