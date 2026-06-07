---
name: FixTests 08 — bqutils ANY TYPE follow-up
overview: Promote the 6 remaining ANY TYPE templated-UDF fixtures still in known_failing after bqutils-03 - typeof, sure_values, and the cw_array_* family - by closing ERROR() guard, TO_JSON_STRING-on-ANY-TYPE, and aggregate (PERCENTILE_CONT) gaps.
depends_on: [fixtests-01-foundation]
est_effort: 3-5 days
isProject: true
todos:
  - id: repro
    content: Run the 6 ANY TYPE known_failing fixtures and capture whether each fails at CREATE (registration/analysis) or CALL (evaluation).
    status: pending
  - id: error-guard
    content: Implement/repair ERROR(FORMAT(...)) guard semantics used by sure_values and typeof.
    status: pending
  - id: to-json-string
    content: Support TO_JSON_STRING type introspection over ANY TYPE values (typeof).
    status: pending
  - id: array-aggregates
    content: Close array stat gaps - cw_array_max/min/median (PERCENTILE_CONT) and cw_array_stable_distinct ordering.
    status: pending
  - id: triage
    content: Re-sync + triage so newly-passing ANY TYPE fixtures move known_failing -> passing; update bqutils-00-index passing count.
    status: pending
---

# FixTests 08 — bqutils ANY TYPE follow-up

## Why

[bqutils-03-any-type-udfs.plan.md](bqutils-03-any-type-udfs.plan.md) is marked done, but **6 ANY TYPE fixtures still fail** — partial coverage. They are the cheapest Track B win because the registration path already exists; the gaps are specific builtins inside the UDF bodies.

Fixtures (in [`conformance/thirdparty-fixtures/bigquery_utils/known_failing/community/`](conformance/thirdparty-fixtures/bigquery_utils/known_failing/community/)): `typeof`, `sure_values`, `cw_array_max`, `cw_array_min`, `cw_array_median`, `cw_array_stable_distinct`.

## Likely gaps

| Fixture | Gap |
|---------|-----|
| `typeof` | `ANY TYPE` + `TO_JSON_STRING` type introspection + `RANGE<>` literals (RANGE handled in plan 10; cover the non-RANGE cases here) |
| `sure_values` | `ANY TYPE` + `ERROR(FORMAT(...))` guard |
| `cw_array_max` / `cw_array_min` | `ANY TYPE` array reduce |
| `cw_array_median` | `PERCENTILE_CONT` / aggregate over array |
| `cw_array_stable_distinct` | Order-preserving distinct over `ANY TYPE` array |

## Key files

- [`backend/engine/semantic/eval_expr_calls.cc`](backend/engine/semantic/eval_expr_calls.cc) — templated call dispatch -> `EvalSqlUdfBody`.
- [`backend/catalog/create_function_util.cc`](backend/catalog/create_function_util.cc), [`backend/catalog/udf_registry.cc`](backend/catalog/udf_registry.cc) — ANY TYPE registration.
- Semantic function impls for `ERROR`, `TO_JSON_STRING`, array aggregates.

## Steps

1. Repro each fixture; classify CREATE vs CALL failure.
2. Implement the missing builtins/guards used inside the bodies.
3. Re-sync and triage:

```bash
task emulator:build-engine:bazel
task conformance:bqutils-sync
./scripts/triage_bqutils_fixtures.sh
task conformance:bqutils
```

4. Update [bqutils-00-index.plan.md](bqutils-00-index.plan.md) passing count, [`third_party/README.md`](third_party/README.md), and `docs/ENGINE_POLICY.md`.

## Out of scope

- RANGE<> (plan 10), regexp/string (plan 11), bytes (plan 09), BIGNUMERIC (plan 12). If a fixture needs one of those, defer it to the owning plan and note the dependency.
