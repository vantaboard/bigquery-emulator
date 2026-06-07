---
name: BQUtils 07 — Stored procedures corpus
overview: Add conformance coverage for bigquery-utils stored_procedures/ (chi_square, linear_regression, get_next_ids, etc.). No upstream automated harness exists, so build fixtures from the README's documented golden-value CALL examples.
depends_on: [bqutils-01-codegen-pipeline, bqutils-02-engine-build-triage]
blocks: []
est_effort: 3-5 days
isProject: true
todos:
  - id: inventory
    content: Inventory stored_procedures/definitions/*.sqlx and the README CALL/ASSERT examples with expected numeric outputs; classify self-contained (chi_square, linear_regression, get_next_ids) vs external-dep (bqml_*).
    status: pending
  - id: engine-check
    content: Confirm engine support for CREATE PROCEDURE + CALL + scripting (BEGIN/DECLARE/ASSERT); identify gaps that block the self-contained procedures.
    status: pending
  - id: fixtures
    content: Hand-author (or extend the generator for) conformance fixtures that CREATE PROCEDURE in setup, CALL with inline data, and assert the README golden values; place in passing/ or known_failing/ per engine result.
    status: pending
  - id: docs
    content: Document the stored-procedure coverage + provenance in third_party/README.md.
    status: pending
---

# BQUtils 07 — Stored procedures corpus

## Why

`stored_procedures/definitions/*.sqlx` ships 6 procedures (`get_next_ids`, `chi_square`, `linear_regression`, `bh_multiple_tests`, `bqml_generate_embeddings`, `bqml_generate_text`). Unlike `udfs/`, there is **no `test_cases.js` harness** — the README documents manual `BEGIN ... CALL ... ASSERT` examples with expected numeric outputs (e.g. chi-square / linear regression on embedded Iris data). Those golden values are the oracle.

## Scope split

- **Self-contained (target):** `get_next_ids`, `chi_square`, `linear_regression`, `bh_multiple_tests` — inline temp-table data, deterministic outputs.
- **External-dep (skip):** `bqml_generate_embeddings`, `bqml_generate_text` — require BQML models / `bigquery-public-data`.

## Steps

1. **Inventory**: extract each procedure's `.sqlx` body (strip Dataform `config`/`${self()}`) and the README CALL+expected-value examples.
2. **Engine check**: confirm `CREATE PROCEDURE` + `CALL` + scripting (`BEGIN`/`DECLARE`/`SET`/`ASSERT`) support; the conformance lane already supports `scripting/` + `ASSERT` fixtures, so an `ASSERT`-based fixture is the natural shape.
3. **Fixtures**: author fixtures that `setup.sql` the `CREATE PROCEDURE`, then `query` a script that `CALL`s and `ASSERT`s the README golden value (or returns a `matches` bool row, mirroring the UDF comparator). Decide whether to extend `genbqutils` (README parsing is brittle) or hand-author these few; hand-authoring is likely cleaner given there are ~4.
4. Triage into `passing/` vs `known_failing/`; **docs**.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run-fixture FIXTURE=conformance/thirdparty-fixtures/bigquery_utils/.../chi_square.yaml
```

## Out of scope

- BQML procedures (`bqml_*`).
- Views (plan 08).
