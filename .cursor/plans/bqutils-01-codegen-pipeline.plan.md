---
name: BQUtils 01 — Codegen pipeline (extractor + generator + sync + task)
overview: Build the sync+generate pipeline that converts bigquery-utils UDF .sqlx + Dataform test_cases.js into native conformance YAML fixtures, wired to the runner via a dedicated non-gating task. No engine changes; no triage run (that is plan 02).
depends_on: []
blocks: [bqutils-02-engine-build-triage, bqutils-07-stored-procedures, bqutils-08-views]
est_effort: 1-2 days
isProject: true
todos:
  - id: extractor
    content: "Write scripts/bigquery_utils/extract_test_cases.js: vm-sandbox eval of each test_cases.js with a unit_test_utils stub, read+strip matching .sqlx, classify/filter to pure-SQL scalar UDFs, emit a JSON manifest (with skip reasons)."
    status: pending
  - id: generator
    content: "Write conformance/cmd/genbqutils/main.go: read the JSON manifest, build runner.Fixture values (reuse conformance/runner structs), emit per-UDF YAML with the TO_JSON_STRING null-safe comparator + provenance header under conformance/thirdparty-fixtures/bigquery_utils/."
    status: pending
  - id: sync-script
    content: "Write scripts/sync_bigquery_utils_udfs.sh mirroring scripts/sync_python_bigquery_tests.sh: BIGQUERY_UTILS_REF/REPO/LOCAL knobs, --dry-run, shallow clone or local clone, resolve+print SHA, pipe extractor into generator, print emitted/skipped summary."
    status: pending
  - id: task-wiring
    content: Add conformance:bqutils-sync and conformance:bqutils targets to taskfiles/conformance.yml, separate from the gating conformance:run.
    status: pending
  - id: layout
    content: Establish conformance/thirdparty-fixtures/bigquery_utils/{passing,known_failing}/<family>/ output layout and .gitignore for any temp clone artifacts.
    status: pending
---

# BQUtils 01 — Codegen pipeline

## Goal

Stand up the deterministic conversion pipeline only. Output: generated fixtures on disk + a runnable (but not-yet-triaged) `task conformance:bqutils`. Building the engine and partitioning passing/failing is **plan 02**.

## Upstream format (confirmed)

- `udfs/tests/dataform_testing_framework/includes/unit_test_utils.js:15-33` — `generate_udf_test(udf_name, test_cases)` builds `SELECT <input_i> AS test_input_i`, invokes `udf_name(test_input_0, ...)`, compares to `SELECT <expected_output> AS udf_output`.
- `udfs/migration/teradata/test_cases.js:15-30` — `const { generate_udf_test } = unit_test_utils;` then `generate_udf_test("nullifzero", [{ inputs:[`CAST(-1 AS INT64)`], expected_output:`CAST(-1 AS INT64)` }, ...])`. No `require` of the helper — `unit_test_utils` is a global, so a sandbox global suffices.
- `udfs/community/test_cases.js:14` — also destructures `generate_udaf_test` (UDAF; **dropped** here, see plan 05).

## 1. Node extractor — `scripts/bigquery_utils/extract_test_cases.js`

- Walk `${SRC}/udfs/community/` and `${SRC}/udfs/migration/*/`.
- For each `test_cases.js`: read file text, run in a `node:vm` context whose sandbox defines `unit_test_utils = { generate_udf_test: capture, generate_udaf_test: captureUdaf }`, plus no-op stubs for anything else the file might touch (`publish`, `ctx`, `dataform`, `uuidv4`). Capture `{ family, name, kind, inputs[], expected_output }` per case (group repeated `generate_udf_test("name", ...)` calls under one name).
- For each captured `name`, read sibling `<name>.sqlx`; strip the leading `config { ... }` block and `OPTIONS(...)`; substitute `${self()}` -> bare `name`.
- **Classify & filter** — emit a UDF only if its body: (a) is not `LANGUAGE js`; (b) has no remaining `${...}` Dataform templating (`${ref(...)}`, `${dataform.projectConfig...}`); (c) came from `generate_udf_test` (not `generate_udaf_test`); (d) is not in a hardcoded exclude list (`exif*`, `OBJ.MAKE_REF`/GCS, BQML, remote). Otherwise record `{ name, reason }` in a `skipped` array.
- Emit one JSON manifest (`{ source_sha, emitted: [...], skipped: [...] }`) to stdout or `--out PATH`.

## 2. Go generator — `conformance/cmd/genbqutils/main.go`

