# Benchmark suite

Compares query latency and correctness across three targets:

| Target | Backend |
|--------|---------|
| **vantaboard** (this repo) | Native GoogleSQL + DuckDB via `emulator_main` |
| **goccy** | [goccy/bigquery-emulator](https://github.com/goccy/bigquery-emulator) Docker image (`0.8.1`) |
| **BigQuery** | Real BigQuery via ADC (golden baseline only) |

## Quick start

```bash
# Build engine (once)
task emulator:build-engine:bazel

# Run vantaboard + goccy benchmarks and charts
task bench:run

# Rerun one case after a fix (merges into bench/results.json, regenerates charts)
CASE=create_view_100k task bench:run

# Compare vantaboard against committed BigQuery baseline (emulator only;
# does not rewrite bench/results.json — use bench:run for goccy charts)
task bench:compare
```

## Case format

Cases live in [`bench/cases/`](cases/). Each file defines deterministic SQL setup and a query:

```yaml
name: agg_group_by_100k
setup:
  - sql: CREATE TABLE {{ds}}.t AS SELECT id FROM UNNEST(GENERATE_ARRAY(1, 100000)) AS id
query: SELECT COUNT(*) AS cnt FROM {{ds}}.t
iterations: 10
warmup: 2
max_ratio: 1.5   # optional; default 1.5x BigQuery p50
max_ms: 180000  # optional; emulator compare-gate cap (also query cap when above 30s)
query_timeout_ms: 600000  # optional; explicit per-query wall cap (overrides max_ms for queries)
```

`{{ds}}` is substituted per target (emulator dataset id, or `project.dataset` on BigQuery).

`max_ms` gates emulator latency in `bench:compare`. Heavy cases also reuse raised
`max_ms` as the default query wall cap (60s otherwise). Goccy on those cases gets
a 10-minute floor so slow queries finish without loosening the emulator gate.
Override with `query_timeout_ms` per case, or `--timeout` on the bench CLI.

Cases tagged `heavy` (e.g. `join_hash_2m`, `agg_high_card_2m`, `order_by_1m`,
`window_partition_1m`) build 1M–2M row tables so engine quality — not fixed
per-query overhead — dominates the result. They use fewer `iterations` and a
raised `max_ms`. Capture a fresh BigQuery baseline (`task bench:baseline`) after
adding heavy cases so they are gated and charted.

> **`GENERATE_ARRAY` cap:** BigQuery rejects a single `GENERATE_ARRAY` that
> produces more than 1,048,576 elements (`Error 400: ... produced too many
> elements`). The heavy 2M-row cases build tables with a `CROSS JOIN` of two
> `UNNEST(GENERATE_ARRAY(1, 1000000))` relations inside a CTAS subquery (2×1M
> rows). That pattern is valid on BigQuery and is transpiled on the vantaboard
> engine.

Cases tagged `ddl` / `view` exercise metadata + materialization paths that pure
`SELECT` cases miss: `view_agg_100k` queries *through* a view, `ctas_agg_100k`
times a `CREATE OR REPLACE TABLE ... AS SELECT`, and `create_view_100k` times a
`CREATE OR REPLACE VIEW`. DDL statements return no rows, so their result hash is
the empty-set hash on every target. goccy 0.8.1 does not honor `CREATE OR
REPLACE` on repeated runs; the goccy target runs an untimed `DROP ... IF
EXISTS` preamble before each timed `CREATE` so iterations stay idempotent.

## BigQuery golden baseline

Capture requires ADC and a billing project:

```bash
export BENCH_BQ_PROJECT=your-gcp-project
task bench:baseline
```

This writes [`bench/baselines/bigquery.json`](baselines/bigquery.json) with per-case p50 latency, result hash, and row count. Commit that file after capture.

Baseline capture sets `DisableQueryCache = true` and rejects cache hits so execution times are not artificially zero.

Until a baseline exists, `task bench:compare` reports `no baseline for case` per case instead of failing the gate.

### Latency metrics

Comparisons and the pass/fail gate use **server-side** latency on both sides where available:

| Field | Meaning |
|-------|---------|
| `execution_p50_ms` | BQ `endTime − startTime` (console **Duration**; excludes queue + client) |
| `total_p50_ms` | BQ client wall-clock incl. poll + fetch (diagnostic only) |
| `queue_p50_ms` | BQ `startTime − creationTime` (slot queue wait; diagnostic) |
| `total_slot_ms_p50` | BQ slot-milliseconds consumed (resource metric, not used in latency gate) |
| `engine_p50` / `phases.total_engine` | Emulator server-side engine path (compare numerator) |
| `latency.p50` | Emulator HTTP round-trip (diagnostic) |

BQ `execution_p50_ms` excludes queue time (`creationTime → startTime`) and all client-side overhead. It matches the console Duration column, not "time from click to results."

Goccy has no `total_engine` phase; goccy chart bars use HTTP wall-clock only. The
`comparison.svg` chart plots **vantaboard (wall)** — the emulator's `latency.p50`
HTTP round-trip — against **goccy (wall)** and BigQuery job duration. The pass/fail
gate continues to use `total_engine`, not wall.

## Outcomes

Each (case, target) records:

- `ok` — query succeeded and result hash matches baseline (when baseline present)
- `error` — setup or query rejected (unsupported feature on goccy is expected data)
- `wrong_result` — hash mismatch vs BigQuery golden
- `timeout` — exceeded per-case wall cap (default 60s)
- `skipped` — case opts out of a target via `skip_targets` in the case YAML (upstream goccy bugs, etc.)

Per-case skips:

```yaml
skip_targets: [goccy]
skip_reason: "upstream goccy bug (document why in the case YAML)"
```

The pass/fail **gate applies only to vantaboard**. Goccy numbers are competitive evidence, not CI failures.

## Phase timing

Loopback `jobs.query` responses include `statistics.query.emulatorPhases` (microseconds per engine phase):

| Phase | Where |
|-------|--------|
| `analyze_frontend` | Frontend `AnalyzeStatement` |
| `route_classify_frontend` | Frontend `RouteClassifier` |
| `analyze_coordinator` | Coordinator re-analysis |
| `route_classify_coordinator` | Coordinator `RouteFor` |
| `transpile` | DuckDB transpiler |
| `duckdb_setup` | Open / attach / materialize |
| `duckdb_execute` | `duckdb_query` |
| `row_stream` | `RowSource::Next` loop |
| `total_engine` | Full engine path |

## Profiling

```bash
task bench:build-profile
task bench:profile CASE=agg_group_by_100k MODE=cpu   # perf.data under bench/profiles/
task bench:profile CASE=agg_group_by_100k MODE=heap  # heaptrack output
```

## Charts & CI

- `task bench:charts` — matplotlib/seaborn SVGs in `bench/charts/out/`:
  - `comparison.svg` — log-scale latency bars: vantaboard wall, goccy wall, BQ job duration (× marks where goccy skipped a case)
  - `phases.svg` — stacked p50 engine phase timings for vantaboard cases
- Re-run `task bench:run` after engine changes so `engine_p50` / `total_engine` populate results
- Committed `bench/charts/out/*.svg` snapshots are embedded in the root
  [`README.md`](../README.md); CI on `main` refreshes them after each run
- `.github/workflows/bench.yml` runs after `build-engine`, pulls goccy Docker, uploads artifacts, publishes charts to `gh-pages` on `main`

## Environment

| Variable | Purpose |
|----------|---------|
| `BENCH_BQ_PROJECT` | BigQuery project for baseline capture |
| `BENCH_SKIP_GOCCY=1` | Skip goccy Docker target |
| `BIGQUERY_EMULATOR_BIN` | Override path to `emulator_main` |
