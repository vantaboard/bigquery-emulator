---
name: FixTests 14 — Third-party full bar
overview: Bring task thirdparty (golang, python, node, java, bigframes gate) to green once the engine is stable, then incrementally expand CI to gate the suites that are manual today. This is the convergence plan after first-party conformance and the bqutils corpus are done.
depends_on: [fixtests-07-advanced-relational, fixtests-13-bqutils-tail]
est_effort: 1-2 weeks
isProject: true
todos:
  - id: baseline
    content: Run task thirdparty against the current engine and capture per-suite failure counts (golang, python, node, java, bigframes gate).
    status: pending
  - id: cross-ref
    content: Cross-reference failures with the existing thirdparty-00-index sub-plans; route deep feature gaps there rather than duplicating, and fix only what the stabilized engine now unblocks.
    status: pending
  - id: green-suites
    content: Drive each suite to its pass bar (golang live, python snippets nox, node mocha, java non-allowlisted ITs, 4 bigframes snippets).
    status: pending
  - id: ci-expand
    content: Add python/node/bigframes as gated CI workflows (sync scripts + fake-gcs seeding in CI), building on the honest Java signal from plan 04.
    status: pending
  - id: dbt
    content: Run task thirdparty:dbt-bigquery-tests (manual lane); triage and document pass bar.
    status: pending
---

# FixTests 14 — Third-party full bar

## Why

`task thirdparty` is the full client-library integration bar and is only partially gated in CI today (golang compile-only on PR; Java live soft-gated; python/node/bigframes/dbt manual). It should be the **last** lane, because most of its failures are downstream of engine correctness fixed in Track A + Track B.

This plan does not re-derive the deep third-party feature work — that already lives in [thirdparty-00-index.plan.md](thirdparty-00-index.plan.md) and its sub-plans. This plan (a) fixes what the now-stable engine unblocks, (b) routes the rest to those existing plans, and (c) expands CI gating.

## Suites and pass bars

```bash
task thirdparty   # golang -> python -> node -> java -> bigframes gate (continues past per-suite failures)
```

| Suite | Prerequisite | Pass bar |
|-------|--------------|----------|
| golang | `BIGQUERY_EMULATOR_HOST` set (live) | live integration pass |
| python | `scripts/sync_python_bigquery_tests.sh` first | `nox -s snippets` against emulator |
| node | fake-gcs seed | Mocha pass |
| java | allowlist in [`third_party/README.md`](third_party/README.md) | non-allowlisted ITs pass (e.g. `QueryMaterializedViewIT`) |
| bigframes | snippet-gate (4 named snippets) | 4 allowlisted snippets pass |
| dbt | `task thirdparty:dbt-bigquery-tests` | pytest emulator profile (manual triage) |

## Steps

1. Baseline `task thirdparty` (with `THIRDPARTY_FRESH_VOLUME=1` default); record per-suite counts.
2. For each failure, decide: **engine-now-fixed** (fix here), **known third-party feature gap** (route to the relevant [thirdparty-00-index.plan.md](thirdparty-00-index.plan.md) sub-plan), or **harness** (sync/seed/fake-gcs).
3. Drive each suite to its bar.
4. Expand CI: add python/node/bigframes workflows (need sync scripts + fake-gcs seeding in CI), building on the honest Java signal from [fixtests-04-ci-parity-infra.plan.md](fixtests-04-ci-parity-infra.plan.md).
5. Run + document the dbt lane.

## Verify

```bash
task thirdparty            # all five suites green
task ci:run                # full local CI mirror green
```

Plus: CI `thirdparty-samples` honest (no `continue-on-error` masking); new python/node/bigframes workflows green on a PR.

## Out of scope

- First-party conformance (Track A) and bqutils corpus (Track B) — prerequisites, not part of this plan.
- Deep per-feature third-party work already owned by the `thirdparty-*` plan set; reference, don't duplicate.
