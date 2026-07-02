---
name: Roadmap remaining work overview
overview: Index of the remaining unimplemented ROADMAP.md items mapped to sustainment plan files, with verified current state at HEAD and recommended execution order.
todos:
  - id: true-up-planned-table
    content: True up the stale "Today" column in ROADMAP.md §Planned work (ML inference, MEASURE, LOAD DATA gs:// rows all show pre-landing states)
    status: pending
  - id: keep-index-fresh
    content: Re-verify each row below against HEAD and flip plan statuses as items land
    status: pending
isProject: false
---

# Roadmap remaining work — overview & index

**Origin:** [`ROADMAP.md`](../../../ROADMAP.md) §Planned work + §External data
sources + §Python UDFs + §Build systems, cross-checked against
[`docs/ENGINE_POLICY.md`](../../../docs/ENGINE_POLICY.md) §Unsupported families
and the disposition registries.

**Verified at HEAD:** 2026-07-01. Everything else in the Planned-work table has
landed (BigQuery ML stubs, privacy-aggregate stubs, `SESSION_USER`,
`ST_GEOGFROMWKB`, KLL sketches, MEASURE functions, expression columns,
sequence/catalog-ref envelope sharpening, `LOAD DATA gs://`, SQL Tools API,
scalar Python UDFs). The rows below are what remains.

---

## Item → plan → current-state map

| # | Roadmap item | Kind | Plan file | Verified state at HEAD |
|---|--------------|------|-----------|------------------------|
| 1 | `UNDROP SCHEMA` | **real** | [`01-undrop-schema.plan.md`](01-undrop-schema.plan.md) | `RunUndrop` (`backend/engine/control/control_op_time_travel.cc` ~262) returns `UNIMPLEMENTED`; REST `datasets.undelete` is a 501 stub (`gateway/handlers/datasets.go` ~321); table-level tombstone infra already exists (`duckdb_storage_version_log_tombstone.cc`) |
| 2 | `EXTERNAL_QUERY` + cloud-resource connections / federated paths | **fixture-backed real** | [`02-external-query-connections.plan.md`](02-external-query-connections.plan.md) | bqconnection CRUD stubs wired (`gateway/handlers/bqconnection/`); `Config.ConnectionFixtureRoot()` reserves `$data_dir/external/connections/`; no engine-side `EXTERNAL_QUERY` TVF — analyzer rejects it today |
| 3 | Table-valued / aggregate Python UDFs | **real** | [`03-python-udf-nonscalar.plan.md`](03-python-udf-nonscalar.plan.md) | Scalar path landed (`python_udf_runtime.cc`, `python_udf_registry.cc`); non-scalar shapes reject at CREATE |
| 4 | Python UDF `packages` option | **bounded real** | [`04-python-udf-packages.plan.md`](04-python-udf-packages.plan.md) | Runtime is stdlib + host `lxml` only; `packages` list rejected/ignored |
| 5 | linux/arm64 engine build | **infra** | [`05-linux-arm64-engine.plan.md`](05-linux-arm64-engine.plan.md) | GoogleSQL hermetic LLVM toolchain is amd64-only; releases + Docker ship amd64 engine only |

**Explicitly NOT planned** (do not create plans): Graph / GQL
(`ResolvedGraph*Scan`), cloud passthrough, BigQuery Omni, Go GoogleSQL port —
see ROADMAP §Non-goals.

---

## Recommended execution order

1. **Plan 01 — UNDROP SCHEMA.** Smallest surface; tombstone infra exists;
   closes the last `unsupported` DDL row and un-stubs `datasets.undelete`.
2. **Plan 03 — non-scalar Python UDFs.** Extends a landed runtime; the UDAF /
   TVF evaluation scaffolding already exists for SQL UDFs.
3. **Plan 04 — Python packages.** Builds directly on plan 03's runtime work.
4. **Plan 02 — EXTERNAL_QUERY.** Larger design surface (fixture contract, TVF
   registration in the analyzer catalog); ship fixture mode first.
5. **Plan 05 — arm64 engine.** Independent infra track; can proceed in
   parallel but has the longest feedback loop (toolchain + CI runners).

## Conventions (same as the top-level plan set)

- Each plan has YAML frontmatter (`name`, `overview`, `todos`) plus: current
  state with file anchors, done-criteria, steps, tests, out-of-scope.
- Every promotion off `unsupported` lands handler + conformance fixture(s) +
  disposition-registry update (`functions.yaml` / `node_dispositions.yaml`) in
  the same commit, per `docs/ENGINE_POLICY.md`.
- Keep `ROADMAP.md`, `docs/ENGINE_POLICY.md`, and `SHAPE_TRACKER.md` in sync
  when a status changes.
