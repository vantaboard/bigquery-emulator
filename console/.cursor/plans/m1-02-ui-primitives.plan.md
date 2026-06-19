---
name: M1-02 — Reusable UI primitives
overview: Build the shared presentational components every later phase depends on — modal/dialog, tab bar, key-value detail table, action toolbar, copy-to-clipboard helper, and an Unplanned tab placeholder — matching the existing Tailwind/CSS-variable styling in the app.
todos:
  - id: modal
    content: Add Modal/Dialog primitive (overlay, focus trap, header/body/footer, close) under src/components/ui
    status: pending
  - id: tabbar
    content: Add TabBar/Tabs primitive supporting active state, optional close buttons, and overflow
    status: pending
  - id: detail-table
    content: Add DetailTable (key-value rows) and SectionHeading for Details tabs
    status: pending
  - id: toolbar
    content: Add ActionToolbar + ToolbarButton (icon + label) for resource action rows
    status: pending
  - id: misc
    content: Add CopyButton (clipboard), UnplannedTab placeholder, and ConfirmDialog wrappers
    status: pending
isProject: false
---

# M1-02: Reusable UI primitives

## Dependencies

- None (parallel with m1-01).
- Index: [`00-index.plan.md`](00-index.plan.md)
- Roadmap: [`ROADMAP.md`](../../ROADMAP.md) → Architectural prerequisites → "Reusable UI primitives".

## Goal

Provide consistent building blocks so M2/M3 pages and modals are assembled, not hand-rolled each time. Match current styling: Tailwind v4 + CSS variables (`--bq-border`, `--bq-surface`, `--bq-muted`), `lucide-react` icons, and the `cn` helper from [src/lib/utils.ts](../../src/lib/utils.ts).

## Current state

- No shared component library; UI is inline in [ExplorerPage.tsx](../../src/features/explorer/ExplorerPage.tsx).
- Existing patterns to mirror: bordered surfaces, `hover:bg-white/5`, blue accent (`bg-blue-600`), tab underline (`border-b-2 border-blue-500`).
- Existing leaf components: [ResultsTable.tsx](../../src/features/explorer/components/ResultsTable.tsx), [JsonViewer.tsx](../../src/features/explorer/components/JsonViewer.tsx), [SqlEditor.tsx](../../src/features/explorer/components/SqlEditor.tsx).

## Scope

Create `src/components/ui/` with:

### 1. `Modal.tsx`

- Props: `open`, `onClose`, `title`, `footer`, `size`, children.
- Overlay with click-out + Escape to close, basic focus management, scroll lock.
- Slots: header (title + close X), scrollable body, sticky footer for actions.
- Used by Create Table, Copy, Snapshot, Edit Schema modals.

### 2. `Tabs.tsx` / `TabBar.tsx`

- Controlled `activeId` + `onChange`, array of `{ id, label, badge?, closable? }`.
- Underline-style tabs (resource tabs) and a closable workspace-tab variant (used by m1-03 for the multi-tab bar).
- Overflow handling (scroll/wrap) for many tabs.

### 3. `DetailTable.tsx` + `SectionHeading.tsx`

- `DetailTable` renders an array of `{ label, value }` as a two-column key/value table (matches the current `info` tab table styling) with support for `—` empty values and custom value renderers (labels/tags chips, code blocks).
- `SectionHeading` for "Dataset info", "Table info", "Storage info", "Query".

### 4. `ActionToolbar.tsx` + `ToolbarButton.tsx`

- Horizontal button row with icon+label, primary/secondary/danger variants, disabled state, and optional dropdown (for Save / Copy-as menus).

### 5. Helpers

- `CopyButton.tsx` — clipboard write with transient "Copied" state (reuse `navigator.clipboard` pattern already used by Share in `ExplorerPage`).
- `UnplannedTab.tsx` — centered placeholder ("This view is not planned yet") for Insights/Lineage/etc.
- `ConfirmDialog.tsx` — thin wrapper over `Modal` for Delete confirmations.

## Out of scope

- Data fetching or business logic (pure presentational + local state only).
- Routing and workspace wiring (m1-03).

## Verification

```bash
npm run build
npm run lint
```

Optionally render each primitive on a temporary scratch route during development; remove before commit.

## Done criteria

- All primitives exist under `src/components/ui/`, typed, lint-clean.
- Styling visually consistent with existing surfaces (dark theme, CSS variables).
- No regression to existing pages (primitives are additive).
