# 07 — Pin every reported bug as a permanent regression fixture

- **Series:** conformance-hardening (plans 01–08). Run sequentially.
- **Sequencing:** seventh. Depends on the harnesses from plans 01 (differential),
  02 (sessions), and 03 (`expect_alive`). Run after those exist so each repro can
  land in the right lane.
- **Priority:** P1 (cheap, high-value; stops the observed fix-then-regress churn).

## Why this exists (origin)

The email thread shows a **fix → re-break → re-fix** cycle across releases
(v0.3.0 → v0.3.1 → v0.4.0 → v0.5.0): the first issue was "resolved," then views
broke a different way, then a fix made the dataset listing go empty, etc. That
ping-pong is the signature of fixes that were **not pinned by a committed test**.
Every defect a user reports should become a permanent regression fixture in the
same change as its fix, so it cannot silently come back. This plan backfills
fixtures for all reported defects and establishes the policy going forward.

## The reported defects to pin (from the thread)

| Tag | Defect | Best lane |
|-----|--------|-----------|
| R1 | `CREATE OR REPLACE TABLE \`ds.t_dedup\`` → "DDL target name … got 1 segment … no defaultDataset" (backtick `dataset.table` parsed as one segment) | `conformance/fixtures/core_usage/qualified_names/` |
| R2 | View created via client returns 0 rows (worked in UI) | `conformance/fixtures/core_usage/views/` + plan 05 scenario |
| R3 | After view fix, dataset `tables.list` returns empty | plan 02 session `dataset_list_after_view_op` |
| R4 | Engine **abort** on duplicate catalog name during view replay (authorize-view ×N) | plan 02 session `authorize_view_repeat` + plan 03 crash test |
| R5 | Views not persisted across container restart | plan 02 `restart` session + plan 08 |
| R6 | Naive TIMESTAMP param `'2026-06-22T10:00:00'` rejected | plan 04 matrix + differential corpus |
| R7 | `UNION DISTINCT` → "SetOperationScan op is not UNION ALL" | `conformance/fixtures/setops/` (extend) + differential corpus |
| R8 | CTE referenced in subquery → "WithRefScan without active WithScan bindings" | `conformance/fixtures/cte_subquery/` (extend) + differential corpus |
| R9 | Anti-join over QUALIFY-deduped views → DuckDB binder "column id not found" | plan 06 composition test + differential corpus |

## Current state (grounded)

- Relevant fixture homes already exist:
  `conformance/fixtures/core_usage/{views,qualified_names,ddl_lifecycle,default_dataset}/`,
  `conformance/fixtures/setops/`, `conformance/fixtures/cte_subquery/`.
- `core_usage` fixtures require production-validated expectations and are
  protected from `--update-baselines`
  (`conformance/fixtures/core_usage/README.md`).
- No single index ties "reported bug → pinning test" together today.

## Goal / done-criteria

1. Each of R1–R9 has at least one committed regression artifact in the lane named
   above, with a top-of-file comment: the symptom, the reported version, and a
   link back to this plan.
2. Expected values are production-validated (plan 01 oracle / `bq`), never
   bootstrapped from emulator output (per `core_usage` rule).
3. A regression index `conformance/REGRESSIONS.md` mapping each tag (R1…) to its
   fixture path(s) and the fixing commit/plan.
4. A standing policy (rule doc): a user-reported bug fix must include its pinning
   fixture in the same PR.

## Implementation steps

### Step 1 — Author the fixtures/sessions per the table
- R1, R2, R7, R8: declarative YAML fixtures in the listed dirs (setup + query +
  expected). Validate SQL against BigQuery first
  (`.cursor/rules/conformance-bq-validation.mdc`).
- R3, R4, R5: session fixtures (plan 02) under `conformance/sessions/`.
- R6: parameter matrix cases (plan 04) + differential corpus.
- R9: transpiler composition test (plan 06) + differential corpus.

### Step 2 — Handle not-yet-fixed repros honestly
- If the engine fix for a repro hasn't landed, commit the fixture as
  `known_failing` (mirroring `conformance/thirdparty-fixtures/bigquery_utils/known_failing/`)
  or `xfail`, with a comment pointing at the owning plan (03/06). Never delete a
  repro to make CI green; flip it to passing when the fix lands.

### Step 3 — Regression index + policy
- Write `conformance/REGRESSIONS.md` (the R1…R9 table with live paths).
- Add `.cursor/rules/pin-reported-bugs.mdc`: "a fix for a reported defect must
  add a regression fixture/test in the same PR; record it in
  `conformance/REGRESSIONS.md`." Reference this plan.

### Step 4 — Verify the suite
- Run `task conformance:run`, `task conformance:session`,
  `task conformance:differential` and confirm the intended pass/known-failing
  state for each tag.

## Tests
- The fixtures/sessions themselves. Plus a meta-check (optional): a small script
  asserting every tag in `REGRESSIONS.md` resolves to an existing fixture path
  (wire into `task conformance:routing-matrix`-style tooling or a tiny Go test).

## Process hygiene (repo rules)
Running the conformance/session/differential lanes spawns engines — follow
`.cursor/rules/process-hygiene.mdc` (audit + cleanup block) around each run.

## Out of scope
- Fixing the underlying engine bugs (owned by plans 03/04/06 and the roadmap);
  this plan **pins** them. Where a fix is trivial and lands here, group the fix +
  fixture per the auto-commit rule.

## Touch list
`conformance/fixtures/core_usage/{views,qualified_names}/*`,
`conformance/fixtures/setops/*`, `conformance/fixtures/cte_subquery/*`,
`conformance/sessions/*`, `conformance/differential/corpus/*`,
`conformance/REGRESSIONS.md`, `.cursor/rules/pin-reported-bugs.mdc`.
