---
name: BQUtils 02 — Engine build, generate, triage
overview: Build emulator_main, generate the bigquery-utils fixtures, run the full set, and partition into passing/ vs known_failing/. Land a green non-gating conformance:bqutils on the passing set. Document the new lane.
depends_on: [bqutils-01-codegen-pipeline]
blocks: [bqutils-03-any-type-udfs, bqutils-04-js-udfs, bqutils-05-sql-udaf, bqutils-06-tvf, bqutils-07-stored-procedures, bqutils-08-views, bqutils-09-promote-to-gate]
est_effort: 1-2 days
isProject: true
todos:
  - id: build-engine
    content: Build ./bin/emulator_main via task emulator:build-engine:bazel, following bazel/process hygiene rules (single invocation, throttled jobs, bazel:status/bazel:shutdown after).
    status: pending
  - id: generate
    content: Run task conformance:bqutils-sync against the local clone to (re)generate the full fixture set under known_failing/.
    status: pending
  - id: triage
    content: Run the full generated set with --output json; partition fixtures into passing/ (move) vs known_failing/ (leave) based on per-fixture verdict.
    status: pending
  - id: green-lane
    content: Confirm task conformance:bqutils is green on the passing/ set; capture counts (ran/passed/skipped-at-codegen/failed).
    status: pending
  - id: docs
    content: Document the lane in third_party/README.md (provenance, sync command, gating policy) and note the task in conformance docs; record the baseline counts.
    status: pending
---

# BQUtils 02 — Engine build, generate, triage

## Goal

Turn the plan-01 pipeline into a real, green, non-gating lane. Establish the first baseline of how many real-world bigquery-utils UDFs the engine handles today, and partition the rest into `known_failing/` for the engine-feature plans (03–06) to chip away at.

## 1. Build the engine

`./bin/emulator_main` is the `--engine-binary` the runner spawns per fixture (default `EMULATOR_BIN=./bin/emulator_main` in `taskfiles/conformance.yml`).

### Prebuilt vs environment (read before building)

1. Run `task googlesql:status`.
2. If `./bin/emulator_main` exists and executes, **skip the rebuild** for this plan unless you changed engine/C++ sources — triage only needs a working binary.
3. If you must build:
   - **Empty prebuilt cache** (preflight: "cache is empty") → **not a broken artifact**. Either:
     - `task googlesql:fetch-prebuilt` with `RELEASE_GOOGLESQL_PREBUILT_URL` / `RELEASE_GOOGLESQL_PREBUILT_SHA256` from [`.github/workflows/release.yml`](../../.github/workflows/release.yml), then `task emulator:build-engine:bazel`, or
     - `GOOGLESQL_SOURCE=local task emulator:build-engine:bazel` when `../googlesql/MODULE.bazel` exists.
   - **`FAIL_*` from `task googlesql:validate`** → corrupt/wrong pin; refetch/repin per [`troubleshooting.md`](../../docs/dev/googlesql-prebuilt/troubleshooting.md).
   - Do **not** infer a broken prebuilt from missing `googlesql/public/functions` under the cache (wrong layout expectation).

```bash
task emulator:build-engine:bazel   # only when no usable bin/emulator_main
```

This is a heavy GoogleSQL-linked C++ build. **Follow** [`.cursor/rules/bazel-process-hygiene.mdc`](../rules/bazel-process-hygiene.mdc) and [`.cursor/rules/process-hygiene.mdc`](../rules/process-hygiene.mdc):
- Pre-spawn audit; single invocation (never stack a second bazel build).
- Throttled jobs/memory (the `task bazel:*` helpers auto-detect).
- After: `task bazel:status` (expect `(clean)`) and `task bazel:shutdown`.

## 2. Generate

```bash
task conformance:bqutils-sync BIGQUERY_UTILS_LOCAL=/home/brighten-tompkins/Code/bigquery-utils
```

Everything lands under `conformance/thirdparty-fixtures/bigquery_utils/known_failing/` from plan 01's codegen default.

## 3. Triage (partition passing vs failing)

Run the whole generated tree and capture machine-readable verdicts:

```bash
go run ./conformance/cmd/runner \
  --fixtures conformance/thirdparty-fixtures/bigquery_utils/known_failing \
  --engine-binary ./bin/emulator_main \
  --output json --output-file /tmp/bqutils-triage.json
```

For each `PASS` fixture, move the YAML from `known_failing/<family>/` to `passing/<family>/`. Leave `FAIL` fixtures in `known_failing/`. A small helper (script or one-liner driven by the JSON report) is fine; keep it reproducible.

Expected reality from the engine baseline:
- Pure-SQL scalar UDFs with concrete typed params -> mostly **PASS**.
- `ANY TYPE` UDFs (e.g. `nullifzero`, `nvl`) -> may FAIL (templated path untested) -> plan 03.
- Anything depending on dropped families already filtered at codegen (JS/UDAF/TVF) won't appear here.

## 4. Green non-gating lane

```bash
task conformance:bqutils    # runs only passing/
```

Must exit 0. Record the baseline counts: total emitted, passing, known_failing, and skipped-at-codegen (from the extractor's `skipped` array).

## 5. Docs

- Extend [third_party/README.md](../../third_party/README.md) with a "bigquery-utils UDF conformance" section: provenance (repo + SHA + Apache-2.0), the `conformance:bqutils-sync`/`conformance:bqutils` commands, the passing/known_failing split, and the gating policy (non-gating until plan 09).
- Note the new task in the conformance docs/README under `conformance/`.
- Record the baseline counts so later plans can show movement.

## Verify

```bash
task conformance:bqutils                      # green
ls conformance/thirdparty-fixtures/bigquery_utils/passing
ls conformance/thirdparty-fixtures/bigquery_utils/known_failing
```

## Out of scope (this plan)

- Engine feature work to convert `known_failing` -> `passing` (plans 03–06).
- stored_procedures / views corpus (plans 07–08).
- Gate promotion (plan 09).
