---
name: BigQuery Console UI — Phase Index
overview: Orchestration index for the BigQuery-console-style UI build described in ROADMAP.md. Splits the roadmap milestones (M1-M5 plus the parallel upstream emulator track) into independently-executable .plan.md files, with dependency ordering and per-phase verification gates.
todos:
  - id: m1-01
    content: Execute m1-01-api-and-types.plan.md (expanded REST client + types); verify typecheck/lint.
    status: completed
  - id: m1-02
    content: Execute m1-02-ui-primitives.plan.md (modal, tab bar, detail table, toolbar, Unplanned placeholder); verify typecheck/lint.
    status: completed
  - id: m1-03
    content: Execute m1-03-routing-workspace-breadcrumbs.plan.md (routing, multi-tab workspace, persistence, breadcrumbs); verify e2e + manual reload.
    status: completed
  - id: m2-01
    content: Execute m2-01-dataset-detail-page.plan.md (dataset Overview + Details + Insights placeholder); verify.
    status: completed
  - id: m2-02
    content: Execute m2-02-table-detail-page.plan.md (Schema read, Details, Preview, Query button, Unplanned tabs); verify.
    status: completed
  - id: m3-01
    content: Execute m3-01-create-table-modal.plan.md (Create Table modal, all sources/schema/options); verify.
    status: completed
  - id: m3-02
    content: Execute m3-02-copy-delete-snapshot-schema.plan.md (Copy, Delete, Snapshot, Edit Schema); verify.
    status: completed
  - id: m4-01
    content: Execute m4-01-query-workspace-upgrades.plan.md (SQL Tools client, autocompletion, diagnostics, save, reference panel); verify.
    status: completed
  - id: m5-01
    content: Execute m5-01-routines-and-external-sources.plan.md (routines sub-tab, autocompletion, external ingestion); verify.
    status: pending
  - id: upstream
    content: Track upstream-emulator-work.plan.md in parallel; file issues/PRs against vantaboard/bigquery-emulator.
    status: pending
isProject: false
---

# BigQuery Console UI — Phase Index

Source roadmap: [`ROADMAP.md`](../../ROADMAP.md).

Goal: evolve the single-page explorer into a BigQuery web console–style UI (breadcrumbs, resource detail pages with tabs, action toolbars, multi-tab query workspace, create/copy/snapshot/delete modals, saved queries, routines). Every feature is built to the real BigQuery REST contract; emulator gaps are tracked upstream, not descoped.

## Plan files

| Phase | Plan file | Depends on |
|-------|-----------|------------|
| M1 | [`m1-01-api-and-types.plan.md`](m1-01-api-and-types.plan.md) | — |
| M1 | [`m1-02-ui-primitives.plan.md`](m1-02-ui-primitives.plan.md) | — |
| M1 | [`m1-03-routing-workspace-breadcrumbs.plan.md`](m1-03-routing-workspace-breadcrumbs.plan.md) | m1-01, m1-02 |
| M2 | [`m2-01-dataset-detail-page.plan.md`](m2-01-dataset-detail-page.plan.md) | m1-* |
| M2 | [`m2-02-table-detail-page.plan.md`](m2-02-table-detail-page.plan.md) | m1-* |
| M3 | [`m3-01-create-table-modal.plan.md`](m3-01-create-table-modal.plan.md) | m1-*, m2-01 |
| M3 | [`m3-02-copy-delete-snapshot-schema.plan.md`](m3-02-copy-delete-snapshot-schema.plan.md) | m1-*, m2-* |
| M4 | [`m4-01-query-workspace-upgrades.plan.md`](m4-01-query-workspace-upgrades.plan.md) | m1-* |
| M5 | [`m5-01-routines-and-external-sources.plan.md`](m5-01-routines-and-external-sources.plan.md) | m2-01, m3-01, m4-01 |
| Parallel | [`upstream-emulator-work.plan.md`](upstream-emulator-work.plan.md) | — |

## Execution order

```mermaid
flowchart TD
    A1[m1-01 API and types] --> A3[m1-03 Routing + workspace]
    A2[m1-02 UI primitives] --> A3
    A3 --> B1[m2-01 Dataset page]
    A3 --> B2[m2-02 Table page]
    B1 --> C1[m3-01 Create Table modal]
    B2 --> C2[m3-02 Copy/Delete/Snapshot/Schema]
    B1 --> C2
    A3 --> D1[m4-01 Query workspace upgrades]
    C1 --> E1[m5-01 Routines + external sources]
    D1 --> E1
    U[upstream emulator work] -.parallel.-> A1
```

## Conventions for every sub-plan

- Build to the real BigQuery REST contract. If the emulator lacks a behavior, add an entry to [`upstream-emulator-work.plan.md`](upstream-emulator-work.plan.md) and degrade gracefully (placeholder/error state) rather than removing the UI control.
- Keep existing functionality working: Run query, Format SQL, Results/JSON tabs, sidebar tree, share URL.
- Verification baseline for UI-only phases: `npm run build` (typecheck + bundle) and `npm run lint` must pass; add/adjust Playwright specs under `e2e/` where a flow is user-visible.
- Follow the repo auto-commit rule: commit each logical unit with conventional-commit messages.

## Out of scope (all plans) — Unplanned tabs

Render an **Unplanned** placeholder, do not implement: dataset Insights, Overview Graphs, Overview Models; table/view Table Explorer, Insights, Lineage, Data Profile, Data Quality.
