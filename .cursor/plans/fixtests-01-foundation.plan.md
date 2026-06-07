---
name: FixTests 01 — Foundation (push, repro, triage)
overview: Push the branch so CI runs on current code, reproduce every failing lane locally against a freshly built engine, capture exact per-fixture diffs, and triage the Bazel exit-37 from cc_test. This unblocks both tracks by turning inferred failures into observed failures.
blocks: [fixtests-02-quickwins, fixtests-03-cc-test, fixtests-04-ci-parity-infra, fixtests-08-bqutils-any-type]
est_effort: ~1 day
isProject: true
todos:
  - id: push
    content: Push main (or open a PR) so build-engine produces a fresh artifact and ci / conformance / googlesql-parity run on the current ~79-commit-ahead tree; record run IDs.
    status: pending
  - id: build
    content: Build the engine locally (task emulator:build-engine:bazel) and confirm bin/emulator_main is current before any conformance run.
    status: pending
  - id: repro-conformance
    content: Run task conformance:fastpath and task conformance:run; for each of the 20 failures capture the exact diff via task conformance:run-fixture and classify (schema / rows / expected-error-got-success / 501 / transpile).
    status: pending
  - id: repro-cctest
    content: Run GOOGLESQL_SOURCE=prebuilt task lint:cpp:test; record whether it is exit 37 (Bazel internal) or exit 3 (test failure) and capture server_log.
    status: pending
  - id: triage-sheet
    content: Write the observed failure classifications back into the relevant sub-plans (02, 03, 05, 06, 07) so downstream work starts from real diffs, not inferences.
    status: pending
---

# FixTests 01 — Foundation (push, repro, triage)

## Why

The failure analysis in the sibling plans was **inferred from fixture contracts vs. implementation**, not from live runner diffs, and remote CI is stale (~79 commits behind). Before spending engine time, convert every failure into an observed diff and let CI run on current code. This is the single dependency for the rest of the effort.

## Key commands

```bash
# 1. Push so CI runs on current code
git push                                  # build-engine -> ci / conformance / googlesql-parity

# 2. Build engine locally (hygiene per bazel-process-hygiene.mdc)
task emulator:build-engine:bazel

# 3. First-party conformance
task conformance:fastpath                 # CI gate: target 20/20 (today 15/20)
task conformance:run                      # full suite: target 100/100 (today 80/100)

# 4. Per-fixture diffs
task conformance:run-fixture FIXTURE=conformance/fixtures/<path>.yaml

# 5. C++ tests in CI mode
GOOGLESQL_SOURCE=prebuilt task lint:cpp:test
```

## The 20 failing first-party fixtures (capture diffs for each)

- advanced_relational: `pivot`, `unpivot`, `recursive_cte`
- cte_subquery: `subquery_expr_array`, `subquery_expr_in`, `subquery_expr_scalar`
- scalar: `expr_array_literal`, `expr_cast_types`, `expr_with_expr`, `regression_integer_overflow`
- fastpath: `regression_struct_anonymous`, `regression_struct_nested_field_order`, `scan_analytic_row_number`, `scan_join_full`, `scan_sample_bernoulli`
- functions: `function_isnull`, `function_log`
- specialized: `keys_new_keyset_stub`, `unsupported_engine_policy_link`, `unsupported_session_user`

## Bazel exit-37 triage

Exit **37** = Bazel `INTERNAL_ERROR` (crash), not a test assertion (which is exit **3**). The June-3 CI step died in ~4s with almost no output — consistent with a crash, not 44 slow tests.

- Re-run with `-s --verbose_failures`; inspect `bazel info server_log`.
- Confirm the `task lint:cpp:test` query path uses `--config=googlesql-prebuilt` ([`taskfiles/lint.yml`](taskfiles/lint.yml)); the old CI revision swallowed `bazel query` stderr with `2>/dev/null`.
- If it reproduces as exit **3** instead, hand off to [fixtests-03-cc-test.plan.md](fixtests-03-cc-test.plan.md) with the failing target list.

## Verify

- CI run IDs recorded for `build-engine`, `ci`, `conformance`, `googlesql-parity`.
- A triage note (file path + observed failure class) exists for all 20 fixtures.
- cc_test outcome classified (37 vs 3) with `server_log` captured.

## Out of scope

- Any engine or fixture edits — this plan only observes and records. Fixes live in plans 02–14.
