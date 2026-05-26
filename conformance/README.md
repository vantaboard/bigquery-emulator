# Conformance fixtures + runner

The conformance harness drives the BigQuery emulator through its REST
gateway with declarative YAML fixtures and diffs the resulting rows (or
errors) against the values pinned in each file. The harness is Phase 8
of `ROADMAP.md`; the runner CLI is the deliverable of plan 40
(`conformance-fixtures-runner`).

## Quick start

```bash
# 1. Build the C++ engine if you have not already.
task emulator:build-engine:bazel

# 2. Run every fixture in conformance/fixtures against both profiles,
#    spawning a fresh emulator per fixture × profile.
go run ./conformance/cmd/runner

# 3. Run a single fixture against a single profile, against an
#    emulator you have already booted on :9060.
go run ./conformance/cmd/runner \
  --fixtures conformance/fixtures/select_literal_value.yaml \
  --profile memory \
  --connect 127.0.0.1:9060

# 4. JSON output for CI consumption (plan 41 hooks this).
go run ./conformance/cmd/runner --output json
```

## Profile matrix

A fixture's `profiles:` list declares which engine + storage profile it
applies to. Two profiles are defined today:

| profile | engine          | storage | use for                                  |
|---------|-----------------|---------|------------------------------------------|
| memory  | reference_impl  | memory  | CI / unit-style assertions, fast startup |
| duckdb  | duckdb          | duckdb  | OLAP shapes, DDL surface, Parquet store  |

A fixture that omits `profiles:` runs on **both** profiles. A fixture
that lists exactly one profile (e.g. a DuckDB-only feature) runs only
against that profile. The runner iterates the matrix and tags every
result with its profile in both `text` and `json` output modes, so a
divergence between profiles is visible without manual cross-referencing.

The DuckDB profile passes `--on_unknown_fn=fallback` so DDL and DML
shapes the DuckDB transpiler does not lower yet are retried against the
reference-impl engine. The memory profile leaves `--on_unknown_fn`
unset (default `unimplemented`), so any UNIMPLEMENTED surface lands as
a structured 501 on the wire.

## Fixture schema

Every fixture is a single YAML document with the following fields:

```yaml
name: insert_then_select          # required, identifier (matches filename)
description: |                    # optional, human prose for the diff log
  Catalog mutation + INSERT VALUES + SELECT round-trip.
profiles: [memory, duckdb]        # optional; omitted = both
project_id: proj-conformance      # optional, defaults to "proj-conformance-<name>"
dataset_id: ds_conformance        # optional, hint only; not auto-created
setup:                            # optional, runs in order before `query`
  - dataset: ds_conformance       # REST POST /datasets
  - table:                        # REST POST /datasets/<id>/tables
      dataset: ds_conformance
      id: people
      schema:
        - {name: id, type: INT64, mode: REQUIRED}
        - {name: name, type: STRING, mode: NULLABLE}
  - sql: INSERT INTO ds_conformance.people (id, name) VALUES (1, 'a')
query: SELECT id, name FROM ds_conformance.people ORDER BY id   # required
expected:                         # exactly one of `rows:` or `error:`
                                  # (schema_only mode may set neither)
  match: ordered                  # optional; ordered (default) | unordered | schema_only
  schema:                         # optional; required for match=schema_only
    - {name: id, type: INT64}
    - {name: name, type: STRING}
  rows:
    - {id: "1", name: "a"}
  # OR
  error:
    code: 400
    message_contains: "Unrecognized name"
```

### `setup` step kinds

The `setup` list is dispatched by which field is present in each step:

1. `dataset: <datasetId>` — `POST /bigquery/v2/projects/<projectId>/datasets`
   with a minimal `{datasetReference, location:"US"}` body. Use this to
   create a dataset before issuing DML against it.
2. `table:` — `POST /bigquery/v2/projects/<projectId>/datasets/<datasetId>/tables`
   with `{tableReference, schema:{fields:[...]}}`. The `schema:` list
   maps directly to BigQuery's `TableFieldSchema` shape (`name`, `type`,
   `mode`, plus nested `fields` for `STRUCT`).
