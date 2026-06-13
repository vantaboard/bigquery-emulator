---
name: Full 06 — GEOGRAPHY / GIS via DuckDB spatial
overview: Promote GEOGRAPHY from a partial constructor (ST_GEOGPOINT only) to a real local implementation by integrating DuckDB's spatial extension - lower GEOGRAPHY type plumbing, the ST_* constructor/accessor/predicate/measurement families, and WKT/WKB/GeoJSON I/O - mapping BigQuery's GEOGRAPHY semantics onto DuckDB GEOMETRY where they agree and the semantic executor where they don't.
est_effort: ~3 weeks
isProject: true
todos:
  - id: spatial-extension
    content: "Vendor/enable DuckDB's `spatial` extension in the engine build (third_party/duckdb/ + executor connection init RegisterAll); confirm it links against the prebuilt libduckdb and loads at connection open. If the prebuilt lib can't load the extension, decide between a self-contained GEOS/s2 link or keeping the semantic-executor-only path."
    status: pending
  - id: type-plumbing
    content: "GEOGRAPHY type lowering end-to-end: stop treating it as kSkiplist in transpiler/types - map GEOGRAPHY<->DuckDB GEOMETRY for storage/scan, and define the REST wire encoding (BigQuery returns GEOGRAPHY as WKT text in results). Reconcile with the existing geog_funcs.cc value plumbing."
    status: pending
  - id: constructors-io
    content: "ST_GEOGFROMTEXT / ST_GEOGFROMWKB / ST_GEOGFROM / ST_GEOGPOINT / ST_MAKELINE / ST_MAKEPOLYGON + ST_ASTEXT / ST_ASBINARY / ST_ASGEOJSON: lower to DuckDB spatial equivalents (duckdb_native/duckdb_rewrite) where WKT/WKB round-trips match BigQuery; semantic executor otherwise."
    status: pending
  - id: predicates-measures
    content: "ST_DISTANCE / ST_WITHIN / ST_CONTAINS / ST_INTERSECTS / ST_DWITHIN / ST_AREA / ST_LENGTH / ST_PERIMETER: BigQuery GEOGRAPHY uses spherical (WGS84) geometry; DuckDB spatial defaults to planar. Either configure spherical mode or route distance/area to the semantic executor's s2-style computation so results match BigQuery's meters-on-a-sphere contract. Document any tolerance."
    status: pending
  - id: null-edge
    content: "NULL / empty-geography / antimeridian edge behavior matching BigQuery; SAFE.ST_* short-circuit consistency."
    status: pending
  - id: fixtures-trackers
    content: "conformance/fixtures/specialized/geography_*.yaml (constructor round-trip, distance, within/contains, WKT output); flip the st_* rows in functions.yaml off `unsupported`; update ENGINE_POLICY Geography row + SHAPE_TRACKER + ROADMAP type-lowering bullet; drop the `geograph` skip substrings from python/go skip matrices that now pass."
    status: pending
---

# Full 06 — GEOGRAPHY / GIS via DuckDB spatial

## Why

GEOGRAPHY is currently `unsupported (partial)`: only the `ST_GEOGPOINT`
constructor + GEOGRAPHY value plumbing exist
(`backend/engine/semantic/functions/geog_funcs.cc`), and the type is
`kSkiplist` in the transpiler. Every client lane skips geography
(`_MODULE_SKIP_SUBSTRINGS` includes `geograph`). DuckDB ships a
`spatial` extension that can do most of the heavy lifting; the
BigQuery-specific gap is **spherical vs planar** geometry — BigQuery
GEOGRAPHY is WGS84-spherical.

## The hard part

BigQuery measurements (`ST_DISTANCE`, `ST_AREA`) are on a sphere (s2);
DuckDB spatial is planar by default. The plan must either configure
DuckDB for spherical computation or own the measurement functions in the
semantic executor so results match BigQuery's meters-on-a-sphere
contract. Topological predicates (`ST_WITHIN`/`ST_CONTAINS`) are less
sensitive and may lower natively.

## Key files

- [`backend/engine/semantic/functions/geog_funcs.{h,cc}`](../../backend/engine/semantic/functions/) — existing GEOGRAPHY values
- [`backend/engine/semantic/functions/dispatch.cc`](../../backend/engine/semantic/functions/dispatch.cc) — ST_* dispatch
- [`backend/engine/duckdb/transpiler/types.{h,cc}`](../../backend/engine/duckdb/transpiler/) — GEOGRAPHY type lowering (`kSkiplist` today)
- [`backend/engine/duckdb/transpiler/functions.yaml`](../../backend/engine/duckdb/transpiler/functions.yaml) — `st_*` rows (`unsupported`)
- [`third_party/duckdb/`](../../third_party/duckdb/) — spatial extension enablement
- [`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md) — Geography row to flip

## Steps

1. Enable + smoke-test the DuckDB spatial extension in the engine build.
2. GEOGRAPHY type plumbing (storage/scan/REST WKT output).
3. Constructors + WKT/WKB/GeoJSON I/O.
4. Predicates + measurements with the spherical decision made + pinned.
5. NULL/edge behavior + SAFE.* consistency.
6. Fixtures + flip `st_*` rows + doc updates + skip-matrix removal.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task thirdparty:python    # geography samples
task bazel:shutdown && task bazel:status
```

## Out of scope

- BigQuery GIS clustering / spatial indexes (perf, not semantics).
- S2 cell-ID functions if they require a full s2 dependency the spatial
  extension doesn't cover — leave those `unsupported` and document.
