---
name: Roadmap remaining work overview
overview: Index of the remaining unimplemented ROADMAP.md items mapped to sustainment plan files, with verified current state at HEAD and recommended execution order.
todos:
  - id: true-up-planned-table
    content: True up the stale "Today" column in ROADMAP.md §Planned work (ML inference, MEASURE, LOAD DATA gs:// rows all show pre-landing states)
    status: completed
  - id: keep-index-fresh
    content: Re-verify each row below against HEAD and flip plan statuses as items land
    status: completed
isProject: false
---

# Roadmap remaining work — overview & index

**Origin:** [`ROADMAP.md`](../../../ROADMAP.md) §Planned work + §External data
sources + §Python UDFs + §Build systems, cross-checked against
[`docs/ENGINE_POLICY.md`](../../../docs/ENGINE_POLICY.md) §Unsupported families
and the disposition registries.

**Verified at HEAD:** 2026-07-02 (`d7da4024`). All five roadmap sustainment
plans have been executed via subagents. Plans 01–04 are fully landed; plan 05
landed CI/infra with release multi-arch deferred until `build-engine-arm64` is
green.

---

## Item → plan → outcome map

| # | Roadmap item | Kind | Plan file | Outcome at HEAD |
|---|--------------|------|-----------|-----------------|
| 1 | `UNDROP SCHEMA` | **real** | [`01-undrop-schema.plan.md`](01-undrop-schema.plan.md) | **Landed** — dataset tombstones, `RunUndrop` for SCHEMA, `datasets.undelete` RPC + REST; conformance fixtures |
| 2 | `EXTERNAL_QUERY` + connections | **fixture-backed real** | [`02-external-query-connections.plan.md`](02-external-query-connections.plan.md) | **Landed** — fixture TVF in engine, bqconnection CRUD persistence, federated posture envelopes |
| 3 | Table-valued / aggregate Python UDFs | **real** | [`03-python-udf-nonscalar.plan.md`](03-python-udf-nonscalar.plan.md) | **Closed (reject)** — bq dry-run rejects both forms; sharpened analyzer rejects + conformance fixtures |
| 4 | Python UDF `packages` option | **bounded real** | [`04-python-udf-packages.plan.md`](04-python-udf-packages.plan.md) | **Landed** — parse/persist, venv resolution, preflight envelope, `task python-udf:provision` |
| 5 | linux/arm64 engine build | **infra** | [`05-linux-arm64-engine.plan.md`](05-linux-arm64-engine.plan.md) | **Partial** — DuckDB arm64 select, prebuilt arm64 CI matrix, non-blocking `build-engine-arm64`; goreleaser/Docker multi-arch deferred |

**Explicitly NOT planned** (do not create plans): Graph / GQL
(`ResolvedGraph*Scan`), cloud passthrough, BigQuery Omni, Go GoogleSQL port —
see ROADMAP §Non-goals.

---

## Execution order (completed 2026-07-02)

All five plans were dispatched sequentially per
`.cursor/plans/subagent_roadmap_execution_aea8d9f8.plan.md`:

1. **Plan 01** — UNDROP SCHEMA (`df654678`..`060a58ec`)
2. **Plan 03** — non-scalar Python UDFs sharpened rejects (`bf1f0b77`)
3. **Plan 04** — Python packages (`8d4a1214`..`fe60f05c`)
4. **Plan 02** — EXTERNAL_QUERY (`16ec1186`..`baf951d8`)
5. **Plan 05** — arm64 infra (`4a2d032f`..`d7da4024`)

## Follow-ups (plan 05 deferred items)

- Set `GOOGLESQL_PREBUILT_URL_ARM64` / `GOOGLESQL_PREBUILT_SHA256_ARM64` after
  first successful arm64 prebuilt publish.
- Monitor `build-engine-arm64` (non-blocking); promote to blocking when stable.
- Land goreleaser + `release.yml` multi-arch once arm64 engine builds green.

## Conventions (same as the top-level plan set)

- Each plan has YAML frontmatter (`name`, `overview`, `todos`) plus: current
  state with file anchors, done-criteria, steps, tests, out-of-scope.
- Every promotion off `unsupported` lands handler + conformance fixture(s) +
  disposition-registry update (`functions.yaml` / `node_dispositions.yaml`) in
  the same commit, per `docs/ENGINE_POLICY.md`.
- Keep `ROADMAP.md`, `docs/ENGINE_POLICY.md`, and `SHAPE_TRACKER.md` in sync
  when a status changes.
