---
name: ""
overview: ""
todos: []
isProject: false
---

# Migration Cleanup Docs

## Goal

Final pass after the local-execution coordinator and its sibling
plans land: scrub the docs / plan files / code comments for stale
DuckDB-only terminology, ensure the only "execution plans" link in
ROADMAP.md is the new index, and confirm the public docs tell the
same multi-strategy story everywhere.

## Background

The policy-rewrite commit removed the worst of the DuckDB-only
framing from README, ROADMAP, ENGINE_POLICY, and SHAPE_TRACKER. The
plan-inventory commit deleted the old DuckDB-only future plans and
added the local-execution roadmap set. The cross-reference cleanup
commit updated conformance / REST / third-party docs.

This plan is the **final** sweep to catch what the earlier passes
missed: code comments, taskfile descriptions, in-tree READMEs under
`backend/`, CI workflow descriptions, error messages still naming
"DuckDB-only" or "transpiler shape" or `FallbackEngine`,
`.golangci.yml` comments, etc.

## Dependencies

- All other 17 plans in this directory should have landed (or at
  least be far enough along that the docs they touch are stable).

## Scope

Search-and-replace targets:

- Terms to remove or rewrite (full-tree, including code comments
  and CI workflow files):
  - `DuckDB-only` → `local-only execution` / `local coordinator with
    DuckDB storage`.
  - `transpiler shape roadmap` → `local-execution roadmap`.
  - `skiplist` (as a tracker disposition; `skiplist` as a function
    disposition is fine to keep until
    `execution-disposition-registry.plan.md` finishes its migration)
    → matching route disposition.
  - `not_started` → matching route disposition.
  - `FallbackEngine` → drop the reference or mark it historical.
  - `ReferenceImpl` → drop the reference or mark it historical.
  - `all shapes become transpiler work` / equivalents → remove.
  - `every shape lowers to DuckDB SQL` / equivalents → remove.
- Cross-link targets:
  - Every `.cursor/plans/` mention should point to the new local-
    execution index, never to a deleted DuckDB-transpiler plan.
  - Every "execution policy" mention should point to
    `docs/ENGINE_POLICY.md`'s current text, not the
    DuckDB-only-era version.

## Implementation Plan

1. Run a full-tree `rg` for each stale term and triage hit-by-hit.
   For historical-context references (e.g. README's "History"
   section under ENGINE_POLICY), leave the term in place but
   ensure it is clearly marked as historical.
2. Audit `backend/`, `frontend/`, `gateway/`, `conformance/`, and
   `proto/` READMEs for the same terms.
3. Audit `.github/workflows/` for stale workflow descriptions
   referencing "DuckDB engine" instead of "local coordinator."
4. Audit `Taskfile.yml` + every file under `taskfiles/` for stale
   task descriptions.
5. Audit `.cursor/rules/` for stale rule text (in particular any
   rule that still tells contributors to add DuckDB-only transpiler
   plans).
6. Audit code comments in `backend/engine/duckdb/transpiler/` so
   `Emit*` doc comments reference the new route vocabulary.
7. Audit error messages emitted from the coordinator / executors:
   every `UNIMPLEMENTED` message should name the route the shape
   would have used if it had landed, plus the plan file that owns
   it.
8. Confirm ROADMAP.md's "Execution plans" section links only to
   `local-execution-roadmap-index.plan.md` and nothing else under
   `.cursor/plans/`.

## Tests

- A repo-wide `rg` test in CI: any of the stale tokens appearing
  outside an explicitly allowlisted "historical context" file
  fails the build. The allowlist lives at
  `.github/workflows/lint-stale-terms.yml` (or equivalent
  taskfile).
- A doc-render smoke test (existing `task lint:markdown`) is
  still green.

## Done Criteria

- The stale-term linter is green and runs in CI on every PR.
- ROADMAP.md "Execution plans" links only to the new index.
- Code comments, task descriptions, workflow names, and error
  messages all use the multi-strategy vocabulary.
- The only "DuckDB-only" mentions left in the tree are inside
  explicit "History" sections that explain what was removed and
  why.
