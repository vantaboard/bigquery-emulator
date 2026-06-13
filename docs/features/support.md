# Feature support

The emulator is a preview (`v0.x`) project. Support is tracked across
several complementary artifacts rather than a single markdown matrix.

## Quick reference

| Area | Where to look | What it covers |
|------|---------------|----------------|
| REST API endpoints | [REST API](../REST_API.md) | Per-method status: `done`, `wired`, `todo` |
| SQL execution routes | [Engine policy](../ENGINE_POLICY.md) | DuckDB fast path, semantic executor, stubs |
| Capability roadmap | [ROADMAP](https://github.com/vantaboard/bigquery-emulator/blob/main/ROADMAP.md) | Milestone narrative and design rationale |
| Executable SQL truth | [Conformance](https://github.com/vantaboard/bigquery-emulator/blob/main/conformance/README.md) | ~170 YAML fixtures pinning query behavior |
| Client library parity | [Third-party harnesses](https://github.com/vantaboard/bigquery-emulator/blob/main/third_party/README.md) | Go, Python, Java, Node, DataFrames, dbt |

## REST API coverage

The gateway implements the BigQuery v2 REST surface end-to-end for the
core resources (projects, datasets, tables, tabledata, jobs, queries,
routines). Many ancillary surfaces (models, migration, data transfer,
IAM custom methods) are **wired stubs** that return structurally-valid
empty pages or 501 so client libraries probe successfully at startup.

See the full endpoint → handler mapping in [REST API](../REST_API.md).

## SQL & engine routes

Query execution dispatches through a route classifier. Each resolved AST
shape lands on one of:

| Route | Role |
|-------|------|
| `duckdb_native` | Fast analytical path via DuckDB SQL |
| `duckdb_udf` / `duckdb_rewrite` | BigQuery functions polyfilled in DuckDB |
| `semantic_executor` | Exact BigQuery evaluation (NUMERIC, etc.) |
| `control_op` | DDL / DML / catalog operations |
| `local_stub` | Returns a documented stub response |
| `unsupported` | Surfaces `UNIMPLEMENTED` to the client |

Per-function and per-AST-node dispositions live in the engine source:

- [`functions.yaml`](https://github.com/vantaboard/bigquery-emulator/blob/main/backend/engine/duckdb/transpiler/functions.yaml)
- [`node_dispositions.yaml`](https://github.com/vantaboard/bigquery-emulator/blob/main/backend/engine/duckdb/transpiler/node_dispositions.yaml)
- [`SHAPE_TRACKER.md`](https://github.com/vantaboard/bigquery-emulator/blob/main/backend/engine/duckdb/transpiler/SHAPE_TRACKER.md)

## Benchmark support matrix

The [bench harness](https://github.com/vantaboard/bigquery-emulator/blob/main/bench/README.md)
compares query latency and **correctness outcomes** across three backends:

- **vantaboard** — this emulator
- **goccy** — [goccy/bigquery-emulator](https://github.com/goccy/bigquery-emulator) Docker `0.7.2`
- **BigQuery** — committed golden baselines

The support matrix chart is a heatmap of benchmark case outcomes
(`ok`, `error`, `wrong_result`, `timeout`, `skipped`) per target — not
a SQL function coverage table.

![Benchmark support matrix](https://vantaboard.github.io/bigquery-emulator/bench/support_matrix.svg)

Live charts also publish to
[gh-pages `bench/`](https://vantaboard.github.io/bigquery-emulator/bench/).
Regenerate locally with `task bench:run` then `task bench:charts`.

## Conformance fixtures

YAML fixtures under
[`conformance/fixtures/`](https://github.com/vantaboard/bigquery-emulator/tree/main/conformance/fixtures)
are the executable source of truth for SQL semantics. Each fixture can
assert an expected engine route:

```yaml
expected:
  route: duckdb_native
  match: ordered
  rows:
    - {n: 1}
```

Run the full suite:

```bash
task conformance:run
```

Generate a routing matrix artifact:

```bash
task conformance:routing-matrix
```

## What is intentionally unsupported

- **Authentication / IAM** — tokens accepted, never validated
- **BQML / trained models** — list returns empty; mutations return 501
- **Row-level access policies** — list returns empty; IAM methods return 501
- **Migration workflows** — list returns empty; create/start return 501
- **Data Transfer Service** — list returns empty; create returns 501
- **Legacy SQL dialect** — only GoogleSQL (narrow bracket-ref translation for samples)

See [Engine policy](../ENGINE_POLICY.md) for the full route catalog and
specialized feature families (ML, GIS, HLL, scripting, wildcards).
