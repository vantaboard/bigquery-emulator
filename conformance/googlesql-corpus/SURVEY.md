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

## Starter subset (plan 13 initial vendoring)

| File | Cases | Why |
|------|-------|-----|
| `logical_functions.test` | 59 | Pure `SELECT` boolean logic; no `prepare_database`, proto, or feature flags |

Deferred for widening (landed routes from plans 01–02):

- `strings.test` — mostly pure scalar strings; tail needs `ADDITIONAL_STRING_FUNCTIONS` / JSON features
- `hash.test` — aggregates + `UNNEST`; good join/array coverage once UNNEST lane lands
- `join_queries.test` — rich `prepare_database` tables; needs seeding support in runner
- `groupby_queries.test` — proto deps + large prepare blocks

## Runner seeding posture

- **Today:** `jobs.query` for `SELECT` cases; `datasets.insert` for project bootstrap.
- **`prepare_database`:** deferred — requires running `CREATE TABLE AS SELECT` via setup SQL and reusing table state across cases in one file (fixture lane's `setup: sql:` path is the template).
- **Proto loads:** out of scope — skip cases with `load_proto_*` directives until catalog proto support exists.

## Feature-flag triage

Cases declaring `required_features` the emulator does not enable are bucketed
`corpus-feature-out-of-scope` and excluded from the pinned gate manifest.
