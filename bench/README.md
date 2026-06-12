# Benchmark suite

Compares query latency and correctness across three targets:

| Target | Backend |
|--------|---------|
| **vantaboard** (this repo) | Native GoogleSQL + DuckDB via `emulator_main` |
| **goccy** | [goccy/bigquery-emulator](https://github.com/goccy/bigquery-emulator) Docker image (`0.7.2`) |
| **BigQuery** | Real BigQuery via ADC (golden baseline only) |

## Quick start

```bash
# Build engine (once)
task emulator:build-engine:bazel

# Run vantaboard + goccy benchmarks and charts
task bench:run

# Compare vantaboard against committed BigQuery baseline
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
max_ms: 30000    # optional absolute cap
```

`{{ds}}` is substituted per target (emulator dataset id, or `project.dataset` on BigQuery).

## BigQuery golden baseline

Capture requires ADC and a billing project:

```bash
export BENCH_BQ_PROJECT=your-gcp-project
task bench:baseline
```

This writes [`bench/baselines/bigquery.json`](baselines/bigquery.json) with per-case p50 latency, result hash, and row count. Commit that file after capture.

Until a baseline exists, `task bench:compare` reports `no baseline for case` per case instead of failing the gate.

## Outcomes

Each (case, target) records:

- `ok` — query succeeded and result hash matches baseline (when baseline present)
- `error` — setup or query rejected (unsupported feature on goccy is expected data)
- `wrong_result` — hash mismatch vs BigQuery golden
- `timeout` — exceeded per-case wall cap (default 60s)

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

- `task bench:charts` — matplotlib/seaborn SVGs in `bench/charts/out/`
- `.github/workflows/bench.yml` runs after `build-engine`, pulls goccy Docker, uploads artifacts, publishes charts to `gh-pages` on `main`

## Environment

| Variable | Purpose |
|----------|---------|
| `BENCH_BQ_PROJECT` | BigQuery project for baseline capture |
| `BENCH_SKIP_GOCCY=1` | Skip goccy Docker target |
| `BIGQUERY_EMULATOR_BIN` | Override path to `emulator_main` |
