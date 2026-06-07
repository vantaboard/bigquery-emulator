---
name: FixTests 02 — Quick wins (disposition + fixture)
overview: Land the 8 high-confidence first-party conformance fixes that need only disposition flips, fixture-expectation corrections, or small wire/route adjustments. Target is fastpath 20/20 and full suite ~88/100.
depends_on: [fixtests-01-foundation]
est_effort: 2-3 days
isProject: true
todos:
  - id: unsupported-routes
    content: Flip GENERATE_UUID and SESSION_USER to the unsupported disposition so they return the 501 envelope expected by specialized_unsupported_engine_policy_link and specialized_unsupported_session_user.
    status: pending
  - id: fixture-expectations
    content: Correct fixture expectations for regression_integer_overflow (message_contains) and keys_new_keyset_stub (base64 BYTES) to match actual engine/wire behavior.
    status: pending
  - id: udf-macros
    content: Fix function_isnull and function_log (duckdb_udf macro registration / BOOL casing / route label) so both pass in the fastpath lane.
    status: pending
  - id: struct-array-wire
    content: Fix regression_struct_anonymous (positional _0/_1/_2 naming + BOOL wire casing) and expr_array_literal (schema_only REPEATED/NULLABLE alignment).
    status: pending
  - id: verify
    content: Re-run task conformance:fastpath (target 20/20) and task conformance:run; record the new full-suite count.
    status: pending
---

# FixTests 02 — Quick wins (disposition + fixture)

## Why

Eight of the 20 conformance failures are not deep engine gaps — they are disposition labels, fixture expectations that disagree with correct engine output, or small wire-format mismatches. Landing these first restores the **PR-gating fastpath lane (20/20)** and shrinks the full-suite backlog before the multi-week engine work.

> Confirm each row against the **observed diff** from [fixtests-01-foundation.plan.md](fixtests-01-foundation.plan.md) before editing — the analysis below is inferred.

## Fixes

| Fixture | Fix | Primary files |
|---------|-----|---------------|
| `specialized_unsupported_engine_policy_link` | Route `GENERATE_UUID` to `unsupported` (501 envelope); today it returns a fixed UUID | function disposition registry / [`node_dispositions.yaml`](backend/engine/duckdb/transpiler/node_dispositions.yaml), `functions.yaml` |
| `specialized_unsupported_session_user` | Route `SESSION_USER` to `unsupported`; today `net_funcs.cc` returns `"dummy"` | same + [`backend/engine/semantic/functions/net_funcs.cc`](backend/engine/semantic/functions/net_funcs.cc) |
| `regression_integer_overflow` | Fixture expects `"out of range"`; semantic executor emits `"Int64 overflow: ..."` — align `message_contains` | [`conformance/fixtures/scalar/regression_integer_overflow.yaml`](conformance/fixtures/scalar/regression_integer_overflow.yaml) |
| `specialized_keys_new_keyset_stub` | Expected BYTES cell should be base64 of the sentinel (BYTES wire is base64 per [`gateway/bqtypes/wire.go`](gateway/bqtypes/wire.go)) | [`conformance/fixtures/specialized/keys_new_keyset_stub.yaml`](conformance/fixtures/specialized/keys_new_keyset_stub.yaml) |
| `function_isnull` | `duckdb_udf` macro `bq_isnull`; likely BOOL wire casing (`"false"` vs `"FALSE"`) or route label | macro registration + [`conformance/fixtures/functions/conditional/function_isnull.yaml`](conformance/fixtures/functions/conditional/function_isnull.yaml) |
| `function_log` | `duckdb_udf` macro `bq_log`; rounding (`ROUND(...,6)`) or macro-not-registered routing fail | macro registration + [`conformance/fixtures/functions/numeric/function_log.yaml`](conformance/fixtures/functions/numeric/function_log.yaml) |
| `regression_struct_anonymous` | Positional struct name synthesis (`_0/_1/_2`) build vs access; BOOL wire casing | struct emit in [`backend/engine/duckdb/transpiler/`](backend/engine/duckdb/transpiler/) |
| `expr_array_literal` | `match: schema_only`; REPEATED/NULLABLE element-mode wire alignment | semantic literal / wire typing |

## Decision note

For disposition flips: prefer changing the **disposition/registry** so behavior matches BigQuery (501 for unsupported builtins), not the fixture. For fixture-expectation rows (`regression_integer_overflow`, `keys_new_keyset_stub`), the engine is correct and the fixture is wrong — edit the fixture and note why in the diff/commit.

## Verify

```bash
task conformance:run-fixture FIXTURE=conformance/fixtures/specialized/unsupported_session_user.yaml
task conformance:fastpath        # target 20/20
task conformance:run             # record new count (target ~88/100)
```

If any disposition flip touches the transpiler, run `tools/check_disposition_parity` (part of `task lint:run`) and update [`SHAPE_TRACKER.md`](backend/engine/duckdb/transpiler/SHAPE_TRACKER.md) / `node_dispositions.yaml` together to keep parity green.

## Out of scope

- `expr_with_expr`, `expr_cast_types`, `subquery_expr_*` (plan 05); scan/struct-nested (plan 06); pivot/unpivot/recursive (plan 07).
