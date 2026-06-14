---
name: Expand — dispatch playbook
overview: Serialized engine-lane runbook for the expand 01-09 feature set. Documents the per-subagent session protocol, the bazel single-invocation + process-hygiene constraints, the hot-file list to watch for cross-plan conflicts, and the parent-agent cleanup between subagents. Mirrors full-dispatch / parity-dispatch for the third wave.
isProject: true
---

# Expand — dispatch playbook

Runbook for draining [expand-00-index.plan.md](expand-00-index.plan.md).
Every expand plan rebuilds the C++ engine (and plan 01 the Go gateway),
so the engine lane is **serialized**: one plan in flight at a time.

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

- `backend/engine/duckdb/transpiler/node_dispositions.yaml` — 02, 04, 05, 06, 07, 08, 09
- `backend/engine/duckdb/transpiler/functions.yaml` — 02, 08
- `backend/engine/duckdb/transpiler/SHAPE_TRACKER.md` — all
- `backend/engine/coordinator/route_classifier*.cc` — any plan adding a pre-dispatch route (02, 05)
- `backend/engine/semantic/functions/dispatch.cc` — 08, 09
- `docs/ENGINE_POLICY.md`, `ROADMAP.md` — all (bookkeeping)

Because the lane is serialized, conflicts are avoided by committing each
plan before dispatching the next; never run two engine plans in parallel.

## Parent-agent cleanup (between every subagent)

Run the full cleanup block from `process-hygiene.mdc` (bazel server +
clients, `emulator_main` / `gateway_main`, conformance runner, docker if
used), then re-run the pre-spawn audit + `free -h` before the next
subagent. Do not trust a subagent's "I cleaned up" — verify.

## Out of scope for this playbook

- The per-feature design decisions (those live in each expand-NN plan).
- Non-engine docs-only follow-ups (handle inline; no separate dispatch).
