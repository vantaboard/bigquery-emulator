---
name: Thirdparty 09 — Public dataset seed
overview: Seed bigquery-public-data catalog tables so thirdparty query samples can reference usa_names, shakespeare, stackoverflow without skips.
depends_on: []
blocks: [thirdparty-02-gateway-query-metadata]
baseline_log: .logs/thirdparty-20260605-112926.log
est_effort: 3-5 days
isProject: true
todos:
  - id: survey-samples
    content: Enumerate all bigquery-public-data table refs used in third_party python/node samples
    status: pending
  - id: seed-format
    content: Choose seed mechanism — initial-data-dir, gateway startup seed, or docker volume fixture
    status: pending
  - id: seed-data
    content: Vendor minimal row subsets for required public tables under testdata/
    status: pending
  - id: wire-startup
    content: Load seed on emulator startup (gateway or engine catalog bootstrap)
    status: pending
  - id: remove-skips
    content: Tighten emulator_pytest_skip and node setup skips only where seed covers tables
    status: pending
  - id: public-data-tests
    content: Green node Queries stackoverflow/usa_names and any python public-data tests
    status: pending
---

# Thirdparty 09 — Public dataset seed

## Goal

Resolve `Table not found: bigquery-public-data.*` failures without hitting real GCP.

## Baseline failures

| Suite | Tests |
|-------|------|
| Node | `should query stackoverflow` — `bigquery-public-data.stackoverflow.posts_questions` |
| Node | `should run a query stateless mode` — `bigquery-public-data.usa_names.usa_1910_2013` |
| Node | `should run a query with the cache disabled` — expects `corpus` from shakespeare |
| Python | Any samples still hitting public-data (most skipped by `emulator_pytest_skip.py`) |

## Context

[`third_party/README.md`](../../third_party/README.md) line 154:

> go-googlesql snapshot under `testdata/bq-emulator/` (`-initial-data-dir`) seeds these. This emulator does not yet expose `--initial-data-dir` on `gateway_main`.

## Implementation options

| Option | Pros | Cons |
|--------|------|------|
| A. `--initial-data-dir` flag on gateway | Matches go-googlesql | New CLI surface + docs |
| B. Docker compose seed job on `bq-emulator-data` volume | Works for `task thirdparty` | Heavier fixture pipeline |
| C. Gateway startup REST bootstrap from `testdata/public-data/` | No new flag | Slower cold start |

**Recommended:** Option A or C — document in `ROADMAP.md`.

## Steps

### 1. Inventory

```bash
rg -l 'bigquery-public-data' third_party/python-bigquery-tests third_party/node-bigquery-tests
```

Build table list: project.dataset.table + minimal schema.

### 2. Minimal fixtures

Store under e.g. [`testdata/public-data/`](../../testdata/public-data/):
- Parquet or JSON row subsets (not full public datasets)
- Schemas matching sample queries (column names types samples SELECT)

### 3. Catalog registration

On startup (or `initial-data-dir` load):
- `RegisterDataset(bigquery-public-data.usa_names)`
- `RegisterTable` + insert seed rows for each required table

Reuse patterns from [`gateway/seed/`](../../gateway/seed/) if applicable.

### 4. Skip matrix update

Only remove skips for tables actually seeded. Keep `emulator_pytest_skip.py` for unseeded public datasets.

## Verification

```bash
task thirdparty:node-bigquery-tests
# Queries suite: stackoverflow, usa_names, cache disabled tests

curl -s 'http://localhost:9050/bigquery/v2/projects/bigquery-public-data/datasets/usa_names/tables' | jq .
```

## Out of scope

- Full public dataset replication
- `bigquery-public-data` billing / cross-project ACL semantics

## Done when

- [ ] Node public-data query tests pass
- [ ] Seed documented in README + third_party/README.md
- [ ] Docker compose / task thirdparty loads seed automatically
