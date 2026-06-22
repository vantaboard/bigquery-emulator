# 02 — Stateful / multi-step session harness (+ mid-session restart)

- **Series:** conformance-hardening (plans 01–08). Run sequentially.
- **Sequencing:** second. Independent of plan 01, but plan 07 (regression
  fixtures) and plan 08 (persistence) both reuse the session harness this plan
  introduces. Build it before them.
- **Priority:** P1.

## Why this exists (origin)

The most dangerous reported defect was an **engine abort** that only appeared
"after my test suite repeats some of the work as setup/teardown; even though
individual tests may work standalone." The trigger was a sequence: create view →
`update_dataset(["access"])` to authorize the view on a source dataset → repeat
across tenants → the engine `CHECK`-failed on a duplicate catalog entry and the
whole subprocess died (`signal: aborted`). A sibling report: after a fix, the
dataset listing went **empty of tables** — a state regression only visible across
operations.

The existing conformance harness **spawns a fresh `emulator_main` per fixture,
runs one query, and tears down** (`conformance/README.md` Quick start). That
single-shot, stateless design *structurally cannot* observe sequence-dependent
bugs, idempotency failures, catalog-replay corruption, or anything that needs the
same engine alive across many operations. This plan adds that missing dimension.

## Current state (grounded)

- `conformance/runner/` runs exactly one `query:` per fixture after an ordered
  `setup:` list, against a per-fixture engine process.
- `setup:` already supports ordered steps (`dataset`, `table`, `rows`, `sql`) —
  the building blocks for a sequence exist; what's missing is **multiple asserted
  queries against one long-lived engine** and a **restart** step.
- `gateway/e2e/` has end-to-end Go tests against a real engine (e.g.
  `ddl_create_drop_test.go`) but they are bespoke, not a declarative sequence
  format.

## Goal / done-criteria

1. A **session fixture** format: an ordered list of `steps`, each step being a
   REST op (dataset/table/rows/sql/query) **or** an assertion (`expect_rows`,
   `expect_error`, `expect_table_list`, `expect_alive`), all executed against a
   **single** emulator process that stays up for the whole session.
2. A `restart` step that gracefully stops and restarts the engine (same
   `--data-dir`) mid-session, so steps after it assert post-restart state (feeds
   plan 08).
3. An `expect_alive` assertion that fails the session if the engine subprocess
   has died (`signal: aborted` / non-zero exit) at any point — turning the
   email's crash into a red test (coordinate with plan 03).
4. A `repeat: N` wrapper on a step group so "create/authorize/drop ×N" reproduces
   the duplicate-catalog abort.
5. `task conformance:session` runs all session fixtures; CI job added.
6. Seeded with a session reproducing the authorize-view-then-repeat sequence and
   one reproducing the "dataset listing goes empty after view op" regression.

## Implementation steps

### Step 1 — Session fixture schema
- New dir `conformance/sessions/`. One YAML per session:
  ```yaml
  name: authorize_view_repeat
  steps:
    - dataset: ds_main
    - table: { dataset: ds_main, id: profiles, schema: [...] }
    - rows:  { dataset: ds_main, table: profiles, rows: [...] }
    - sql: "CREATE VIEW ds_tenant.v AS SELECT * FROM ds_main.profiles"
    - rest: { method: PATCH, path: ".../datasets/ds_main", body: { access: [...view entry...] } }
    - repeat: 5
      steps:
        - sql: "CREATE OR REPLACE VIEW ds_tenant.v AS SELECT * FROM ds_main.profiles"
        - rest: { method: PATCH, path: ".../datasets/ds_main", body: {...} }
        - expect_alive: true
    - query: "SELECT COUNT(*) AS n FROM ds_tenant.v"
      expect_rows: [{ n: "3" }]
    - expect_table_list: { dataset: ds_main, contains: [profiles] }
  ```

### Step 2 — Session executor (`conformance/runner` extension or
`conformance/cmd/session`)
- Start one engine, keep its `*exec.Cmd` handle, and after every step poll
  `cmd.ProcessState`/the process to implement `expect_alive`.
- Add a generic `rest:` step (method+path+body) so ACL PATCH and other non-query
  REST ops are expressible without new step kinds per API.
- Reuse the typed-cell comparator for `expect_rows` and the error matcher for
  `expect_error`.
- Implement `restart`: send SIGTERM, wait for exit, relaunch with identical args
  (same `--data-dir`/project), reconnect.

### Step 3 — Task + CI
- `conformance:session` in `taskfiles/conformance.yml`.
- CI job after `build-engine`. Document in `conformance/README.md` ("Session
  fixtures" section) noting these are deliberately stateful and long-lived.

### Step 4 — Seed sessions
- `authorize_view_repeat` (reproduces the duplicate-catalog abort).
- `dataset_list_after_view_op` (asserts tables remain listed after creating an
  authorized view).
- `view_select_after_create_via_client_path` (create + immediately SELECT).

## Tests
- Unit: session schema loader + each assertion kind (golden pass/fail).
- The seeded crash-repro session should be **red until plan 03** lands the
  Status conversion (or red until the engine bug is fixed); mark it
  `known_failing: true` with a pointer to plan 03 so CI stays green meanwhile,
  exactly like the bqutils `known_failing/` pattern.

## Process hygiene (repo rules)
Long-lived + restart steps spawn engines repeatedly. Follow
`.cursor/rules/process-hygiene.mdc`: the executor MUST reap every engine it
starts (the runner already installs SIGINT/SIGTERM handlers — extend them to the
restart-spawned children). Run the cleanup block after the lane.

## Out of scope
- Concurrency/parallel-client stress (separate effort; this is ordered
  single-client sequences).
- Persistence semantics themselves — plan 08 owns the restart *durability*
  assertions; this plan only provides the `restart` primitive.

## Touch list
`conformance/sessions/*`, `conformance/runner/` (session executor + `rest:` /
`restart` / `expect_alive` / `repeat`), optionally
`conformance/cmd/session/main.go`, `taskfiles/conformance.yml`,
`.github/workflows/conformance.yml`, `conformance/README.md`.
