# GoogleSQL compliance `.test` corpus survey

Upstream source: sibling checkout `../googlesql/googlesql/compliance/testdata/`
(tag `2026.1.1`, matching `MODULE.bazel`). 258 `.test` files; ~851
`.test` files repo-wide (parser + compliance).

## File format

Each file is a sequence of cases separated by a lone `==` line.

Per case:

1. **Directives** (optional, `[key=value]` lines):
   - `[name=case_id]` — stable identifier
   - `[required_features=FEAT1,FEAT2]` — `LanguageFeature` tokens without `FEATURE_` prefix
   - `[prepare_database]` — SQL is catalog seeding (`CREATE TABLE … AS SELECT …`)
   - `[default required_features=…]` — file-level feature defaults
   - `[load_proto_files=…]`, `[load_proto_names=…]`, `[load_enum_names=…]` — proto catalog deps
2. **SQL** statement (may be multiline)
3. **`--`** separator
4. **Expected** result:
   - Success: `ARRAY<STRUCT<…>>[{…}]` (optional `[known order:` prefix)
   - Failure: `ERROR: …` message fragment

## Vendored subset

Byte-identical upstream files under `corpus/`. The runner gates each
file behind `manifest/pinned.json`: only `pinned-pass` cases run in the
CI gate; everything else is explicitly bucketed in `triage` (never
silently dropped). Counts below are from the triage pass; rerun
`go run ./conformance/cmd/googlesql-corpus --triage --gate-pinned=false`
after a corpus refresh.

| File | Pinned | Excluded (bucket) | Why vendored |
|------|--------|-------------------|--------------|
| `logical_functions.test` | 55 | 2 engine-bug (`IS` on `NULL`), 2 error-expectation | Pure `SELECT` boolean logic (starter file) |
| `arithmetic_functions.test` | 15 | 7 error-expectation, 3 engine-bug (`DIV`/`SAFE_DIVIDE` edge cases) | Scalar `+ - * / DIV MOD` arithmetic |
| `math_functions.test` | 1 | 28 feature-out-of-scope (`RADIANS_DEGREES_FUNCTIONS`) | Scalar math; trig deferred behind the feature flag |
| `cast_format_validation.test` | 10 | 12 error-expectation, 2 engine-bug | `SAFE_CAST … FORMAT …` scalar string/bytes/date casts |
| `regexp_functions.test` | 4 | — | `LIKE` pattern matching → scalar `BOOL` |

`unsupported_features` in the manifest skips whole feature families that
the engine does not enable yet (currently `RADIANS_DEGREES_FUNCTIONS`);
those cases bucket as `corpus-feature-out-of-scope` rather than running
and failing.

## Deferred families (and why)

These categories are intentionally *not* vendored yet — each is blocked
on a runner capability, not on the engine:

- **string / array / bytes** (`strings.test`, `array_functions.test`,
  `array_constructors.test`, `bytes.test`, `concat_function.test`) —
  results contain `ARRAY<…>[…]` / proto-typed cells that the
  expected-block parser (`expected.go`) does not yet decode. It handles
  scalar cells and nested `{…}` structs only; an array/struct-cell
  parser extension unlocks these.
- **date / time** (`date.test`, `additional_date_time_functions.test`)
  — these upstream files carry `#` comments before `[name=…]`, trailing
  `NOTE:` prose after the expected block, and bare multi-token
  `DATETIME` values (space between date and time) that the
  whitespace-delimited bare-token reader cannot parse. Vendoring needs
  either a parser hardening pass or a curated date file.
- **aggregation / analytic** (`aggregation_queries.test`,
  `analytic_*.test`) — depend on `prepare_database` table seeding, which
  the runner does not implement yet (cases bucket as
  `not-yet-landed-route`). The fixture lane's `setup: sql:` path is the
  template for adding it.

## Runner seeding posture

- **Today:** `jobs.query` for `SELECT` cases; `datasets.insert` for project bootstrap.
- **`prepare_database`:** deferred — requires running `CREATE TABLE AS SELECT` via setup SQL and reusing table state across cases in one file (fixture lane's `setup: sql:` path is the template).
- **Proto loads:** out of scope — skip cases with `load_proto_*` directives until catalog proto support exists.

## Feature-flag triage

Cases declaring `required_features` the emulator does not enable are bucketed
`corpus-feature-out-of-scope` and excluded from the pinned gate manifest.
