---
name: Expand — dispatch playbook
overview: Serialized engine-lane runbook for the expand feature set (plans 01-09, with 05 removed — Graph/GQL is out of scope). Documents dispatch waves, per-subagent session protocol, the bazel single-invocation + process-hygiene constraints, the hot-file list to watch for cross-plan conflicts, and the parent-agent cleanup between subagents. Mirrors full-dispatch / parity-dispatch for the third wave.
todos:
  - id: preflight
    content: "Pre-flight before wave 1: confirm main is clean, `task lint:dispositions` green, `task bazel:status` -> `(clean)`, `free -h` available > 4 GiB. Re-read expand-00-index + ROADMAP §Planned work + ENGINE_POLICY §Unsupported families."
    status: pending
  - id: wave1-stubs
    content: "Wave 1 — stub warm-up (SERIALIZED): dispatch expand-02-bigquery-ml (ML.* placeholders) -> expand-06-privacy-aggregates (strip DP modifiers, plain aggregate). Cheap no-fail stubs; establishes the stub lane before heavier work. Parent cleanup + expand-00 status update between each."
    status: pending
  - id: wave2-external
    content: "Wave 2 — external data (SERIALIZED): dispatch expand-01-external-data-sources (gs:// LOAD/EXPORT, Google Sheets, connections/EXTERNAL_QUERY, tableDefinitions; fixture vs opt-in live config). Touches Go gateway + engine. Parent cleanup + status update after."
    status: pending
  - id: wave3-language-surfaces
    content: "Wave 3 — language surfaces (SERIALIZED): dispatch expand-03-python-udf-runtime -> expand-04-protobuf-shapes. Independent but both are multi-week real implementations; run one at a time. Parent cleanup + status update between each."
    status: pending
  - id: wave4-catalog-measures
    content: "Wave 4 — catalog + measures (SERIALIZED): dispatch expand-07-sequences-catalog-refs -> expand-09-measure-functions. Parent cleanup + status update between each."
    status: pending
  - id: wave5-scalar
    content: "Wave 5 — scalar/statement long tail (SERIALIZED): dispatch expand-08-scalar-statement-long-tail (real: ST_GEOGFROMWKB + EXPLAIN; stub: KEYS encrypt/decrypt + SESSION_USER). Parent cleanup + status update after."
    status: pending
  - id: track
    content: "After each subagent returns: run the parent cleanup block (process-hygiene.mdc), flip the matching wave todo here to completed, update the expand-00-index status table (state / conformance delta / commits / notes), verify `task lint:dispositions` green on main, and confirm `task bazel:status` -> `(clean)` before the next dispatch."
    status: pending
  - id: closeout
    content: "Closeout when all waves land: final cross-cutting skip audit (each plan's `skip-audit` todo runs per-plan; this is the catch-all). Sweep third-party skip matrices (`third_party/*/emulator_*skip*`, `third_party/README.md`, `node-bigquery-tests/EMULATOR.md`), the bqutils corpus (`conformance/thirdparty-fixtures/bigquery_utils/known_failing/` -> `passing/`), and the GoogleSQL `.test` corpus (`conformance/googlesql-corpus/`) for shapes now unblocked; run `task conformance:routing-matrix`; final `task bazel:shutdown` + `task bazel:status`; mark any remaining ROADMAP §Planned work bullets ✅ or document blockers."
    status: pending
isProject: true
---

# Expand — dispatch playbook

Runbook for draining [expand-00-index.plan.md](expand-00-index.plan.md).
Every expand plan rebuilds the C++ engine (and plan 01 the Go gateway),
so the engine lane is **serialized**: one plan in flight at a time.
There is no `expand-05-*`: Graph / GQL is `unsupported` and not planned.

## Dispatch waves

Execute the `todos` in this file's frontmatter in order. **Every wave is
serialized** — one subagent at a time, parent cleanup between each.

| Wave | Todo id | Plans (in order) | Kind | Notes |
|------|---------|------------------|------|-------|
| — | `preflight` | — | — | Run once before wave 1 |
| 1 | `wave1-stubs` | [02](expand-02-bigquery-ml.plan.md) → [06](expand-06-privacy-aggregates.plan.md) | stub | Warm-up; cheap no-fail placeholders |
| 2 | `wave2-external` | [01](expand-01-external-data-sources.plan.md) | real | Go gateway + engine; highest workload impact |
| 3 | `wave3-language-surfaces` | [03](expand-03-python-udf-runtime.plan.md) → [04](expand-04-protobuf-shapes.plan.md) | real | Python UDF runtime, then proto surface |
| 4 | `wave4-catalog-measures` | [07](expand-07-sequences-catalog-refs.plan.md) → [09](expand-09-measure-functions.plan.md) | real | Catalog refs, then MEASURE types |
| 5 | `wave5-scalar` | [08](expand-08-scalar-statement-long-tail.plan.md) | mixed | real ST_GEOGFROMWKB + EXPLAIN; stub KEYS + SESSION_USER |
| — | `track` | — | — | After **every** subagent return |
| — | `closeout` | — | — | Once all waves complete |

