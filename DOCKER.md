# Docker image (local build)

The emulator links **go-googlesql** (CGO + C++) and **go-googlesql-engine** using the default **`googlesql,googlesql_unified_prebuilt`** tags (see [go-googlesql `docs/prebuilt-cgo.md`](https://github.com/vantaboard/go-googlesql/blob/main/docs/prebuilt-cgo.md)).

## Primary path: build from this repo ([`Taskfile.yml`](Taskfile.yml))

With sibling checkouts `../go-googlesql` and `../go-googlesql-engine`, from **this** directory:

```bash
task docker:build
```

This uses [`Dockerfile.linked`](Dockerfile.linked), **`GO_GOOGLESQL_BASE`** (default pinned in [`Taskfile.yml`](Taskfile.yml) to the same `go-googlesql` tag as `go.mod`), and **`docker build --build-context`** so the image matches your local trees. This is the path used by CI/release. Override paths if needed: `GO_GOOGLESQL_ROOT=... GO_GOOGLESQL_ENGINE_ROOT=... task docker:build`.

## Secondary local fallback: parent-directory context (`Dockerfile`)

Local development uses [`go.work.dev`](go.work.dev) (workspace `replace`, not `go.mod`); this fallback image is built from a **parent directory** that contains all three repositories. Keep it for local workspace builds only; do not treat it as the primary CI/release path.

```text
your-workspace/
  go-googlesql/
  go-googlesql-engine/
  bigquery-emulator/    ← this repo
```

From the **parent** directory (e.g. `~/Code` if your clones live there):

1. **One-time:** install a workspace-root `.dockerignore` so Docker does not try to send Bazel/cache trees under `go-googlesql` (often root-owned → `permission denied`):

   ```bash
   ./bigquery-emulator/docker/prep-context.sh
   ```

   This writes `../.dockerignore` relative to `bigquery-emulator` (the directory that contains `go-googlesql`, `go-googlesql-engine`, and `bigquery-emulator`). It refuses to overwrite an existing `.dockerignore` that differs; merge by hand or remove the file first.

2. **Build the image:**

   ```bash
   docker build -f bigquery-emulator/Dockerfile -t bigquery-emulator:local .
   ```

If your Docker CLI supports `docker build --ignorefile …` (BuildKit 0.11+), you can skip `prep-context.sh` and pass `--ignorefile bigquery-emulator/docker/parent.dockerignore` instead of installing `.dockerignore`.

## Run

Default flags listen on all interfaces (HTTP **9050**, gRPC **9060**). Create projects at runtime with **`POST /emulator/v1/projects`** and JSON body `{"id":"<project-id>"}`. The **`--project`** / **`BIGQUERY_EMULATOR_PROJECT`** and **`--dataset`** / **`BIGQUERY_EMULATOR_DATASET`** flags are **deprecated** (optional seed only).

```bash
docker run --rm -p 9050:9050 -p 9060:9060 bigquery-emulator:local
```

Override ports or logging:

```bash
docker run --rm -p 8080:8080 -p 9090:9090 bigquery-emulator:local \
  --host=0.0.0.0 \
  --port=8080 \
  --grpc-port=9090 \
  --log-level=debug
```

Point BigQuery client libraries at the emulator using your usual mechanism (e.g. `BIGQUERY_EMULATOR_HOST` for some stacks, or API endpoint URL for REST). The REST base URL is `http://localhost:<port>` when the port is published as above.

Persist the SQLite backing store (default is a temp file) by mounting a file and passing `--database`:

```bash
docker run --rm -p 9050:9050 -v bqemu-data:/data bigquery-emulator:local \
  --database=/data/emulator.db
```

Use in-memory storage:

```bash
docker run --rm -p 9050:9050 bigquery-emulator:local \
  --database=:memory:
```

Environment variables (see `--help`) mirror the long flags, e.g. `BIGQUERY_EMULATOR_PROJECT` (deprecated), `BIGQUERY_EMULATOR_DATASET`.

Long DuckDB reads: **wall clock vs `EXPLAIN ANALYZE`**, pprof, and verification are documented in [`docs/duckdb-long-query-profiling.md`](docs/duckdb-long-query-profiling.md).

## Why not plain `docker build .` without `Dockerfile.linked`?

Published versions on the module proxy do not ship prebuilt `.a` archives; the supported stack uses sibling **`go-googlesql`** sources (with prebuilts) plus **`go-googlesql-stack-bootstrap.sh`**-compatible env at link time. [`Taskfile.yml`](Taskfile.yml) `docker:build` encodes that layout.

## Why not `docker build` inside only `bigquery-emulator/` without build contexts?

Published versions on the module proxy may not match your local workspace; building from the three-repo layout matches sibling checkouts and avoids API skew.
