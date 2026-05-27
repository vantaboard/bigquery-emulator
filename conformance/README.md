# Conformance fixtures + runner

The conformance harness drives the BigQuery emulator through its REST
gateway with declarative YAML fixtures and diffs the resulting rows (or
errors) against the values pinned in each file. The harness is Phase 8
of `ROADMAP.md`; the runner CLI is the deliverable of plan 40
(`conformance-fixtures-runner`) and the seed fixture set is plan 42
(`conformance-seed-docs`). At time of writing the directory contains
**24 fixtures** spanning SELECT shapes, GROUP BY / aggregates, JOINs,
DML, structural errors, DDL, and a schema-only smoke check; see the
"Contributing a new fixture" section below to add more.

> **Sibling lane:** [`third_party/`](../third_party/README.md) hosts
> the imported client-library conformance suites
> (`task thirdparty:*`). Those tests assert that the published
> Google BigQuery clients (Go, Node.js, Python, BigQuery DataFrames)
> talk to this emulator over its REST + gRPC surface end-to-end, in
> contrast to the fixture lane below which pins SQL semantics through
> a purpose-built runner. The two lanes are independent in CI.

## Quick start

```bash
# 1. Build the C++ engine if you have not already.
task emulator:build-engine:bazel

# 2. Run every fixture in conformance/fixtures against both profiles,
#    spawning a fresh emulator per fixture × profile.
task conformance:run
# (equivalent to `go run ./conformance/cmd/runner`)

# 3. Restrict the matrix to one or more profiles. PROFILE is a
#    comma-separated list; each entry maps to a single --profile flag.
task conformance:run PROFILE=memory
task conformance:run PROFILE=memory,duckdb

# 4. Run a single fixture file. Honors PROFILE too.
task conformance:run-fixture FIXTURE=conformance/fixtures/select_literal_value.yaml

# 5. JSON output for CI consumption (plan 41 hooks this).
task conformance:run OUTPUT=json OUTPUT_FILE=conformance-result.json

# 6. Reach an already-running gateway on :9060 instead of spawning
#    emulator_main subprocesses (faster dev loop).
go run ./conformance/cmd/runner \
  --fixtures conformance/fixtures/select_literal_value.yaml \
  --profile memory \
  --connect 127.0.0.1:9060
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
  --output-file PATH      If non-empty, tee the rendered report into
                          this file (atomic write via a sibling tmp +
                          rename) in addition to stdout. Used by the
                          CI workflow to capture per-profile JSON
                          artifacts.
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

## Contributing a new fixture

This section walks through the end-to-end workflow for adding one
new fixture. The seed set already covers the broad categories
(SELECT shapes, GROUP BY / aggregates, JOINs, DML, structural
errors, DDL, schema-only); a new fixture is justified when it pins
a *new* construct or a *new* failure mode the existing set does not
already exercise.

### 1. Pick a category and name

Use the filename pattern `<category>_<feature>.yaml` so a directory
listing groups related fixtures together. Categories already in
use (skim the existing files before naming):

| Category prefix | Examples |
|---|---|
| `select_` | `select_literal_value`, `select_where_clause`, `select_case_expression` |
| `aggregate_` | `aggregate_count_star`, `aggregate_count_distinct` |
| `groupby_` | `groupby_count`, `groupby_having` |
| `join_` | `join_inner`, `join_left_outer` |
| `dml_` | `insert_then_select`, `dml_update_delete`, `dml_delete_where_predicate` |
| `ddl_` | `ddl_create_table_as_select`, `ddl_drop_table_then_select` |
| `error_` | `error_invalid_sql`, `error_unknown_table`, `error_type_mismatch` |

The `name:` field inside the YAML **must** match the filename
without the extension; the runner echoes the `name:` in its diff
log and a mismatch makes the failure harder to bisect.

### 2. Write `name`, `profiles`, `setup`, `query`, `expected`

Start from a similar existing fixture and adapt it. The shape is
documented under "Fixture schema" above; key choices to make:

- **`profiles:`** -- both `memory` and `duckdb` unless you have a
  specific reason to target one. See "When DuckDB and the memory
  profile disagree" below.
- **`setup:`** -- only the catalog state the fixture *needs*. A
  SELECT fixture that doesn't read any tables can omit `setup:`
  entirely (see `select_literal_value.yaml`).
- **`query:`** -- a single SQL statement. One scenario per fixture
  keeps the diff readable.
- **`expected:`** -- pin either rows or an error, per the schema
  table above.

Lead the file with a **top-of-file comment** explaining what the
fixture pins and which existing test (if any) it mirrors. Reviewers
lean on this when a fixture starts failing; "I copied another one"
is not enough context to triage.

### 3. Run the fixture locally

The conformance harness needs `./bin/emulator_main` -- the
canonical Bazel-built engine (run `task emulator:build-engine:bazel`
once if you have not already; it is cached).

```bash
# Run the new fixture against both profiles.
task conformance:run-fixture FIXTURE=conformance/fixtures/<name>.yaml

