---
name: M1-03 — Routing, multi-tab workspace, persistence, breadcrumbs
overview: Replace the single-page model with console-style routing, a persistent multi-tab workspace (many query + resource tabs) whose full session is stored in localStorage, and a clickable breadcrumb trail. This is the structural backbone that M2+ resource pages plug into.
todos:
  - id: routes
    content: Add resource routes (dataset, table/view) in src/app/router.tsx and a workspace shell layout
    status: pending
  - id: workspace-store
    content: Add a workspace tab store (open/activate/rename/reorder/close, +new query) with localStorage persistence
    status: pending
  - id: tab-bar
    content: Render the multi-tab bar using the Tabs primitive; wire sidebar clicks and Query button to open tabs
    status: pending
  - id: query-tab
    content: Extract the SQL editor + Results/JSON panel into a self-contained QueryTab owning its own state
    status: pending
  - id: breadcrumbs
    content: Add a Breadcrumbs component driven by route params on resource pages
    status: pending
  - id: persistence-migrate
    content: Persist tabs/order/active + per-tab SQL/sub-tab; keep URL as shareable active-tab deep link; migrate bigqueryExplorerUILayout
    status: pending
  - id: e2e
    content: Add Playwright coverage for open-multiple-tabs and reload-restores-session
    status: pending
isProject: false
---

# M1-03: Routing, multi-tab workspace, persistence, breadcrumbs

## Dependencies

- [`m1-01-api-and-types.plan.md`](m1-01-api-and-types.plan.md) (typed data access)
- [`m1-02-ui-primitives.plan.md`](m1-02-ui-primitives.plan.md) (Tabs, etc.)
- Index: [`00-index.plan.md`](00-index.plan.md)
- Roadmap: [`ROADMAP.md`](../../ROADMAP.md) → Architectural prerequisites (routing, multi-tab workspace, persistence) + Feature areas → Breadcrumbs, Multi-tab workspace.

## Goal

Turn the app into a tabbed workspace shell: many query and resource tabs open at once, the whole session restored on reload, and breadcrumb navigation on resource pages. Preserve all current behavior (run/format/results/JSON/share).

## Current state

- Single route `/` → `ExplorerPage` ([src/app/router.tsx](../../src/app/router.tsx)).
- [ExplorerPage.tsx](../../src/features/explorer/ExplorerPage.tsx) holds global `currentProject/Dataset/Table`, one `sql`, one `queryResult`, one `activeTab` (`info`/`results`/`json`), and `UiPrefs` persisted under `bigqueryExplorerUILayout`.
- URL state via [urlState.ts](../../src/features/explorer/urlState.ts): project/dataset/table/results/base64 query.

## Scope

### 1. Routing — [src/app/router.tsx](../../src/app/router.tsx)

- Workspace shell layout with persistent sidebar + tab bar + active-tab outlet.
- Routes:
  - `/` — empty workspace / last session.
  - `/project/:projectId/dataset/:datasetId` — dataset page (M2-01).
  - `/project/:projectId/dataset/:datasetId/table/:tableId` — table/view page (M2-02).
  - `/query/:tabId` (or query string) — active query tab.
- Active tab drives the URL; opening/closing tabs updates the store, not necessarily the URL.

### 2. Workspace tab store

- New module (e.g. `src/features/workspace/store.ts`) holding:
  - `tabs: WorkspaceTab[]` where a tab is a discriminated union `{ type: 'query' | 'dataset' | 'table' }`.
  - `activeTabId`, ordered `tabs`.
  - Query tab state: `title`, `sql`, `subTab` (`results`|`json`), cached `results` (optional).
  - Resource tab state: project/dataset/(table), `activeResourceTab`.
- Actions: `openQueryTab(initialSql?)`, `openResourceTab(ref)`, `activateTab`, `renameTab`, `reorderTab`, `closeTab`, `newBlankQuery`.
- Implement with React context + reducer (no new dependency) or a tiny store; keep it framework-light.

### 3. Persistence

- Persist `{ tabs, tabOrder, activeTabId }` and per-query-tab `{ sql, subTab }` to `localStorage` (new key, e.g. `bigqueryWorkspaceSession`).
- Fold existing `bigqueryExplorerUILayout` (sidebar width/collapse, editor height) into or alongside the session; keep backward-compatible read.
- On load, hydrate the workspace from storage; reload restores the exact set of open tabs, order, active tab, and each query tab's SQL + sub-tab.
- Keep [urlState.ts](../../src/features/explorer/urlState.ts) as the **shareable** encoding for the active tab only (deep link); extend if needed but do not move full session into the URL.

### 4. Tab bar + query tab

- Render the multi-tab bar with the `Tabs` primitive (closable variant) plus a `+` to open a blank query tab.
- Extract the editor + Results/JSON panel from `ExplorerPage` into a `QueryTab` component that reads/writes its own slice of the store (reuse [SqlEditor.tsx](../../src/features/explorer/components/SqlEditor.tsx), [ResultsTable.tsx](../../src/features/explorer/components/ResultsTable.tsx), [JsonViewer.tsx](../../src/features/explorer/components/JsonViewer.tsx) and the existing `runMutation`/`onFormat`/`onRun` logic, scoped per tab).
- Sidebar table click opens (or focuses) a resource tab; the resource page's **Query** button (M2) opens a new query tab via `openQueryTab(defaultSql(...))`.

### 5. Breadcrumbs

- New `Breadcrumbs` component on resource pages:
  - Dataset: `{project} / Datasets / {dataset}`.
  - Table/view: `{project} / Datasets / {dataset} / Tables / {table}`.
- Each segment is a link to the corresponding route.

## Out of scope

- Dataset/table page tab content (M2). This plan can render placeholder bodies for resource tabs until M2 lands.
- Mutations, saving queries, autocompletion (M3/M4).

## Verification

```bash
npm run build
npm run lint
npm run test:e2e   # includes new multi-tab + reload specs
```

Manual: open several tables and `+` query tabs, reorder/close, reload the page → workspace restores; Share copies a working deep link for the active tab.

## Done criteria

- Multiple query and resource tabs can be open simultaneously, each with independent state.
- Full session persists across reload via `localStorage`.
- Breadcrumbs render and navigate on resource pages.
- Run/Format/Results/JSON/Share still work (now per query tab).
- Existing e2e specs updated to the new structure and passing.
