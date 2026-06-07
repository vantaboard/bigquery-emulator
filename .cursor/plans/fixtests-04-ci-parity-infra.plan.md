---
name: FixTests 04 — CI / parity infra
overview: Stabilize the googlesql-parity workflow (DuckDB fetch resilience, artifact-download gating, smoke/full split) and make third-party CI signal honest (stop continue-on-error from masking real Java regressions). Infra only - no engine changes.
depends_on: [fixtests-01-foundation]
est_effort: 2-3 days
isProject: true
todos:
  - id: duckdb-fetch
    content: Add retry/mirror for the DuckDB zip http_archive so a transient 504 during Bazel external fetch stops failing the prebuilt parity leg.
    status: pending
  - id: artifact-gating
    content: In googlesql-parity.yml, download conformance JSON artifacts if any leg uploaded them (if always / when upload ran), not only on job success, so the comparator stops reporting neither leg produced a report.
    status: pending
  - id: smoke-split
    content: Split smoke vs full conformance into separate parity jobs so smoke parity can pass even while the full suite is red; narrow scheduled-tier scope until conformance is green (plan 07).
    status: pending
  - id: comparator-ux
    content: Distinguish both legs failed before upload from reports exist but diverge in the comparator output.
    status: pending
  - id: thirdparty-signal
    content: Add a summary step to thirdparty-samples.yml that fails the job on non-allowlisted Java IT failures instead of relying on continue-on-error.
    status: pending
---

# FixTests 04 — CI / parity infra

## Why

Two CI lanes are red for reasons unrelated to engine correctness, and one lane is falsely green:

- **googlesql-parity** fails on transient infra and a workflow design flaw, masking the real parity signal.
- **thirdparty-samples** Java uses `continue-on-error: true`, so a real `exit status 1` concluded the job **success** (run [26860630256](https://github.com/vantaboard/bigquery-emulator/actions/runs/26860630256)).

## googlesql-parity root-cause chain

Latest run [27086058258](https://github.com/vantaboard/bigquery-emulator/actions/runs/27086058258):

| Job | Failed step | Cause |
|-----|-------------|-------|
| build (prebuilt) | Build C++ engine | Transient DuckDB zip **504** (`libduckdb-linux-amd64.zip`) during Bazel external fetch |
| build (source) | Conformance — duckdb (scheduled tier) | Real conformance failures (501/400; e.g. empty `TIMESTAMPTZ ''`) — expected until plan 07 |
| compare | Compare and summarize | Artifacts only downloaded on job `success`; both legs failed -> "neither leg produced a report" |

Workflow flaw in [`.github/workflows/googlesql-parity.yml`](.github/workflows/googlesql-parity.yml): the compare job's download steps are gated `if: needs.build-*.result == 'success'`, so even the source leg's passing smoke JSON (5/5) never reaches the comparator.

### Fixes

1. **DuckDB fetch**: add `http_archive` retry or an internal mirror; make transient 504 re-runnable.
2. **Artifact download**: switch to `if: always()` (or condition on the upload step having run) so partial reports are compared.
3. **Smoke/full split**: separate jobs so smoke parity is measurable independent of the full suite.
4. **Scheduled-tier scope**: narrow the nightly `scheduled` tier (runs the entire `conformance/fixtures` tree) until plan 07 lands, or mark it informational.
5. **Comparator UX** ([`.github/scripts/googlesql_parity_compare.py`](.github/scripts/googlesql_parity_compare.py)): treat "both legs failed before upload" distinctly from "reports diverge".

## Third-party CI signaling

[`thirdparty-samples.yml`](.github/workflows/thirdparty-samples.yml): Java live IT step is `continue-on-error: true`. Allowlist is 9 gRPC `UNIMPLEMENTED` ITs (in [`third_party/README.md`](third_party/README.md)); `QueryMaterializedViewIT` is **not** allowlisted and must pass.

- Add a summary step that parses results and **fails** the job on any non-allowlisted IT failure, replacing the blanket `continue-on-error`.
- Keep allowlisted failures non-fatal until their features land (storage gRPC tracked separately).

## Verify

- Re-run `googlesql-parity`: prebuilt leg survives a DuckDB fetch hiccup; compare job emits real smoke parity even when full conformance is red.
- Force a non-allowlisted Java IT failure (or dry-run the summary parser) and confirm the job goes red.

## Out of scope

- Engine fixes for the source-leg conformance failures (plans 02, 05–07) and the full third-party feature work (plan 14). This plan only fixes CI plumbing and signal honesty.
