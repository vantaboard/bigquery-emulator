---
name: BQUtils 10 — dbt-bigquery integration (deferred)
overview: Bring dbt-labs/dbt-adapters dbt-bigquery functional tests against the emulator via the adapter's api_endpoint override, mirroring the existing python-bigquery third_party lane. Large; independent of the bigquery-utils plans.
depends_on: []
blocks: []
est_effort: 2+ weeks
isProject: true
todos:
  - id: feasibility
    content: Prototype pointing dbt-bigquery at the emulator via credentials api_endpoint + NoAuth; run one trivial functional test (BaseSimpleMaterializations) and capture the first wave of missing API surfaces.
    status: pending
  - id: scaffold
    content: Create third_party/dbt-bigquery-tests/ scaffold (committed curated files) mirroring the python-bigquery-tests scaffold+sync model.
    status: pending
  - id: sync-script
    content: "Write scripts/sync_dbt_bigquery_tests.sh: clone dbt-labs/dbt-adapters at a ref, rsync dbt-bigquery/tests/ (+ sibling dbt-tests-adapter where needed) *.py into the scaffold; pin ref; print SHA."
    status: pending
  - id: conftest-wiring
    content: Add emulator-aware conftest/profile wiring (api_endpoint -> BIGQUERY_EMULATOR_HOST, anonymous creds) and a skip plugin for surfaces the emulator lacks (Dataproc/python models, GCS uploads, INFORMATION_SCHEMA gaps, materialized views).
    status: pending
  - id: task-wiring
    content: Add task thirdparty:dbt-bigquery-tests wiring (nox/pytest via uvx) mirroring thirdparty:python-bigquery-tests; document the skip matrix in third_party/README.md.
    status: pending
  - id: triage
    content: Run, build the allowlist/skip set, and record which dbt functional tests pass against the emulator as real-world coverage.
    status: pending
---

# BQUtils 10 — dbt-bigquery integration (deferred)

## Why

[dbt-labs/dbt-adapters `dbt-bigquery`](https://github.com/dbt-labs/dbt-adapters/tree/main/dbt-bigquery) (Apache-2.0, cloned at `/home/brighten-tompkins/Code/dbt-adapters`) is the highest-fidelity real-world BigQuery workload available, but it is **deferred** because the SQL lives inside Python fixture strings + Jinja macros and is exercised only by ~95 pytest functional tests that today need a live BigQuery. This plan integrates it as a client-library lane (like `python-bigquery-tests`), not as conformance fixtures.

## Feasibility facts (verified)

- The adapter supports `api_endpoint` on credentials (`dbt-bigquery/src/dbt/adapters/bigquery/credentials.py:57-58`) and passes it through to `google.cloud.bigquery.Client` (`tests/unit/test_bigquery_adapter.py:87-93`).
- Functional `tests/conftest.py` (oauth/service-account targets) does **not** set `api_endpoint` today — emulator wiring requires extending it + anonymous/mock auth.
- Tests hit BQ-specific surfaces (jobs, datasets, INFORMATION_SCHEMA, materialized views, Dataproc python models, GCS uploads) — expect a meaningful skip matrix.

## Steps (mirror the python-bigquery third_party lane)

1. **Feasibility prototype**: point the adapter at the emulator (`api_endpoint` + NoAuth), run one `BaseSimpleMaterializations` test, capture missing surfaces.
2. **Scaffold**: `third_party/dbt-bigquery-tests/` with committed curated files (profiles, emulator conftest, skip plugin), `*.py` synced on demand — same policy as `third_party/python-bigquery-tests/`.
3. **Sync script** `scripts/sync_dbt_bigquery_tests.sh`: clone `dbt-labs/dbt-adapters` at a pinned ref; rsync `dbt-bigquery/tests/**/*.py` (and the sibling `dbt-tests-adapter` base classes the functional tests subclass) into the scaffold; print resolved SHA.
4. **Conftest/profile wiring**: emulator target via `api_endpoint`/`BIGQUERY_EMULATOR_HOST` + anonymous creds; skip plugin for unsupported surfaces (Dataproc/python models, GCS uploads, MV/INFORMATION_SCHEMA gaps).
5. **Task wiring**: `task thirdparty:dbt-bigquery-tests` (nox/pytest via `uvx`), documented skip matrix in `third_party/README.md`.
6. **Triage**: run, build the allowlist/skip set, record passing functional tests.

## Verify

```bash
task emulator:run-full                  # gateway + engine
task thirdparty:dbt-bigquery-tests      # functional tests against the emulator (with skip matrix)
```

## Out of scope

- Python models / Dataproc, GCS file uploads, and any surface the emulator does not implement (document as skips, do not block).
- The bigquery-utils conformance work (plans 01–09).