# Run against only one profile (faster iteration on a duckdb-only
# fixture, for example).
task conformance:run-fixture FIXTURE=conformance/fixtures/<name>.yaml PROFILE=duckdb

# Run the full suite to confirm no regression.
task conformance:run
```

### 4. Use `--update-baselines` carefully

When you're authoring a fixture and don't know the exact wire
shape the engine returns, `--update-baselines` rewrites the
fixture's `expected:` block with whatever the engine actually
produced:

```bash
task conformance:update-baselines FIXTURE=conformance/fixtures/<name>.yaml
```

**Do not commit the rewritten baseline without reading it.** The
engine might return what you expected, or it might be returning
something subtly wrong (extra column, stringified NULL, wrong row
order, a literal `"NULL"` instead of a real null). Eyeball every
cell. If the engine looks wrong, fix the *engine* (or open a bug),
do not pin the bad output as the expected baseline.

### 5. Add to your PR

The fixture YAML is the only file you typically need to touch. If
the new fixture is a brand-new category or stresses an unusual
shape, mention it briefly in your PR description so reviewers know
to look at it.

## Choosing the right match mode

`expected.match` controls how the runner compares rows. See the
"matching modes" table above for the full semantics; this is the
quick decision tree for picking one:

1. **Does the fixture have a single scalar result row?** Use the
   default (`ordered`); ordering is trivial with one row.
2. **Does the query use `ORDER BY`?** Use `ordered` (the default).
3. **Is it a `GROUP BY` / parallel-scan query whose row order is
   implementation-defined?** Either add an `ORDER BY` and use
   `ordered`, or use `unordered`. Prefer `ORDER BY` -- the diff
   output is more readable when the rows are pinned positionally.
4. **Are the row values non-deterministic (timestamps, generated
   IDs, hashes)?** Use `schema_only` with an explicit `schema:`
   list. The runner asserts on the column names + types and
   ignores the cell values.

## When DuckDB and the memory profile disagree

The `memory` profile is `--engine=reference_impl --storage=memory`;
the `duckdb` profile is `--engine=duckdb --storage=duckdb
--on_unknown_fn=fallback` (DuckDB-uncovered constructs route to
ReferenceImpl through `FallbackEngine`).

The two profiles disagree on:

- **DDL** -- only the DuckDB engine implements `CREATE` / `DROP`
  / `CREATE TABLE AS SELECT`. The memory profile leaves
  `--on_unknown_fn` at its default (`unimplemented`), so DDL
  fixtures targeting the memory profile would surface a 501. Mark
  DDL fixtures `profiles: [duckdb]`.
- **Cell type fidelity from in-memory storage** -- the in-memory
  storage layer returns every cell as `Value::String` regardless
  of the declared column type. Equality (`WHERE id = 1`) and
  `ORDER BY id` work because the engine normalizes both sides;
  but predicates like `WHERE id > 1` fail on memory because
  INT64 > STRING is not a supported comparison. Fixtures that
  exercise comparison operators on integer columns should either
  filter on STRING columns or be tagged `profiles: [duckdb]`.
- **`INFORMATION_SCHEMA` views** -- only ReferenceImpl implements
  most of them today. A fixture pinning a system view may need
  `profiles: [memory]`.

When in doubt, target the `duckdb` profile first; see
[`docs/ENGINE_POLICY.md`](../docs/ENGINE_POLICY.md) for the
"DuckDB-primary" policy decision and why.

## Adding fixtures (compact recap)

Already comfortable with the above? The short version:

1. Drop a new `<name>.yaml` under `conformance/fixtures/`. Keep the
   filename and the `name:` field in sync.
2. Decide which profile(s) the fixture applies to and list them in
   `profiles:`. Default to both unless you know one profile cannot
   support the shape (e.g. a Parquet-only storage feature). See
   "When DuckDB and the memory profile disagree" above for the
   common reasons to single-target a profile.
3. Run `task conformance:run-fixture FIXTURE=conformance/fixtures/<name>.yaml`
   to verify the fixture against your local emulator.
4. If the runner reports a result that matches what you expected, you
   are done. If you are bootstrapping a fixture against a known-good
   emulator, pass `--update-baselines` (via
   `task conformance:update-baselines FIXTURE=...`) to write the
   captured rows back into the fixture, then **read the diff**
   carefully before committing.

## Process hygiene

The runner registers signal handlers (`SIGINT`, `SIGTERM`) that
propagate to every emulator subprocess it spawned. A crashed runner
will still leave zombies (Go cannot reap children of a SIGKILLed
parent); after such a crash, run `pkill -f emulator_main` or use the
repo's `task bazel:kill-strays` helper. The integration test
(`conformance/cmd/runner/runner_test.go`, gated by `-tags=integration`)
exercises the cleanup path against a real engine.