> **05 (Graph / GQL)** is intentionally absent — not planned.

Stub waves (1) can be reordered relative to wave 2 if you prefer to land
external data first; the only hard constraint is **one engine build at a
time** and **parent cleanup between subagents**.

## Session protocol (per sub-plan)

1. **Pre-flight.** Confirm main is clean and the prior plan is committed:
   - `rtk git status` clean (or only the current plan's edits).
   - `task lint:dispositions` green (no tracker drift left behind).
   - Process / bazel audit per `process-hygiene.mdc` and
     `bazel-process-hygiene.mdc`:

     ```bash
     task bazel:status            # expect (clean)
     free -h | head -2            # expect available > 4 GiB
     ```

2. **Re-read the trackers** named in the index (`ROADMAP.md` §Planned
   work, `docs/ENGINE_POLICY.md` §Unsupported families,
   `node_dispositions.yaml` / `functions.yaml`, `SHAPE_TRACKER.md`).
   These are authoritative and edited in the same commit as the code.

3. **Implement** following the sub-plan's `todos`. Keep the change set
   scoped to one feature family. Land the handler + conformance
   fixture(s) + tracker edits together (promotion policy).

4. **Verify** with the index's verification matrix. New fixtures must
   pass; no first-party conformance regressions.

5. **Bookkeeping** (index §Bookkeeping per landed plan): flip the
   `unsupported` rows, update ROADMAP / ENGINE_POLICY posture, drop any
   third-party skip-matrix rows the shape unblocks.

6. **Commit** per the auto-commit workflow (conventional commits, group
   handler + test + tracker edits). Then end the bazel daemon:

   ```bash
   task bazel:shutdown && task bazel:status   # expect (clean)
   ```

## Subagent prompt template

> Implement [`expand-NN-<theme>.plan.md`](expand-NN-<theme>.plan.md) end
> to end. Re-read the trackers it names first. Land the engine handler
> (+ gateway changes for plan 01), conformance fixtures, and the
> `node_dispositions.yaml` / `functions.yaml` + `SHAPE_TRACKER.md` edits
> in the same commit(s). Run the index verification matrix and report
> the conformance delta + commit SHAs. Obey `process-hygiene.mdc` and
> `bazel-process-hygiene.mdc`: one bazel invocation at a time, end with
> `task bazel:shutdown` + `task bazel:status` -> `(clean)`.

- `subagent_type: generalPurpose`, `run_in_background: false`.

## Bazel single-invocation invariant

- Build only through `task emulator:build-engine:bazel`; test through
  `task bazel:test`. Never spawn a second bazel against this workspace.
- Do **not** `task bazel:shutdown` between scoped tests inside one
  plan (warm-daemon-per-plan); shut down only at the end of the plan.
- If a build looks stuck, `task bazel:status` first; abandon with
  `task bazel:kill-strays` (kills the whole tree), never `kill -9` the
  daemon alone.

## Hot files (watch for cross-plan conflicts)

These are touched by more than one expand plan; coordinate so two
serialized plans don't clobber each other's edits:

- `backend/engine/duckdb/transpiler/node_dispositions.yaml` — 04, 06, 07, 08, 09
- `backend/engine/duckdb/transpiler/functions.yaml` — 02, 08
- `backend/engine/duckdb/transpiler/SHAPE_TRACKER.md` — all
- `backend/engine/coordinator/route_classifier*.cc` — any plan adding a route/stub dispatch (02, 06, 08)
- `backend/engine/semantic/stubs/` — 02, 08 (ML, KEYS, SESSION_USER stub lane)
- `backend/engine/semantic/functions/dispatch.cc` — 08, 09
- `docs/ENGINE_POLICY.md`, `ROADMAP.md` — all (bookkeeping)

Because the lane is serialized, conflicts are avoided by committing each
plan before dispatching the next; never run two engine plans in parallel.

## Parent-agent cleanup (between every subagent)

Run the full cleanup block from `process-hygiene.mdc` (bazel server +
clients, `emulator_main` / `gateway_main`, conformance runner, docker if
used), then re-run the pre-spawn audit + `free -h` before the next
subagent. Do not trust a subagent's "I cleaned up" — verify.

## Progress tracking

Maintain the status table in
[expand-00-index.plan.md](expand-00-index.plan.md). Update after each
subagent returns, alongside flipping this file's wave `todos` (`track`
todo). Record: plan state, conformance pass delta, commit SHAs, and any
blockers.

## Anti-patterns to avoid

- Fanning out multiple expand plans as concurrent subagents — they are
  dependency-independent, not bazel-independent or merge-independent.
- Dispatching the next wave before `track` confirms the prior plan is
  committed, `task lint:dispositions` is green, and `task bazel:status`
  is `(clean)`.
- Promoting a row off `unsupported` without the implementation (or stub)
  + conformance fixture in the same commit.
- Skipping the parent cleanup block because the subagent reported success.
- Running third-party skip-matrix removal before the unblocking shape has
  landed on main.

## Out of scope for this playbook

- The per-feature design decisions (those live in each expand-NN plan).
- Non-engine docs-only follow-ups (handle inline; no separate dispatch).
