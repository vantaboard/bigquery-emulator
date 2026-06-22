# 08 — Persistence / restart durability + `--database`→`--data-dir` migration UX

- **Series:** conformance-hardening (plans 01–08). Run sequentially (last).
- **Sequencing:** eighth. Uses the `restart` session primitive from plan 02 for
  the durability assertion. Do plan 02 first.
- **Priority:** P2.

## Why this exists (origin)

On the v0.5.0 image the user reported:

> "First run shows that views are not persisting through container start/stop,
> but maybe I'm misusing something?"

…while migrating a compose file from the recidiviz fork. Two things are tangled:

1. A **durability** question — do view definitions survive an engine/container
   restart against the same data dir?
2. A **migration footgun** — the old setup used `--database=/opt/x.db` (a single
   sqlite-style file); the new emulator uses `--data-dir=/opt`. The user changed
   the flag and re-pointed the same volume, with no signal about whether old
   state is recognized or silently ignored.

Both need to become explicit: a tested restart round-trip, and a clear
startup contract/warning for the flag change.

## Current state (grounded)

- View persistence already exists in some form: `backend/catalog/view_persistence.{cc,h}`,
  `gateway/handlers/views_catalog.go`, and a test
  `gateway/e2e/view_persistence_test.go`. Routine persistence has a parallel:
  `backend/catalog/routine_persistence.{cc,h}`, `gateway/e2e/routine_persistence_test.go`.
- Flag/CLI wiring: `binaries/gateway_main/cli.go` (+ `cli_test.go`), `gateway/server.go`,
  `gateway/gateway.go`. Data-dir/seed handling: `gateway/seedfile/`, `gateway/seed/`.
- What's unproven: a **full restart round-trip** that creates datasets/tables/
  **views via the client path**, restarts, and asserts everything (especially
  client-created views, per R2) survives — and a **migration/warning** when an
  old `--database` file path or pre-existing incompatible volume layout is given.

## Goal / done-criteria

1. A restart-durability test (engine-level + e2e) proving that after
   `--data-dir` boot → create datasets/tables/rows/**views (client path)** →
   graceful restart with the same `--data-dir` → all objects and view
   definitions are present and `SELECT` through views returns the same rows.
2. The plan 02 `restart` session covering the same flow declaratively (and
   pinned as R5 in plan 07).
3. A startup **migration contract**: if `--database <file>` is passed (legacy)
   or a data dir contains an unrecognized/legacy layout, the emulator emits a
   clear, actionable log line (what changed, what to do) and either migrates or
   fails loudly — never silently starts empty.
4. Docs updated: the `--database`→`--data-dir` migration is called out in
   `docs/REST_API.md` / `ROADMAP.md` ops notes and any compose/Docker guidance.

## Implementation steps

### Step 1 — Audit current persistence coverage
- Read `gateway/e2e/view_persistence_test.go` and `backend/catalog/view_persistence.cc`.
  Determine whether **client-created** views (not just `tables.insert` or REST
  views) are persisted and rehydrated — R2 showed the create path matters.
  Close any gap so all create paths persist identically.

### Step 2 — Restart round-trip test
- Extend `gateway/e2e/view_persistence_test.go` (or add
  `gateway/e2e/restart_durability_test.go`) to: boot with a temp `--data-dir`,
  create dataset+table+rows+view via the gateway, **stop and restart** the engine
  with the same dir, then assert `tables.list`/`tables.get` and a `SELECT`
  through the view all succeed with identical data.
- Add the declarative equivalent as `conformance/sessions/restart_view_durability.yaml`
  using plan 02's `restart` step.

### Step 3 — Migration contract + warning
- In `binaries/gateway_main/cli.go`: when `--database` is supplied, log a clear
  deprecation/migration message (old single-file vs new `--data-dir`), and either
  (a) accept + adapt it, or (b) reject with guidance — decide based on whether
  the old format is loadable. Cover both with `cli_test.go` cases.
- On `--data-dir` startup, if the dir contains a recognizable legacy/foreign
  layout, warn rather than silently ignoring it.

### Step 4 — Docs
- Document the migration in `docs/REST_API.md` (ops/flags section) and add a
  compose snippet showing the recidiviz-fork → this-emulator flag mapping
  (`--database=/opt/x.db` → `--data-dir=/opt`), mirroring the user's exact case.

## Tests
- `gateway/e2e/restart_durability_test.go` (or extended view_persistence test):
  client-created view survives restart.
- `binaries/gateway_main/cli_test.go`: `--database` legacy flag emits the
  migration message and behaves as decided; `--data-dir` with legacy layout
  warns.
- `conformance/sessions/restart_view_durability.yaml` green (pins R5).

## Process hygiene (repo rules)
e2e + session tests spawn the gateway and engine. Follow
`.cursor/rules/process-hygiene.mdc`: pre-spawn audit, and the emulator/gateway
kill + (if docker used) `docker compose down` cleanup block after the run. C++
engine changes follow the bazel hygiene rule.

## Out of scope
- A general schema-migration framework for the on-disk format (only the
  legacy-flag detection + clear messaging here).
- ARM/`linux/amd64` platform issues from the user's compose file (separate; note
  it in docs if trivial).

## Touch list
`backend/catalog/view_persistence.cc` / `.h`, `gateway/handlers/views_catalog.go`,
`gateway/e2e/view_persistence_test.go` (+ new `restart_durability_test.go`),
`binaries/gateway_main/cli.go` / `cli_test.go`, `gateway/server.go`,
`conformance/sessions/restart_view_durability.yaml`,
`docs/REST_API.md`, `ROADMAP.md`.