3. `sql: <query>` — `POST /bigquery/v2/projects/<projectId>/queries`
   with `{query, useLegacySql:false}`. Use this for `INSERT`,
   `UPDATE`, `DELETE`, `CREATE TABLE AS SELECT`, etc. The setup step
   fails the fixture if the gateway responds with a non-2xx status.

A fixture that needs only a `SELECT` (with no catalog state) omits
`setup` entirely.

### `expected.match` matching modes

Three matching modes are supported. The mode is declared on
`expected.match`; omitting it defaults to `ordered` (the plan-40
default, kept stable for the seed fixtures).

| mode | what it pins | when to use |
|------|--------------|-------------|
| `ordered` *(default)* | row[i] ↔ actual[i], cell-by-cell, typed compare | queries with `ORDER BY` |
| `unordered` | multiset equality after type-aware canonicalization | engines that do not guarantee row order, parallel scans |
| `schema_only` | column names + types only; row values ignored | non-deterministic data (timestamps, generated IDs), dryRun smoke checks |

`ordered` and `unordered` both require `rows:` (or `error:`).
`schema_only` requires `schema:` (preferred) or `rows:` (the first
row's keys are used as the expected column-name set, with type
checks skipped).

#### Typed cell comparison

Cells are compared against the column's SQL type taken from the
gateway's `QueryResponse.schema`. This means a fixture writing
`id: 1` (decoded by YAML as an `int`) still matches the gateway's
wire encoding `"1"` (a string). Per-type rules:

| BigQuery type | comparison |
|---------------|------------|
| `INT64` / `INTEGER` / `NUMERIC` / `BIGNUMERIC` | exact rational equality (`math/big.Rat`) |
| `FLOAT64` / `FLOAT` | relative epsilon of `1e-9` (NaN compares equal to NaN for diff purposes) |
| `BOOL` / `BOOLEAN` | normalize `true` / `false` / `t` / `f` / `1` / `0` (case-insensitive) before compare |
| `STRING` / `BYTES` | literal byte equality on the canonical string form |
| `TIMESTAMP` / `DATETIME` / `DATE` / `TIME` | parse both sides into `time.Time` (RFC3339, SQL form, or Unix-seconds-as-string) and compare for instant equality |
| `STRUCT` / `REPEATED` / unknown | fall back to JSON-serialized string compare |

NULL is distinct from the literal string `"NULL"`: a YAML `null`
expects a NULL cell (`{"v":null}` on the wire) and refuses to match
a cell whose string value happens to be `"NULL"`. NULL on either
side without NULL on the other is always a mismatch.

#### Unordered mode caveats

The unordered diff buckets rows by their canonicalized one-line
form (sorted by column name, with type normalization applied per
the table above). For float columns the canonicalizer rounds to 12
significant digits, so values within ~1e-12 relative tolerance
still bucket together; tighter float tolerances need `ordered`
mode. Failure output lists every multiset element plus an explicit
`missing` (expected-only) / `extra` (actual-only) breakdown so the
diverging row is visible without manual subtraction.

#### Schema-only mode

`schema_only` validates the gateway's returned `schema.fields[]`
positionally against `expected.schema`. The `Type` field is matched
case-insensitively (`INTEGER` matches `INT64`). If `expected.schema`
is omitted and only `expected.rows[0]` is present, the runner uses
that row's keys as the expected column-name set and skips the type
check.

The diff output on failure is a unified text diff between the
YAML's `expected.rows` (or `schema:`) and the actual rows / schema
as the runner observed them, so a human can pin which column / row
diverged at a glance.

### `expected.error` matching

`expected.error.code` is matched exactly against the HTTP status code
the gateway returned. `expected.error.message_contains` is matched as a
substring against the BigQuery error envelope's `error.message` field
(and, as a fallback, the first `error.errors[].message`). Either field
may be omitted; the runner only asserts on what the fixture pins.

### Worked example

