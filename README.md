# BigQuery Emulator

A locally-runnable emulator of Google Cloud BigQuery, intended for local
development and integration testing of applications that target the
BigQuery REST API.

> **Status:** very early scaffold. The Go REST gateway boots, answers a
> health probe, and registers every documented BigQuery v2 REST endpoint
> as a 501 stub. The C++ engine is a placeholder. See
> [`ROADMAP.md`](./ROADMAP.md) for the phased plan and
> [`docs/REST_API.md`](./docs/REST_API.md) for the per-endpoint mapping
> and current status.

## Architecture

This emulator is modeled directly on Google's
[`cloud-spanner-emulator`](https://github.com/GoogleCloudPlatform/cloud-spanner-emulator):

```
+-------------------------------+        +--------------------------------+
|  gateway_main (Go)            |  gRPC  |  emulator_main (C++)           |
|                               | <----> |                                |
|  - Implements BigQuery REST   |        |  - Links GoogleSQL directly    |
|    (projects/datasets/tables/ |        |  - Analyzer + reference_impl   |
|     jobs/queries/insertAll)   |        |  - In-memory catalog/storage   |
|  - Spawns engine as subproc   |        |                                |
+-------------------------------+        +--------------------------------+
```

- The **engine is C++** so it can link [GoogleSQL](https://github.com/google/googlesql)
  directly. SQL parsing, name resolution, type inference, and reference-impl
  execution come from upstream. We do **not** re-port that surface to Go.
- The **REST gateway is Go** because that is where the BigQuery-specific
  value lives: REST routes, jobs lifecycle, datasets/tables/projects model,
  streaming inserts, error envelope, discovery doc.
- The Go gateway spawns the C++ engine as a subprocess on startup and
  shuts it down cleanly on exit, identical to how `gateway_main` spawns
  `emulator_main` in the Spanner emulator.

This split is the same one Google's own emulator team picked for Spanner,
and the same one [`goccy/bigquery-emulator`][goccy] has converged on (via
WASM) for similar reasons. See [`ROADMAP.md`](./ROADMAP.md) for the design
rationale.

[goccy]: https://github.com/goccy/bigquery-emulator

## Repo layout

```
bigquery-emulator/
  binaries/
    gateway_main/main.go    # Go REST gateway entrypoint
    emulator_main/main.cc   # C++ engine entrypoint (links GoogleSQL)
  gateway/              # Go: HTTP server + subprocess manager
    gateway.go            # Lifecycle: spawn engine, run HTTP, shutdown
    server.go             # HTTP routing
    handlers/             # BigQuery REST handlers (one file per resource)
    bqtypes/              # Wire-compatible BigQuery REST types
    enginepb/             # Generated Go bindings for proto/emulator.proto
  frontend/             # C++: gRPC server that fronts the engine
    server/                 # gRPC plumbing
    handlers/               # Implements proto/emulator.proto services
  backend/              # C++: in-memory storage / schema / catalog
  proto/                # Internal Go <-> C++ contract
    emulator.proto
  build/                # Bazel + Docker glue (planned)
  Taskfile.yml          # Common dev commands (build, run, test, lint)
  Makefile              # Same as Taskfile, for users who prefer make
  CMakeLists.txt        # CMake build for the C++ side (alt. to Bazel)
  BUILD.bazel           # Bazel root (planned)
  MODULE.bazel          # Bzlmod root (planned)
  go.mod / go.sum       # Go module
  docs/                 # Documentation
    REST_API.md           # Endpoint -> handler mapping (read this when
                          # debugging a specific BigQuery REST call)
    bigquery/             # Vendored copy of the upstream BigQuery docs
                          # corpus, used as the source of truth when
                          # verifying request/response shapes
  ROADMAP.md            # Phased plan (read this first)
  README.md
  LICENSE               # MIT
```

## Quickstart

> Right now only the Go side builds and runs. Engine wiring is Phase 2 in
> [`ROADMAP.md`](./ROADMAP.md).

```bash
# Build the gateway.
go build -o bin/gateway_main ./binaries/gateway_main

# Run the gateway. It will try to spawn the engine; for now you can disable
# the engine subprocess with --engine_binary="" while we are still scaffolding.
./bin/gateway_main --engine_binary="" --http_port=9050

# In another shell:
curl -sS http://localhost:9050/        # health check
curl -sS http://localhost:9050/bigquery/v2/projects/test/datasets
```

When the C++ side starts being implemented (Phase 2 onward), the default
flow becomes:

```bash
task build         # build both gateway_main and emulator_main
task run           # run gateway, which spawns the engine
```

### Docker

The repo ships a multi-stage [`Dockerfile`](./Dockerfile) that builds
both the Go gateway and the C++ engine and packages them into a single
runtime image (mirrors the `gcr.io/cloud-spanner-emulator/emulator`
layout). A `docker/gateway_main.sh` shim injects `--hostname=0.0.0.0`
inside the container so the published port is reachable from the host
without forcing every caller to remember the flag.

```bash
# Build the image. Tag whatever you like; `bigquery-emulator:dev` here.
docker build -t bigquery-emulator:dev .

# Run it. Publish the REST gateway (9050) and, optionally, the internal
# engine gRPC port (9060) for debugging.
docker run --rm -p 9050:9050 -p 9060:9060 bigquery-emulator:dev

# In another shell, hit the REST surface on the host:
curl -sS http://localhost:9050/
curl -sS http://localhost:9050/bigquery/v2/projects/test/datasets
```

To override container defaults, pass extra flags after the image name —
they are forwarded to `gateway_main`:

```bash
docker run --rm -p 9050:9050 bigquery-emulator:dev \
    --log_requests --hostname=0.0.0.0 --http_port=9050
```

## Pointing client libraries at the emulator

Two equivalent ways to redirect a BigQuery client at the emulator:

1. **Endpoint override** (works in every official client). In Go:

   ```go
   client, err := bigquery.NewClient(ctx, "test-project",
       option.WithEndpoint("http://localhost:9050"),
       option.WithoutAuthentication(),
   )
   ```

2. **`BIGQUERY_EMULATOR_HOST` environment variable** (mirrors the
   `STORAGE_EMULATOR_HOST` and `SPANNER_EMULATOR_HOST` conventions used by
   other Google emulators):

   ```bash
   export BIGQUERY_EMULATOR_HOST=localhost:9050
   ```

Bearer tokens in `Authorization` headers are accepted but never
validated, identical to `cloud-spanner-emulator`'s posture. The full
upstream auth model (ADC, service-account keys, OAuth scopes) documented
under [`docs/bigquery/docs/authentication.md`](./docs/bigquery/docs/authentication.md)
is intentionally **not** modeled.

### SQL dialect

BigQuery's `useLegacySql` field defaults to `true` on the wire (older
clients still rely on this). The emulator only supports GoogleSQL,
because the engine is GoogleSQL's analyzer + reference impl. The query
handlers will:

- Treat `useLegacySql` unset or `false` as GoogleSQL.
- Reject `useLegacySql=true` with HTTP 400 + `reason: invalidQuery`.

If you're using the official Go client, explicitly set
`Query.UseLegacySQL = false` to be safe.

Python (`google-cloud-bigquery`), Java, and Node.js clients all support
the analogous endpoint override. We document each one as the relevant
smoke tests pass in Phase 8.

## Why C++ for the engine

See the rationale captured at the top of [`ROADMAP.md`](./ROADMAP.md). The
short version: GoogleSQL is a 700+-file, monthly-releasing C++ library with
no idiomatic Go equivalent. `cloud-spanner-emulator` and Google's own
non-C++ GoogleSQL bindings (Java) all wrap the C++ implementation rather
than porting it. We do the same and put our effort into the
BigQuery-specific surface.

## License

MIT. See [`LICENSE`](./LICENSE).
