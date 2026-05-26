# Engine policy

This emulator ships **two C++ engines** behind a common
`backend/engine/engine.h` interface. The choice between them is
orchestrated by `--engine` (+ `--on_unknown_fn`) on `emulator_main`.
This document is the **single source of truth** for which engine new
work should target.

> **TL;DR**: build against **DuckDB**. Touch ReferenceImplEngine only
> when DuckDB cannot host the construct, or when a maintenance-mode
> bug fix is needed for the small surface ReferenceImplEngine still
> owns.

## The engines

| Engine | Source | Status | Used by |
|---|---|---|---|
| **DuckDB** | `backend/engine/duckdb/` | **Primary.** Active development surface. | `--engine=duckdb` (the default for production paths, the `duckdb` conformance profile, and `gateway/e2e/*duckdb*_test.go`). |
| **ReferenceImplEngine** | `backend/engine/reference_impl/` | **Maintenance-only.** No new features. | `--engine=reference_impl` (the `memory` conformance profile and the small set of E2E tests that pin reference-impl-specific surfaces). |
| **FallbackEngine** | `backend/engine/fallback/` (wrapper) | Transitional. **Will shrink** as DuckDB coverage grows. | `--engine=duckdb --on_unknown_fn=fallback` -- the canonical configuration for `gateway_main`. Routes unsupported-on-DuckDB constructs to ReferenceImplEngine. |

## What this means in practice

1. **New feature work targets DuckDB.** Add the construct to the
   DuckDB transpiler (`backend/engine/duckdb/transpile/`) and the
   DuckDB engine (`backend/engine/duckdb/duckdb_engine.cc`). The
   ReferenceImplEngine is **not** a development target for new
   features, even when it would be easier to land there first.

2. **ReferenceImplEngine receives bug fixes and security work
   only.** The reference impl covers UPDATE / DELETE / INSERT today
   and a slice of `INFORMATION_SCHEMA`. Treat that surface as frozen
   in shape; fix bugs in what already works, but resist the urge to
   grow it.

3. **FallbackEngine is a bridge, not a long-term home.** The
   `--on_unknown_fn=fallback` policy lets DuckDB-uncovered
   constructs (DML today, most DDL until plan 35 landed, parts of
   the analytics surface) work end-to-end while the DuckDB engine
   catches up. **Every fallback hop is a TODO**: the long-term goal
   is for DuckDB coverage to grow so the fallback can shrink to
   zero. When you implement a new construct on DuckDB, delete the
   corresponding fallback workaround.

4. **Storage follows the same asymmetry.** DuckDB+Parquet storage
   (`--storage=duckdb`) is the primary; the in-memory storage
   (`--storage=memory`) is kept around for hermetic test harnesses
   and for the `memory` conformance profile, but is not a target
   for new on-disk features.

## Implication for conformance fixtures

The conformance harness (`conformance/cmd/runner`, `conformance/fixtures/*.yaml`)
runs against two named profiles:

| Profile | Engine | Storage | Fallback |
|---|---|---|---|
| `memory` | reference_impl | memory | off |
| `duckdb` | duckdb | duckdb | on (`--on_unknown_fn=fallback`) |

Per the policy above:

- **Prefer fixtures that pass on both profiles.** A construct that
  works on `memory` almost always works on `duckdb` (either natively
  on DuckDB or via the fallback to ReferenceImpl). Targeting both
  catches engine-divergence regressions cheaply.

- **When a fixture only passes on one profile, prefer `duckdb`.**
  DuckDB is where new features land first; a duckdb-only fixture is
  pinning a real DuckDB feature. A memory-only fixture is pinning a
  ReferenceImpl-only surface (which should be rare and intentional;
  see point 2 above).

- **Document the rationale.** Any single-profile fixture should
  start with a top-of-file comment explaining *why* only one engine
  applies (e.g. "DDL is only implemented on DuckDB; ReferenceImpl
  returns UNIMPLEMENTED"). Without that comment, reviewers cannot
  tell if the asymmetry is intentional or a latent bug.

- **Legitimate ReferenceImpl-only surfaces** are exactly:
  - the `memory` profile's hermetic-storage fast path used by some
    CI lanes (no on-disk artifacts);
  - the small slice of BigQuery-specific behavior that DuckDB will
    never natively cover (some `INFORMATION_SCHEMA` views, certain
    BQ-specific functions DuckDB lacks).

  Everything else should target DuckDB and let `FallbackEngine`
  bridge the gap until DuckDB catches up.

## Cross-references

- `HANDOFF.md` §4 -- engine-architecture context (the WIP-checkpoint
  notes from plan 34 introduced this asymmetry; this policy doc
  promotes it to a first-class invariant).
- `conformance/README.md` -- fixture authoring guide; references this
  document from its "Contributing a new fixture" section.
- `backend/engine/fallback/fallback_engine.cc` -- the implementation
  of the bridge described above.