The fixture below seeds three rows via the REST catalog + INSERT
VALUES, mutates id=1 and deletes id=2, then asserts the post-mutation
SELECT returns the two remaining rows in `id` order:

```yaml
name: dml_update_delete
profiles: [memory, duckdb]
project_id: proj-dml-update-delete
dataset_id: ds_dml_update_delete
setup:
  - dataset: ds_dml_update_delete
  - table:
      dataset: ds_dml_update_delete
      id: people
      schema:
        - {name: id, type: INT64, mode: REQUIRED}
        - {name: name, type: STRING, mode: NULLABLE}
  - sql: |
      INSERT INTO ds_dml_update_delete.people (id, name)
      VALUES (1, 'ada'), (2, 'linus'), (3, 'grace')
  - sql: |
      UPDATE ds_dml_update_delete.people
      SET name = 'augusta'
      WHERE id = 1
  - sql: |
      DELETE FROM ds_dml_update_delete.people
      WHERE id = 2
query: |
  SELECT id, name FROM ds_dml_update_delete.people ORDER BY id
expected:
  rows:
    - {id: "1", name: "augusta"}
    - {id: "3", name: "grace"}
```

## Runner CLI

```
go run ./conformance/cmd/runner --help

Flags:
  --fixtures PATH         Directory or single .yaml file. Default
                          conformance/fixtures.
  --engine-binary PATH    Path to emulator_main. Default ./bin/emulator_main.
                          Mutually exclusive with --connect.
  --connect HOST:PORT     Reach an already-running gateway instead of
                          spawning emulator_main. The connected gateway
                          is reused for every fixture; the runner does
                          not change its --engine / --storage at runtime,
                          so use --profile to restrict which fixtures run.
  --profile NAME          Repeatable. Restrict the matrix to one or more
                          named profiles (memory, duckdb). Default: all.
  --update-baselines      Overwrite the `expected:` block of every fixture
                          with the actual response. Used to bootstrap new
                          fixtures (plan 42 leans on this).
  --output FORMAT         text (human, default) or json (machine,
                          consumed by plan 41 CI).
```

Exit codes:

- `0` — every fixture × profile PASSed.
- `1` — at least one fixture × profile FAILed.
- `2` — runner-internal error (bad YAML, can't start engine, etc).

### JSON output shape (consumed by plan 41)

```json
{
  "schema_version": 1,
  "summary": {
    "total": 8,
    "passed": 8,
    "failed": 0,
    "skipped": 0
  },
  "results": [
    {
      "fixture": "select_literal_value",
      "path": "conformance/fixtures/select_literal_value.yaml",
      "profile": "memory",
      "status": "PASS",
      "duration_ms": 142,
      "message": ""
    }
  ]
}
```

`status` is one of `PASS`, `FAIL`, `SKIP`. `message` carries the
short-form reason; `diff` (only present on `FAIL`) carries the unified
diff of expected vs actual.

## Adding fixtures

1. Drop a new `<name>.yaml` under `conformance/fixtures/`. Keep the
   filename and the `name:` field in sync.
2. Decide which profile(s) the fixture applies to and list them in
   `profiles:`. Default to both unless you know one profile cannot
   support the shape (e.g. a Parquet-only storage feature).
3. Run `go run ./conformance/cmd/runner --fixtures conformance/fixtures/<name>.yaml`
   to verify the fixture against your local emulator.
4. If the runner reports a result that matches what you expected, you
   are done. If you are bootstrapping a fixture against a known-good
   emulator, pass `--update-baselines` to write the captured rows
   back into the fixture, then review the diff and commit.

## Process hygiene

The runner registers signal handlers (`SIGINT`, `SIGTERM`) that
propagate to every emulator subprocess it spawned. A crashed runner
will still leave zombies (Go cannot reap children of a SIGKILLed
parent); after such a crash, run `pkill -f emulator_main` or use the
repo's `task bazel:kill-strays` helper. The integration test
(`conformance/cmd/runner/runner_test.go`, gated by `-tags=integration`)
exercises the cleanup path against a real engine.
