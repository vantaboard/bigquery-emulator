---
name: BQUtils 09 — Promote stable subset into the gate
overview: Once a stable bigquery-utils passing/ set exists, promote it into the gating conformance:run (or a CI job) so regressions on real-world UDFs fail PRs. Add CI wiring and a refresh/pinning policy.
depends_on: [bqutils-02-engine-build-triage]
blocks: []
est_effort: 2-3 days
isProject: true
todos:
  - id: stability
    content: Confirm the passing/ set is deterministic across repeated runs (no flakiness from non-deterministic UDFs); quarantine any flaky fixtures.
    status: pending
  - id: pin-ref
    content: Pin the upstream bigquery-utils SHA used for generation (record in third_party/README.md + a manifest) so the gate is reproducible; document the refresh workflow.
    status: pending
  - id: gate-wiring
    content: Decide gate mechanism — fold passing/ into conformance:run, OR add a dedicated CI job running task conformance:bqutils. Wire it into .github/workflows.
    status: pending
  - id: ci
    content: Ensure CI builds emulator_main and runs the chosen gate; tolerate known_failing/ (never run by the gate) and surface its count as informational.
    status: pending
  - id: docs
    content: Update third_party/README.md + conformance docs with the gating policy and how to add/refresh fixtures.
    status: pending
---

# BQUtils 09 — Promote stable subset into the gate

## Why

Plans 01–02 keep the bigquery-utils lane **non-gating** so engine gaps don't break PRs. Once `passing/` is stable (and grows as 03–06 land), promote it so regressions on real-world UDFs are caught.

## Steps

1. **Stability**: run the `passing/` set repeatedly; quarantine any fixture whose result varies (non-deterministic UDFs) into `known_failing/` or a `flaky/` bucket.
2. **Pin the upstream ref**: record the bigquery-utils SHA used for generation in `third_party/README.md` (and/or a small manifest the sync script reads) so regeneration is reproducible. Document the refresh workflow (`conformance:bqutils-sync` at a new ref -> re-triage).
3. **Gate mechanism** (pick one):
   - Fold `passing/` into the default `conformance:run` fixture set, or
   - Add a dedicated CI job that runs `task conformance:bqutils` after `build-engine`.
4. **CI wiring**: build `emulator_main`, run the chosen gate; `known_failing/` is never run by the gate but its count is surfaced informationally.
5. **Docs**: gating policy + add/refresh instructions.

## Verify

```bash
task conformance:bqutils          # green and stable across runs
# CI: the chosen gate job passes on a clean tree and fails if a passing/ fixture regresses
```

## Out of scope

- New engine features (plans 03–06) — this plan only gates what already passes.
