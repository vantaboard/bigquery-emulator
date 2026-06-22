# Conformance fixtures + runner

The conformance harness drives the BigQuery emulator through its REST
gateway with declarative YAML fixtures and diffs the resulting rows (or
errors) against the values pinned in each file. The harness is the
conformance-harness capability area of `ROADMAP.md`; it consists of
the runner CLI (`conformance/cmd/runner`) plus the fixture set
under `conformance/fixtures/`. At time of writing the directory contains
**190+ YAML fixtures** spanning SELECT shapes, GROUP BY / aggregates,
JOINs, CTEs / subqueries, DML / DDL round-trips, functions, scripting /
UDFs, time travel, wildcard tables, GIS, security, `MATCH_RECOGNIZE`,
`INFORMATION_SCHEMA`, structural errors, and schema-only smokes; see the
"Contributing a new fixture" section below to add more.

> **Sibling lanes:** [`third_party/`](../third_party/README.md) hosts
> the imported client-library conformance suites
> (`task thirdparty:*`) and the **bigquery-utils UDF lane**
> (`task conformance:bqutils`). Client tests assert that the published
> Google BigQuery clients talk to this emulator over REST + gRPC;
> the bigquery-utils lane runs generated YAML under
> `conformance/thirdparty-fixtures/bigquery_utils/passing/` (CI-gated via
> `.github/workflows/conformance.yml` job `bqutils` after `build-engine`).
> Refresh with `task conformance:bqutils-sync`; triage with
> `./scripts/triage_bqutils_fixtures.sh`. See
> [`third_party/README.md`](../third_party/README.md#bigquery-utils-udf-conformance-non-gating).
>
> **GoogleSQL `.test` corpus lane** (`task conformance:googlesql-corpus`):
> vendored upstream compliance files under
> [`conformance/googlesql-corpus/`](googlesql-corpus/README.md), executed
> via `jobs.query` with the same typed-cell comparator as YAML fixtures.
> CI job `googlesql-corpus` in
> [`.github/workflows/googlesql-parity.yml`](../.github/workflows/googlesql-parity.yml)
> gates on `manifest/pinned.json`. Re-triage after GoogleSQL upgrades with
> `go run ./conformance/cmd/googlesql-corpus --triage --gate-pinned=false`.
>
> **Differential production-BigQuery oracle lane** (`task conformance:differential`):
> replays committed oracles under [`conformance/differential/oracle/`](differential/oracle/)
> against the emulator for each case in [`conformance/differential/corpus/`](differential/corpus/).
> Uses the same typed-cell comparator as YAML fixtures. CI job `differential` in
> [`.github/workflows/conformance.yml`](../.github/workflows/conformance.yml)
> runs the replay only (no GCP access). Record fresh oracles with
> `task conformance:differential-record` when `BIGQUERY_DIFFERENTIAL_PROJECT` and
> ADC are available; otherwise pin from `bq query` output and mark
> `oracle_source: bq-cli` per `.cursor/rules/conformance-bq-validation.mdc`.

## Quick start

```bash
# 1. Build the C++ engine if you have not already.
task emulator:build-engine:bazel

# 2. Run every fixture in conformance/fixtures against the active
#    profile set (the local-execution coordinator's `local` profile
#    today), spawning a fresh emulator per fixture x profile.
task conformance:run
# (equivalent to `go run ./conformance/cmd/runner`)

# 3. Restrict the matrix to one or more profiles. PROFILE is a
#    comma-separated list; each entry maps to a single --profile flag.
#    The legacy `duckdb` profile name is accepted as an alias for
#    `local` while the rename is in flight.
task conformance:run PROFILE=local

# 4. Run a single fixture file. Honors PROFILE too.
task conformance:run-fixture FIXTURE=conformance/fixtures/select_literal_value.yaml

# 5. JSON output for CI consumption (the diff CI ingests this).
task conformance:run OUTPUT=json OUTPUT_FILE=conformance-result.json

# 6. Reach an already-running gateway on :9060 instead of spawning
#    emulator_main subprocesses (faster dev loop).
go run ./conformance/cmd/runner \
  --fixtures conformance/fixtures/select_literal_value.yaml \
  --profile local \
  --connect 127.0.0.1:9060
```

## Profile matrix

A fixture's `profiles:` list declares which runtime profile it
applies to. Only one profile is defined today:

| profile | engine                      | storage | use for                                |
|---------|-----------------------------|---------|----------------------------------------|
| local   | local execution coordinator | duckdb  | All conformance assertions (only lane) |

The runner also accepts the legacy name `duckdb` as an alias for
`local` while the rename is in flight; new fixtures should write
`local`. The single profile drives the in-process route classifier
that dispatches to DuckDB fast path / DuckDB UDF / semantic
executor / control-op handlers (see
[`docs/ENGINE_POLICY.md`](../docs/ENGINE_POLICY.md)).

A fixture that omits `profiles:` runs on the default profile set
(currently `[local]`). The runner still tags every result with its
profile name in both `text` and `json` output modes so a future
expansion (an experimental storage backend, an additional
coordinator variant, etc.)
does not need a CI-format break.

## Fixture schema

Every fixture is a single YAML document with the following fields:

```yaml
name: insert_then_select          # required, identifier (matches filename)
description: |                    # optional, human prose for the diff log
  Catalog setup + row seed + SELECT round-trip.
profiles: [duckdb]                # optional; omitted = default profile set
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
  - rows:                         # REST POST tabledata.insertAll
      dataset: ds_conformance
      table: people
      rows:
        - {id: 1, name: a}
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
3. `rows:` — `POST /bigquery/v2/projects/<projectId>/datasets/<datasetId>/tables/<tableId>/insertAll`
   with `{kind:"bigquery#tableDataInsertAllRequest", rows:[{json:{...}}, ...]}`.
   This is the canonical seeding path today because the coordinator
   currently routes `INSERT VALUES` to the `unsupported` route (see
   `docs/ENGINE_POLICY.md` for the landing plan). Each row map
   is wrapped in `{json: ...}` by the runner.
4. `sql: <query>` — `POST /bigquery/v2/projects/<projectId>/queries`
   with `{query, useLegacySql:false}`. Use this for `MERGE`,
   `CREATE TABLE`, `DROP TABLE`, etc. `INSERT` / `UPDATE` / `DELETE`
   are on the `unsupported` route today; use `rows:` for seeding
   instead. The setup step fails the fixture if the gateway responds
   with a non-2xx status.

A fixture that needs only a `SELECT` (with no catalog state) omits
`setup` entirely.

### `expected.match` matching modes

Three matching modes are supported. The mode is declared on
`expected.match`; omitting it defaults to `ordered` (the original
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

### `expected.route` matching

`docs/ENGINE_POLICY.md` wires
the coordinator's `RouteClassifier` decision through to the REST
response as the `emulatorRoute` debug field on
`Job.statistics.query`. The field is loopback-only -- the
`gateway/middleware/loopback.go` middleware drops it for non-loopback
callers so the public REST shape stays byte-identical to BigQuery --
and the conformance runner always sees it because it talks to a
local emulator subprocess.

`expected.route` pins the canonical lowercase-snake disposition
the fixture's `query:` step MUST route through. One of:

- `duckdb_native` -- lowers to DuckDB SQL whose semantics already
  match BigQuery exactly.
- `duckdb_rewrite` -- lowers to DuckDB SQL via a structural rewrite.
- `duckdb_udf` -- lowers via a DuckDB UDF / macro polyfill.
- `semantic_executor` -- runs on the local row/value semantic
  executor.
- `control_op` -- DDL / metadata / catalog op.
- `local_stub` -- deterministic BigQuery-shaped stub.
- `unsupported` -- deliberately out-of-scope locally.

`expected.route_strict` (default `true`) toggles between
exact-match and `expected.route_allowlist`-membership. Strict mode
is the right default for every shape whose route is a
fixture-meaningful behavior (which is almost everything).
`route_strict: false` opts into the allowlist mode, where the
runner accepts any route in `route_allowlist` (and treats an empty
actual as a skip -- the documentation-only pattern used by
error-path fixtures whose engine response never carries a trailer).

Vocabulary spelling is validated at load time; an unknown
`expected.route` value fails the load with a hint listing the
canonical names.

The `task conformance:routing-matrix` Task walks every fixture
and emits a Markdown table of `Shape | Route | Strict` so a
reviewer can spot when a fixture family's actual route drifts
from its directory's aspirational label. The
[`conformance.yml` CI workflow](../.github/workflows/conformance.yml)
uploads the matrix as a non-blocking artifact.

### Worked example

The fixture below seeds three rows via `tabledata.insertAll` and then
asserts a filtered SELECT returns the single matching row. `INSERT` /
`UPDATE` / `DELETE` are on the `unsupported` route today (see
`docs/ENGINE_POLICY.md`), so fixtures use `rows:` for seeding
rather than DML `sql:` steps.

```yaml
name: select_where_clause
project_id: proj-select-where
dataset_id: ds_select_where
setup:
  - dataset: ds_select_where
  - table:
      dataset: ds_select_where
      id: people
      schema:
        - {name: id, type: INT64, mode: REQUIRED}
        - {name: name, type: STRING, mode: NULLABLE}
  - rows:
      dataset: ds_select_where
      table: people
      rows:
        - {id: 1, name: ada}
        - {id: 2, name: linus}
        - {id: 3, name: grace}
query: |
  SELECT id, name FROM ds_select_where.people
  WHERE name = 'linus' ORDER BY id
expected:
  rows:
    - {id: "2", name: "linus"}
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
                          named profiles (duckdb). Default: all.
  --update-baselines      Overwrite the `expected:` block of every fixture
                          with the actual response. Used to bootstrap new
                          fixtures.
  --output FORMAT         text (human, default) or json (machine,
                          consumed by the diff CI).
  --output-file PATH      If non-empty, tee the rendered report into
                          this file (atomic write via a sibling tmp +
                          rename) in addition to stdout. Used by the
                          CI workflow to capture per-profile JSON
                          artifacts.
```

Exit codes:

- `0` — every fixture x profile PASSed.
- `1` — at least one fixture x profile FAILed.
- `2` — runner-internal error (bad YAML, can't start engine, etc).

### JSON output shape (consumed by the diff CI)

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
      "profile": "duckdb",
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
new fixture. The fixture set already covers the broad categories
(SELECT shapes, GROUP BY / aggregates, JOINs, CTEs / subqueries, DML,
scripting / UDFs, time travel, wildcard tables, GIS, security,
`MATCH_RECOGNIZE`, `INFORMATION_SCHEMA`, structural errors, DDL,
schema-only); a new fixture is justified when it pins a *new* construct
or a *new* failure mode the existing set does not already exercise.

When a fixture's query misbehaves in the emulator, validate the SQL
against production BigQuery with `bq query` before assuming an emulator
bug: a `bq` error means fix the fixture SQL; `bq` success means use
the `bq` output as expected (see
[`.cursor/rules/conformance-bq-validation.mdc`](../.cursor/rules/conformance-bq-validation.mdc)).

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
| `ddl_` | `ddl_drop_table_then_select` |
| `time_travel/` | `snapshot_clone_round_trip`, `decorator_relative` |
| `wildcard/` | `wildcard_union`, `wildcard_suffix_filter` |
| `security/` | `row_access_filtered_select`, `masked_column_select` |
| `udf/` | `js_scalar_add`, `drop_function` |
| `error_` | `error_invalid_sql`, `error_unknown_table`, `error_type_mismatch` |

The `name:` field inside the YAML **must** match the filename
without the extension; the runner echoes the `name:` in its diff
log and a mismatch makes the failure harder to bisect.

### 2. Write `name`, `profiles`, `setup`, `query`, `expected`

Start from a similar existing fixture and adapt it. The shape is
documented under "Fixture schema" above; key choices to make:

- **`profiles:`** -- omit unless you need to single-target a future
  profile. Today the default profile set is `[local]`.
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
# Run the new fixture.
task conformance:run-fixture FIXTURE=conformance/fixtures/<name>.yaml

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

## Local execution constraints

The conformance harness drives the emulator's only supported
runtime: the local execution coordinator (DuckDB fast path +
planned semantic executor + planned control-op handlers) on top of
DuckDB storage. A few constructs are routed `unsupported` today;
fixtures should avoid them entirely until their landing plan
ships:

- **`INSERT VALUES` / `INSERT ... SELECT`** -- on the `unsupported`
  route today; lands via `docs/ENGINE_POLICY.md`. Use a
  `rows:` setup step to seed table data via `tabledata.insertAll`
  instead.
- **`UPDATE` / `DELETE`** -- on the `unsupported` route today;
  lands via `docs/ENGINE_POLICY.md`. `MERGE`'s easy branches
  lower through the DuckDB fast path and work as a fixture `sql:`
  setup step.
- **`SELECT 1` / scalar-only `SELECT`** -- on the `unsupported`
  route today; lands via `docs/ENGINE_POLICY.md`. Use a
  real table scan even for trivial fixtures.

See [`docs/ENGINE_POLICY.md`](../docs/ENGINE_POLICY.md) for the
local execution policy and the per-shape route catalog, and
[`docs/ENGINE_POLICY.md`](../docs/ENGINE_POLICY.md)
for the landing schedule.

## Adding fixtures (compact recap)

Already comfortable with the above? The short version:

1. Drop a new `<name>.yaml` under `conformance/fixtures/`. Keep the
   filename and the `name:` field in sync.
2. Leave `profiles:` unset unless you need to single-target a
   profile (today there is only one).
3. Run `task conformance:run-fixture FIXTURE=conformance/fixtures/<name>.yaml`
   to verify the fixture against your local emulator.
4. If the runner reports a result that matches what you expected, you
   are done. If you are bootstrapping a fixture against a known-good
   emulator, pass `--update-baselines` (via
   `task conformance:update-baselines FIXTURE=...`) to write the
   captured rows back into the fixture, then **read the diff**
   carefully before committing.

## Differential lane

The differential lane compares emulator output against **committed
production-BigQuery oracles** instead of hand-pinned `expected:` blocks in
fixture YAML. It exists to catch semantic divergences the YAML suite misses
when expectations were bootstrapped from emulator output.

### Layout

| Path | Role |
|------|------|
| `conformance/differential/corpus/*.yaml` | One case per file: `setup:`, `query:`, `oracle_ref:` |
| `conformance/differential/oracle/*.json` | Recorded schema/rows/error + provenance (`captured_at`, `job_id`) |

Corpus YAML reuses the fixture `setup:` / `query:` schema. Optional fields:

- `oracle_ref` — basename under `oracle/` (required)
- `oracle_source` — `recorded` or `bq-cli` provenance hint
- `match` — `ordered` / `unordered` override for replay
- `known_failing` — expected divergence; counted as SKIP, not FAIL
- `query_parameters` — named parameters for parameterized queries

### Commands

```bash
# Replay committed oracles (CI path; no GCP creds).
task conformance:differential

# JSON report for CI artifacts.
task conformance:differential OUTPUT=json OUTPUT_FILE=conformance-differential.json

# Record oracles from production BigQuery (manual/opt-in).
export BIGQUERY_DIFFERENTIAL_PROJECT=my-gcp-project
gcloud auth application-default login
task conformance:differential-record
```

When `BIGQUERY_DIFFERENTIAL_PROJECT` is unset, `differential-record` prints
setup instructions and exits 0.

### Divergence classification

Each mismatch is tagged as one of:

| Kind | Meaning |
|------|---------|
| `match` | Oracle and emulator agree |
| `feature_gap` | Emulator returns Unimplemented / not-yet-landed |
| `semantic_divergence` | Both succeed, rows differ |
| `error_divergence` | One side errors, the other succeeds |
| `crash` | Engine abort or RPC failure |

## Session fixtures

Session fixtures under [`conformance/sessions/`](sessions/) exercise **ordered,
stateful sequences** against a **single long-lived** `emulator_main` process.
Unlike the YAML fixture lane (one query per fresh engine), sessions can express
multi-step client workflows, `repeat:` loops, generic REST steps (`rest:`),
mid-session `restart:` (same `--data_dir`), and assertions such as
`expect_alive`, `expect_table_list`, and `expect_rows` on intermediate queries.

```bash
# Run every committed session (skips _selftest_* files).
task conformance:session

# JSON report for CI artifacts.
task conformance:session OUTPUT=json OUTPUT_FILE=conformance-session.json

# Single session file.
go run ./conformance/cmd/session \
  --sessions conformance/sessions/dataset_list_after_view_op.yaml \
  --engine-binary ./bin/emulator_main
```

Optional session fields:

- `known_failing` — expected divergence; counted as SKIP, not FAIL (mirrors the
  differential / bqutils pattern). Use with `known_failing_ref` pointing at the
  owning fix plan when the repro must stay red until an engine fix lands.

CI job `session` in
[`.github/workflows/conformance.yml`](../.github/workflows/conformance.yml)
runs after `build-engine`.

## Process hygiene

The runner registers signal handlers (`SIGINT`, `SIGTERM`) that
propagate to every emulator subprocess it spawned. A crashed runner
will still leave zombies (Go cannot reap children of a SIGKILLed
parent); after such a crash, run `pkill -f emulator_main` or use the
repo's `task bazel:kill-strays` helper. The integration test
(`conformance/cmd/runner/runner_test.go`, gated by `-tags=integration`)
exercises the cleanup path against a real engine.