- Read the JSON manifest.
- Per emitted UDF, build a `runner.Fixture` (import the structs from `conformance/runner/fixture.go` so schema can't drift):
  - `name: bqutils_<family>_<name>` (sanitized), `project_id: proj-bqutils-<name>`, `profiles: [duckdb]`.
  - `setup: [{ sql: <CREATE FUNCTION ...> }]`.
  - `query:` a `WITH cases AS (SELECT <i> AS case_id, TO_JSON_STRING(<fn>(<inputs_i>)) AS actual, TO_JSON_STRING(<expected_i>) AS expected UNION ALL ...) SELECT case_id, actual = expected AS matches FROM cases ORDER BY case_id`.
  - `expected: { match: ordered, rows: [{case_id:"i", matches:true}, ...] }`.
- Comparator rationale: `TO_JSON_STRING` is null-safe (`TO_JSON_STRING(NULL)='null'`) and uniform across scalar/array/struct, so one path covers every case kind. Per-case rows pinpoint the failing `case_id`.
- Prepend a provenance/license header comment (`# Source: GoogleCloudPlatform/bigquery-utils @ <SHA> / <upstream path> / License: Apache-2.0. Generated; do not edit.`).
- Idempotent: wipe + rewrite the output tree each run (diff-bounded refreshes).
- Initially write everything under `.../bigquery_utils/known_failing/<family>/` (plan 02 promotes passers to `passing/`).

### Generated fixture example

```yaml
# Source: GoogleCloudPlatform/bigquery-utils @ <SHA>
#   udfs/migration/teradata/nullifzero.sqlx (+ test_cases.js)
# License: Apache-2.0. Generated by scripts/sync_bigquery_utils_udfs.sh; do not edit by hand.
name: bqutils_migration_teradata_nullifzero
description: bigquery-utils teradata UDF nullifzero (3 cases)
profiles: [duckdb]
project_id: proj-bqutils-nullifzero
setup:
  - sql: |
      CREATE FUNCTION nullifzero(expr ANY TYPE) AS (
        IF(CAST(expr AS INT64) = 0, NULL, expr)
      )
query: |
  WITH cases AS (
    SELECT 0 AS case_id, TO_JSON_STRING(nullifzero(CAST(-1 AS INT64))) AS actual, TO_JSON_STRING(CAST(-1 AS INT64)) AS expected UNION ALL
    SELECT 1, TO_JSON_STRING(nullifzero(CAST(1 AS INT64))),  TO_JSON_STRING(CAST(1 AS INT64)) UNION ALL
    SELECT 2, TO_JSON_STRING(nullifzero(CAST(0 AS INT64))),  TO_JSON_STRING(CAST(NULL AS INT64))
  )
  SELECT case_id, actual = expected AS matches FROM cases ORDER BY case_id
expected:
  match: ordered
  rows:
    - {case_id: "0", matches: true}
    - {case_id: "1", matches: true}
    - {case_id: "2", matches: true}
```

## 3. Sync wrapper — `scripts/sync_bigquery_utils_udfs.sh`

Mirror `scripts/sync_python_bigquery_tests.sh` ergonomics:
- Env: `BIGQUERY_UTILS_REF` (default `master`), `BIGQUERY_UTILS_REPO` override, `BIGQUERY_UTILS_LOCAL=/home/brighten-tompkins/Code/bigquery-utils` to reuse the existing clone instead of cloning; `--dry-run`.
- Shallow `git fetch --depth=1` into a temp dir (or use the local clone), resolve + print the upstream SHA, run the Node extractor piped into the Go generator, print emitted-vs-skipped counts + skip reasons.

## 4. Task wiring — [taskfiles/conformance.yml](../../taskfiles/conformance.yml)

- `conformance:bqutils-sync` -> runs `scripts/sync_bigquery_utils_udfs.sh`.
- `conformance:bqutils` -> `go run ./conformance/cmd/runner --fixtures conformance/thirdparty-fixtures/bigquery_utils/passing --engine-binary $EMULATOR_BIN`. Keep it **out** of the gating `conformance:run` default so upstream UDFs the engine can't handle never break the PR gate.

## 5. Output layout

```
conformance/thirdparty-fixtures/bigquery_utils/
  passing/<family>/<name>.yaml        # populated by plan 02
  known_failing/<family>/<name>.yaml  # default codegen target
```

`<family>` = `migration/<dialect>` or `community`. Add `.gitignore` entries for any temp-clone scratch the script may leave.

## Verify (this plan)

```bash
node scripts/bigquery_utils/extract_test_cases.js --out /tmp/bqutils.json   # JSON manifest, sane counts
task conformance:bqutils-sync BIGQUERY_UTILS_LOCAL=/home/brighten-tompkins/Code/bigquery-utils
go vet ./conformance/cmd/genbqutils
ls conformance/thirdparty-fixtures/bigquery_utils/known_failing/migration/teradata/
```

Expect: extractor emits a JSON manifest with `emitted`/`skipped`; generator writes valid YAML that round-trips through `runner` YAML loading (no `KnownFields` errors). Do **not** require any fixture to pass yet.

## Out of scope (this plan)

- Building the engine / running the fixtures / partitioning passing vs failing (plan 02).
- Any engine feature work (plans 03–06).
- Promotion into the gating `conformance:run` (plan 09).
